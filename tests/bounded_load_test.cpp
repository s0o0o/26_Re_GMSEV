#define GMSEV_BOUNDED_LOAD_TEST
#include "../stress_test/STRESS_TEST/bounded_load.cpp"
#include <cassert>
#include <thread>
#include <atomic>
using namespace bounded_load;
namespace {
void fixture(int count,int mode,bool mixed=false,bool drain=true) {
    WSADATA data{}; assert(WSAStartup(MAKEWORD(2,2),&data)==0);
    SOCKET listener=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP); assert(listener!=INVALID_SOCKET);
    sockaddr_in address{}; address.sin_family=AF_INET; address.sin_addr.s_addr=htonl(INADDR_LOOPBACK);
    assert(bind(listener,reinterpret_cast<sockaddr*>(&address),sizeof(address))==0);
    assert(listen(listener,SOMAXCONN)==0); int length=sizeof(address); assert(getsockname(listener,reinterpret_cast<sockaddr*>(&address),&length)==0);
    u_long nb=1; assert(ioctlsocket(listener,FIONBIO,&nb)==0);
    std::atomic_bool done{false};
    std::thread server([&] {
        std::vector<Peer> peers; Stats ignored;
        while(!done) {
            SOCKET client=accept(listener,nullptr,nullptr);
            if(client!=INVALID_SOCKET) { assert(ioctlsocket(client,FIONBIO,&nb)==0); Peer p; p.socket=client; p.id=static_cast<int>(peers.size()); peers.push_back(std::move(p)); }
            for(auto& p:peers) {
                if(p.socket==INVALID_SOCKET) continue;
                char bytes[1024]; int n=recv(p.socket,bytes,sizeof(bytes),0);
                if(n==0) { if(mode!=5) close(p,ignored); continue; }
                if(n<0 && WSAGetLastError()!=WSAEWOULDBLOCK) { close(p,ignored); continue; }
                if(n>0) p.input.insert(p.input.end(),bytes,bytes+n);
                while(p.input.size()>=2 && p.input.size()>=p.input[0]) {
                    unsigned size=p.input[0]; assert(size>=2);
                    if(p.input[1]==C2S_P_LOGIN) {
                        assert(size==sizeof(cs_packet_login));
                        if(mode==1) { sc_packet_login_fail fail{}; fail.size=sizeof(fail); fail.type=S2C_P_LOGIN_FAIL; queue(p,fail); }
                        else if(mode==2) { close(p,ignored); break; }
                        else if(mode==3) { /* Keep the connection silent until the deadline. */ }
                        else if(mode==4) { p.output={2,255}; }
                        else {
                            sc_packet_avatar_info info{}; info.size=sizeof(info); info.type=S2C_P_AVATAR_INFO; info.id=p.id; info.x=11; info.y=12; queue(p,info);
                            sc_packet_avatar_login_ok ok{}; ok.size=sizeof(ok); ok.type=S2C_P_AVATAR_LOGIN_OK; ok.id=p.id; queue(p,ok);
                        }
                    } else if(p.input[1]==C2S_P_MOVE) {
                        cs_packet_move move{}; std::memcpy(&move,p.input.data(),sizeof(move));
                        sc_packet_move reply{}; reply.size=sizeof(reply); reply.type=S2C_P_MOVE; reply.id=p.id; reply.x=12; reply.y=12; reply.move_time=move.move_time; queue(p,reply);
                    }
                    p.input.erase(p.input.begin(),p.input.begin()+size);
                }
                if(p.socket!=INVALID_SOCKET && !p.output.empty()) {
                    // Deliberately fragment every server packet into tiny writes.
                    int amount=std::min(3,static_cast<int>(p.output.size()));
                    int sent=send(p.socket,reinterpret_cast<const char*>(p.output.data()),amount,0);
                    if(sent>0) p.output.erase(p.output.begin(),p.output.begin()+sent);
                }
            }
            Sleep(1);
        }
        for(auto& p:peers) close(p,ignored);
    });
    Config c{count,900000000,2,mode==3 ? 2 : 15,ntohs(address.sin_port)};
    c.mixed=mixed; c.drain=drain;
    const int result=run(c); done=true; server.join(); closesocket(listener); WSACleanup();
    assert(result==(mode==0 ? 0 : 1));
}
}
int main() {
    int n=-1; assert(!number("",n) && !number("-1",n) && !number("2147483648",n));
    assert(number("2147483647",n) && n==INT_MAX);
    Config c; char a0[]="load",a1[]="300",a2[]="900000000",a3[]="2",a4[]="15",a5[]="3000";
    char* argv[]={a0,a1,a2,a3,a4,a5}; assert(config(6,argv,c));
    argv[1]=a2; assert(!config(6,argv,c)); argv[1]=a1; argv[2]=a2;
    Peer p; Stats s; p.input={2,255}; assert(!parse(p,s));
    // Two complete packets in one receive; wrong login-OK identity is rejected.
    p={}; sc_packet_avatar_info info{}; info.size=sizeof(info); info.type=S2C_P_AVATAR_INFO; info.id=1;
    sc_packet_avatar_login_ok ok{}; ok.size=sizeof(ok); ok.type=S2C_P_AVATAR_LOGIN_OK; ok.id=2;
    queue(p,info); queue(p,ok); p.input=std::move(p.output); assert(!parse(p,s));
    // Two sockets must not masquerade as two users with the same assigned session ID.
    s={}; p={}; ok.id=1; queue(p,info); queue(p,ok); p.input=std::move(p.output); assert(parse(p,s));
    assert(s.packets==2);
    p.sentMoves=1; sc_packet_move echo{}; echo.size=sizeof(echo); echo.type=S2C_P_MOVE; echo.id=1; echo.move_time=1;
    queue(p,echo); queue(p,echo); p.input=std::move(p.output); assert(parse(p,s));
    assert(p.moves==1 && s.moved==1 && s.packets==4); // Duplicate token is not an extra acknowledgement.
    Peer duplicate; queue(duplicate,info); queue(duplicate,ok); duplicate.input=std::move(duplicate.output);
    assert(!parse(duplicate,s) && s.accepted==1);
    fixture(300,0);
    fixture(30,0,true,true);
    fixture(3,0,true,false);
    fixture(1,5); // Peer refuses EOF: cleanup timeout must fail independently of the successful hold.
    for(int mode=1;mode<=4;++mode) fixture(1,mode);
    std::cout<<"Bounded load: 300 concurrent mock peers, fragmented packets, reject/close/timeout/invalid paths passed (no DB).\n";
}
