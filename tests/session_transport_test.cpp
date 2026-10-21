#include "../TP_2023192003/TP_2023192003/game_session.hpp"
#include <cassert>
#include <iostream>

int main() {
    WSADATA data{};
    assert(WSAStartup(MAKEWORD(2,2),&data) == 0);
    sc_packet_avatar_login_ok packet{};
    packet.size = sizeof(packet); packet.type = S2C_P_AVATAR_LOGIN_OK;
    {
        GAMESESSION broken;
        broken._socket = INVALID_SOCKET;
        broken.registerSend(&packet);
        assert(broken.transportFailed);
        for (int i=0;i<100;++i) broken.registerSend(&packet);
        assert(!broken.registerRecv());
    }
    {
        GAMESESSION broken;
        broken._socket = INVALID_SOCKET;
        assert(!broken.registerRecv());
        assert(broken.transportFailed);
        broken.registerSend(&packet);
    }
    SOCKET listener = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
    assert(listener != INVALID_SOCKET);
    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    assert(bind(listener,reinterpret_cast<sockaddr*>(&address),sizeof(address)) == 0);
    assert(listen(listener,1) == 0);
    int length = sizeof(address);
    assert(getsockname(listener,reinterpret_cast<sockaddr*>(&address),&length) == 0);
    SOCKET peer = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
    assert(connect(peer,reinterpret_cast<sockaddr*>(&address),length) == 0);
    GAMESESSION session;
    session._socket = accept(listener,nullptr,nullptr);
    assert(session._socket != INVALID_SOCKET);
    HANDLE port = CreateIoCompletionPort(reinterpret_cast<HANDLE>(session._socket),nullptr,123,0);
    assert(port != nullptr);
    assert(session.registerRecv()); // Pending receive is not a failure.
    assert(!session.transportFailed);
    const char input[] = "test";
    assert(send(peer,input,sizeof(input),0) == sizeof(input));
    DWORD bytes = 0;
    ULONG_PTR key = 0;
    OVERLAPPED* over = nullptr;
    assert(GetQueuedCompletionStatus(port,&bytes,&key,&over,2000));
    assert(over == &session._recvOver._over && bytes == sizeof(input) && key == 123);
    session.registerSend(&packet);
    assert(!session.transportFailed);
    assert(GetQueuedCompletionStatus(port,&bytes,&key,&over,2000));
    assert(bytes == sizeof(packet));
    auto sendOperation = reinterpret_cast<OVER_EXP*>(over);
    assert(sendOperation->_compType == OP_SEND);
    delete sendOperation;
    DWORD timeout = 2000;
    assert(setsockopt(peer,SOL_SOCKET,SO_RCVTIMEO,reinterpret_cast<const char*>(&timeout),sizeof(timeout)) == 0);
    char output[sizeof(packet)]{};
    int received = 0;
    while (received < sizeof(packet)) {
        const int count = recv(peer,output+received,sizeof(packet)-received,0);
        assert(count > 0);
        received += count;
    }
    assert(static_cast<unsigned char>(output[1]) == S2C_P_AVATAR_LOGIN_OK);
    session.disconnectStarted = true;
    session.registerSend(&packet);
    assert(!session.registerRecv());
    closesocket(session._socket);
    closesocket(peer);
    closesocket(listener);
    CloseHandle(port);
    WSACleanup();
    std::cout << "Session transport IOCP and immediate failure checks passed\n";
}
