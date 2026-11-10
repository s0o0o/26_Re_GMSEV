#include "../TP_2023192003/TP_2023192003/session_completion.hpp"
#include "../TP_2023192003/TP_2023192003/session_registry.hpp"
#include <cassert>
#include <cstring>
#include <iostream>

struct Completion {
    std::unique_ptr<OVER_EXP> operation;
    DWORD bytes;
    ULONG_PTR key;
    bool success;
};
int main() {
    WSADATA data{}; assert(WSAStartup(MAKEWORD(2,2),&data) == 0);
    SOCKET listener=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP); assert(listener != INVALID_SOCKET);
    sockaddr_in address{}; address.sin_family=AF_INET; address.sin_addr.s_addr=htonl(INADDR_LOOPBACK);
    assert(bind(listener,reinterpret_cast<sockaddr*>(&address),sizeof(address)) == 0);
    assert(listen(listener,2) == 0);
    int length=sizeof(address);
    assert(getsockname(listener,reinterpret_cast<sockaddr*>(&address),&length) == 0);
    HANDLE port=CreateIoCompletionPort(INVALID_HANDLE_VALUE,nullptr,0,0); assert(port);
    SOCKET peer=INVALID_SOCKET;
    auto connectSession=[&]() {
        peer=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP); assert(peer != INVALID_SOCKET);
        assert(connect(peer,reinterpret_cast<sockaddr*>(&address),length) == 0);
        auto session=std::make_shared<GAMESESSION>(); session->object._id=7; session->_state=ST_ALLOC;
        session->_socket=accept(listener,nullptr,nullptr); assert(session->_socket != INVALID_SOCKET);
        assert(CreateIoCompletionPort(reinterpret_cast<HANDLE>(session->_socket),port,7,0));
        clients.set(7,session); return session;
    };
    auto take=[&]() {
        DWORD bytes=0; ULONG_PTR key=0; OVERLAPPED* over=nullptr;
        const bool success=GetQueuedCompletionStatus(port,&bytes,&key,&over,2000) != FALSE;
        assert(over && key == 7); // Failed cancellation still owns a non-null OVERLAPPED.
        return Completion{std::unique_ptr<OVER_EXP>(reinterpret_cast<OVER_EXP*>(over)),bytes,key,success};
    };
    {
        GAMESESSION unowned;
        assert(!unowned.registerRecv() && unowned.transportFailed);
        auto broken=std::make_shared<GAMESESSION>();
        std::weak_ptr<GAMESESSION> weak=broken;
        assert(!broken->registerRecv() && !broken->recvPending && broken->transportFailed);
        broken.reset(); assert(weak.expired()); // Immediate failure never waits for completion.
        broken=std::make_shared<GAMESESSION>(); weak=broken;
        sc_packet_avatar_login_ok packet{}; packet.size=sizeof(packet); packet.type=S2C_P_AVATAR_LOGIN_OK;
        broken->registerSend(&packet); assert(broken->transportFailed);
        broken.reset(); assert(weak.expired());
        GAMESESSION unownedSend;
        unownedSend.registerSend(&packet); assert(unownedSend.transportFailed);
        for(int invalid : {-1,static_cast<int>(BUF_SIZE)}) {
            auto invalidBuffer=std::make_shared<GAMESESSION>();
            invalidBuffer->_prevRemainData=invalid;
            assert(!invalidBuffer->registerRecv() && !invalidBuffer->recvPending && invalidBuffer->transportFailed);
        }
    }
    // Preserve every fragment in the next receive operation without OS access to embedded storage.
    {
        auto session=connectSession();
        cs_packet_move packet{}; packet.size=sizeof(packet); packet.type=C2S_P_MOVE;
        auto input=reinterpret_cast<const char*>(&packet);
        for(int i=0;i<sizeof(packet);++i) {
            assert(session->registerRecv());
            assert(!session->registerRecv() && !session->transportFailed); // No second outstanding receive.
            assert(send(peer,input+i,1,0) == 1);
            auto c=take(); assert(c.success && c.bytes == 1);
            assert(c.operation->owner == session && c.operation.get() != &session->_recvOver);
            assert(classifySessionCompletion(*c.operation,c.key,c.success,c.bytes) == SessionCompletion::Process);
            session->recvPending=false;
            session->_prevRemainData=i+1;
            assert(std::memcmp(c.operation->_sendBuf,input,i+1) == 0);
            std::memcpy(session->_recvOver._sendBuf,c.operation->_sendBuf,i+1);
        }
        session->_prevRemainData=0;
        session->closeTransport(); clients.set(7,nullptr); closesocket(peer);
    }
    // A successful queued send keeps its old owner alive even after slot replacement.
    {
        auto old=connectSession(); std::weak_ptr<GAMESESSION> weak=old;
        const auto oldConnection=old->connectionId;
        sc_packet_avatar_login_ok packet{}; packet.size=sizeof(packet); packet.type=S2C_P_AVATAR_LOGIN_OK;
        old->registerSend(&packet); assert(!old->transportFailed);
        auto replacement=std::make_shared<GAMESESSION>(); replacement->object._id=7;
        replacement->_recvOver._sendBuf[0]=42; clients.set(7,replacement);
        old.reset(); assert(!weak.expired());
        auto c=take(); assert(c.operation->_compType == OP_SEND && c.operation->owner->connectionId == oldConnection);
        assert(c.success && c.bytes == sizeof(packet));
        c.operation->owner->closeTransport();
        assert(classifySessionCompletion(*c.operation,c.key,c.success,c.bytes) == SessionCompletion::Ignore);
        assert(!replacement->disconnectStarted && replacement->_recvOver._sendBuf[0] == 42);
        c.operation.reset(); assert(weak.expired());
        clients.set(7,nullptr); closesocket(peer);
    }
    // Queue LOGIN_OK and cancel RECV; drain both only after reusing the numeric ID.
    for(int iteration=0;iteration<20;++iteration) {
        auto old=connectSession(); std::weak_ptr<GAMESESSION> weak=old;
        const auto oldConnection=old->connectionId;
        assert(old->registerRecv());
        auto login=new OVER_EXP; login->_compType=OP_LOGIN_OK; login->owner=old;
        assert(PostQueuedCompletionStatus(port,1,7,&login->_over));
        old->closeTransport();
        auto replacement=std::make_shared<GAMESESSION>(); replacement->object._id=7; replacement->_state=ST_ALLOC;
        replacement->_prevRemainData=3; replacement->_recvOver._sendBuf[0]=99;
        clients.set(7,replacement); old.reset(); assert(!weak.expired());
        int receives=0,logins=0;
        for(int i=0;i<2;++i) {
            auto c=take(); assert(c.operation->owner->connectionId == oldConnection);
            if(c.operation->_compType == OP_RECV) {
                ++receives;
                assert(!c.success || c.bytes == 0); // Cancelled/error or graceful zero-byte completion.
                c.operation->owner->recvPending=false;
            }
            else { assert(c.operation->_compType == OP_LOGIN_OK); ++logins; }
            assert(classifySessionCompletion(*c.operation,c.key,c.success,c.bytes) == SessionCompletion::Ignore);
        }
        assert(receives == 1 && logins == 1 && weak.expired());
        assert(!replacement->disconnectStarted && !replacement->transportFailed && !replacement->recvPending);
        assert(replacement->_state == ST_ALLOC && replacement->_prevRemainData == 3 && replacement->_recvOver._sendBuf[0] == 99);
        OVER_EXP fake; fake.owner=replacement; fake._compType=OP_SEND;
        assert(classifySessionCompletion(fake,8,false,0) == SessionCompletion::Ignore); // Wrong key cannot close owner.
        assert(classifySessionCompletion(fake,7,false,0) == SessionCompletion::Disconnect);
        assert(classifySessionCompletion(fake,7,true,0) == SessionCompletion::Disconnect);
        replacement->transportFailed=true;
        assert(classifySessionCompletion(fake,7,true,1) == SessionCompletion::Disconnect); // Shutdown wake still cleans up.
        clients.set(7,nullptr); closesocket(peer);
    }
    clients.clear(); closesocket(listener); CloseHandle(port); WSACleanup();
    std::cout << "Session IOCP ownership, cancellation, reuse and fragmentation checks passed\n";
}
