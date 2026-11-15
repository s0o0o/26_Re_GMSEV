// Real TCP silence fixture for ordinary client timeout/retry UI. No DB dependency.
#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <chrono>
#include <iostream>
#pragma comment(lib,"ws2_32.lib")
int main() {
    WSADATA data{}; if(WSAStartup(MAKEWORD(2,2),&data)) return 1;
    SOCKET listener=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
    BOOL exclusive=TRUE; setsockopt(listener,SOL_SOCKET,SO_EXCLUSIVEADDRUSE,reinterpret_cast<char*>(&exclusive),sizeof(exclusive));
    sockaddr_in address{}; address.sin_family=AF_INET;address.sin_addr.s_addr=htonl(INADDR_LOOPBACK);address.sin_port=htons(3000);
    if(bind(listener,reinterpret_cast<sockaddr*>(&address),sizeof(address)) || listen(listener,1)) return 2;
    u_long nonblocking=1;ioctlsocket(listener,FIONBIO,&nonblocking);
    std::cout<<"SILENT_FIXTURE_READY port=3000 no_DB no_response"<<std::endl;
    const auto until=std::chrono::steady_clock::now()+std::chrono::seconds(60);
    SOCKET peer=INVALID_SOCKET;unsigned bytes=0;
    while(std::chrono::steady_clock::now()<until) {
        if(peer==INVALID_SOCKET) peer=accept(listener,nullptr,nullptr);
        if(peer!=INVALID_SOCKET) {
            ioctlsocket(peer,FIONBIO,&nonblocking);char buffer[512];const auto count=recv(peer,buffer,sizeof(buffer),0);
            if(count>0) {bytes+=count;std::cout<<"SILENT_FIXTURE_RECEIVED bytes="<<bytes<<std::endl;}
            else if(count==0) break;
            else if(WSAGetLastError()!=WSAEWOULDBLOCK) break;
        }
        Sleep(10);
    }
    if(peer!=INVALID_SOCKET) closesocket(peer);closesocket(listener);WSACleanup();
    std::cout<<"SILENT_FIXTURE_END received="<<bytes<<" sent=0"<<std::endl;return bytes==58 ? 0 : 3;
}
