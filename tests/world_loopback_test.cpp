#include "../TP_2023192003/TP_2023192003/world_events.hpp"
#include "../TP_2023192003/TP_2023192003/monster_lifecycle.hpp"
#include <cassert>
#include <vector>

// Two real TCP peers; production send/IOCP and packet writers, no DB/game server loop.
std::vector<char> receivePacket(SOCKET peer) {
    unsigned char size=0;
    assert(recv(peer,reinterpret_cast<char*>(&size),1,0) == 1 && size >= 2);
    std::vector<char> bytes(size); bytes[0]=static_cast<char>(size);
    int received=1;
    while(received < size) {
        int count=recv(peer,bytes.data()+received,size-received,0);
        assert(count > 0); received+=count;
    }
    assert(is_valid_server_packet(reinterpret_cast<const std::uint8_t*>(bytes.data()),bytes.size()));
    return bytes;
}
int main() {
    WSADATA data{}; assert(WSAStartup(MAKEWORD(2,2),&data) == 0);
    SOCKET listener=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP); assert(listener != INVALID_SOCKET);
    sockaddr_in address{}; address.sin_family=AF_INET; address.sin_addr.s_addr=htonl(INADDR_LOOPBACK);
    assert(bind(listener,reinterpret_cast<sockaddr*>(&address),sizeof(address)) == 0);
    assert(listen(listener,2) == 0);
    int length=sizeof(address);
    assert(getsockname(listener,reinterpret_cast<sockaddr*>(&address),&length) == 0);
    HANDLE port=CreateIoCompletionPort(INVALID_HANDLE_VALUE,nullptr,0,0); assert(port);
    SOCKET peers[2]{};
    for(int i=0;i<2;++i) {
        peers[i]=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP); assert(peers[i] != INVALID_SOCKET);
        assert(connect(peers[i],reinterpret_cast<sockaddr*>(&address),length) == 0);
        auto s=std::make_shared<GAMESESSION>(); s->object._id=i; s->_state=ST_INGAME;
        s->_socket=accept(listener,nullptr,nullptr); assert(s->_socket != INVALID_SOCKET);
        assert(CreateIoCompletionPort(reinterpret_cast<HANDLE>(s->_socket),port,i,0));
        clients[i]=s;
        DWORD timeout=2000;
        assert(setsockopt(peers[i],SOL_SOCKET,SO_RCVTIMEO,reinterpret_cast<const char*>(&timeout),sizeof(timeout)) == 0);
    }
    auto drain=[&](int count) {
        for(int i=0;i<count;++i) {
            DWORD bytes=0; ULONG_PTR key=0; OVERLAPPED* over=nullptr;
            assert(GetQueuedCompletionStatus(port,&bytes,&key,&over,2000));
            assert(over && bytes > 0 && key < 2);
            auto operation=reinterpret_cast<OVER_EXP*>(over);
            assert(operation->_compType == OP_SEND); delete operation;
        }
    };
    auto monster=std::make_shared<GAMESESSION>(); monster->object._id=MAX_USER;
    clients[MAX_USER]=monster;
    const std::unordered_set<int> audience{0,1};
    monster->object._hp=0; markMonsterDead(monster->monster,1000);
    publishCombat(*monster,0,2,true,audience); drain(4);
    for(int packet=0;packet<2;++packet) {
        auto a=receivePacket(peers[0]),b=receivePacket(peers[1]);
        assert(a == b);
        assert(static_cast<unsigned char>(a[1]) == (packet == 0 ? S2C_P_ATTACK_PtoN : S2C_P_MONSTER_DIE));
    }
    assert(reviveMonster(monster->object,monster->monster,31000));
    forWorldPlayers(audience,[&](GAMESESSION& s) { s.sendAddPlayerPacket(MAX_USER); }); drain(2);
    auto a=receivePacket(peers[0]),b=receivePacket(peers[1]); assert(a == b);
    assert(static_cast<unsigned char>(a[1]) == S2C_P_ENTER);
    auto leaving=clients[0]; clients[1]->_viewList.insert(0);
    leaving->disconnectStarted=true; leaving->transportFailed=true;
    detachWorldPresence(*leaving,[](int) {}); drain(1);
    assert(static_cast<unsigned char>(receivePacket(peers[1])[1]) == S2C_P_LEAVE);
    for(int i=0;i<2;++i) { closesocket(clients[i]->_socket); closesocket(peers[i]); }
    closesocket(listener); CloseHandle(port); WSACleanup();
}
