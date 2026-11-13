// Console companion to the original GUI stress tool. No DB library is linked.
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <winsock2.h>
#include <ws2tcpip.h>
#include <chrono>
#include <vector>
#include <string>
#include <iostream>
#include <algorithm>
#include <climits>
#include <cstring>
#include <unordered_set>
#include "../../shared/protocol.hpp"
#include "../../shared/load_trace.hpp"
#pragma comment(lib,"ws2_32.lib")
namespace bounded_load {
using Clock=std::chrono::steady_clock;
struct Config { int count=0,start=0,hold=0,deadline=0,port=0; bool mixed=false,drain=true; };
bool number(const char* s,int& out) {
    if(!s || !*s) return false;
    int n=0; for(;*s;++s) { if(*s<'0'||*s>'9'||n>(INT_MAX-(*s-'0'))/10) return false; n=n*10+(*s-'0'); }
    out=n; return true;
}
bool config(int argc,char** argv,Config& c) {
    if(argc!=6 && argc!=8) return false;
    if(argc==8) {
        const std::string mode=argv[6],closing=argv[7];
        if((mode!="mixed" && mode!="separate") || (closing!="drain" && closing!="immediate")) return false;
        c.mixed=mode=="mixed"; c.drain=closing=="drain";
    }
    return number(argv[1],c.count) && number(argv[2],c.start) && number(argv[3],c.hold)
        && number(argv[4],c.deadline) && number(argv[5],c.port) && c.count>=1 && c.count<=300
        && c.start<=INT_MAX-c.count+1 && c.hold>=2 && c.hold<=30 && c.deadline>=2 && c.deadline<=60 && c.port>=1 && c.port<=65535;
}
struct Peer {
    SOCKET socket=INVALID_SOCKET; bool connecting=true,info=false,ok=false,logged=false;
    int id=-1,moves=0,sentMoves=0; std::vector<unsigned char> input,output;
    Clock::time_point opened{},nextMove{};
};
struct Stats {
    int attempts=0,accepted=0,rejected=0,errors=0,protocol=0,timeouts=0,closed=0,active=0,peak=0,moved=0;
    std::unordered_set<int> identities;
};
void close(Peer& p,Stats& s) {
    if(p.socket==INVALID_SOCKET) return;
    closesocket(p.socket); p.socket=INVALID_SOCKET; ++s.closed;
    if(p.logged) { p.logged=false; --s.active; }
}
template<class T> void queue(Peer& p,const T& packet) {
    const auto* bytes=reinterpret_cast<const unsigned char*>(&packet);
    p.output.insert(p.output.end(),bytes,bytes+sizeof(packet));
}
bool parse(Peer& p,Stats& s) {
    std::size_t offset=0;
    while(p.input.size()-offset>=2) {
        const auto* bytes=p.input.data()+offset; const unsigned size=bytes[0];
        if(size<PACKET_HEADER_SIZE) return false;
        if(p.input.size()-offset<size) break;
        if(!is_valid_server_packet(bytes,size)) return false;
        switch(bytes[1]) {
        case S2C_P_LOGIN_FAIL: ++s.rejected; close(p,s); return true;
        case S2C_P_AVATAR_INFO: {
            sc_packet_avatar_info info{}; std::memcpy(&info,bytes,sizeof(info));
            if(p.info || info.id<0 || info.id>=MAX_USER) return false;
            p.info=true; p.id=info.id; break;
        }
        case S2C_P_AVATAR_LOGIN_OK: {
            sc_packet_avatar_login_ok ok{}; std::memcpy(&ok,bytes,sizeof(ok));
            if(!p.info || p.ok || ok.id!=p.id || !s.identities.insert(ok.id).second) return false;
            p.ok=true; p.logged=true; p.nextMove=Clock::now(); ++s.accepted; ++s.active; s.peak=std::max(s.peak,s.active); break;
        }
        case S2C_P_MOVE: {
            sc_packet_move move{}; std::memcpy(&move,bytes,sizeof(move));
            // Tokens distinguish our own movement response from broadcasts/old packets.
            if(p.logged && move.id==p.id && move.move_time>0 && move.move_time<=static_cast<unsigned>(p.sentMoves)) {
                if(p.moves==0) ++s.moved; ++p.moves;
            }
            break;
        }
        default: break;
        }
        offset+=size;
    }
    p.input.erase(p.input.begin(),p.input.begin()+offset); return true;
}
struct DrainStats { int eof=0,errors=0,timeouts=0; std::uint64_t bytes=0; };
DrainStats finish(std::vector<Peer>& peers,Stats& stats,bool drain) {
    DrainStats result;
    std::cout<<"PHASE ms="<<load_trace::now()<<" event=shutdown_begin mode="<<(drain?"drain":"immediate")<<std::endl;
    for(auto& p:peers) if(p.socket!=INVALID_SOCKET) shutdown(p.socket,SD_SEND);
    if(drain) {
        const auto until=Clock::now()+std::chrono::seconds(5);
        while(Clock::now()<until) {
            bool pending=false;
            for(auto& p:peers) {
                if(p.socket==INVALID_SOCKET) continue;
                pending=true; char bytes[16384];
                const int n=recv(p.socket,bytes,sizeof(bytes),0);
                if(n>0) result.bytes+=n; // Do not feed cleanup packets into gameplay pass/fail metrics.
                else if(n==0) { ++result.eof; close(p,stats); }
                else if(WSAGetLastError()!=WSAEWOULDBLOCK) { ++result.errors; close(p,stats); }
            }
            if(!pending) break;
            Sleep(1);
        }
        for(const auto& p:peers) if(p.socket!=INVALID_SOCKET) ++result.timeouts;
    }
    for(auto& p:peers) close(p,stats);
    std::cout<<"PHASE ms="<<load_trace::now()<<" event=shutdown_end eof="<<result.eof<<" errors="<<result.errors<<" timeouts="<<result.timeouts<<std::endl;
    return result;
}
int run(const Config& c) {
    WSADATA data{}; if(WSAStartup(MAKEWORD(2,2),&data)) return 2;
    Stats s; std::vector<Peer> peers; peers.reserve(c.count);
    const auto began=Clock::now(); auto nextConnect=began,holdStart=Clock::time_point{};
    bool held=false; int activeAtEnd=0;
    std::cout<<"[LOAD] localhost:"<<c.port<<" count="<<c.count<<" account_start="<<c.start<<" hold_seconds="<<c.hold<<" login_deadline="<<c.deadline<<std::endl;
    std::cout<<"PHASE ms="<<load_trace::now()<<" event=ramp_begin mode="<<(c.mixed?"mixed":"separate")<<std::endl;
    while(Clock::now()-began<std::chrono::seconds(c.deadline+c.hold+2)) {
        const auto now=Clock::now();
        if(s.attempts<c.count && now>=nextConnect) {
            Peer p; p.opened=now; p.socket=::socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
            const int account=c.start+s.attempts++; nextConnect=now+std::chrono::milliseconds(10);
            u_long nonblock=1;
            if(p.socket==INVALID_SOCKET || ioctlsocket(p.socket,FIONBIO,&nonblock)) { ++s.errors; close(p,s); }
            else {
                sockaddr_in target{}; target.sin_family=AF_INET; target.sin_port=htons(static_cast<u_short>(c.port)); target.sin_addr.s_addr=htonl(INADDR_LOOPBACK);
                const int result=connect(p.socket,reinterpret_cast<sockaddr*>(&target),sizeof(target));
                const int error=result==0 ? 0 : WSAGetLastError();
                if(result!=0 && error!=WSAEWOULDBLOCK && error!=WSAEINPROGRESS) { ++s.errors; close(p,s); }
                else { p.connecting=result!=0; cs_packet_login login{}; login.size=sizeof(login); login.type=C2S_P_LOGIN; login.inputId=account; strcpy_s(login.name,"LoadTest"); queue(p,login); }
            }
            peers.push_back(std::move(p));
        }
        std::vector<WSAPOLLFD> descriptors; std::vector<std::size_t> indices;
        for(std::size_t i=0;i<peers.size();++i) {
            auto& p=peers[i]; if(p.socket==INVALID_SOCKET) continue;
            if(!p.logged && now-p.opened>=std::chrono::seconds(c.deadline)) { ++s.timeouts; close(p,s); continue; }
            // Keep login ramp separate from movement pressure; measure both phases explicitly.
            if(p.logged && (c.mixed || holdStart!=Clock::time_point{}) && now>=p.nextMove) {
                cs_packet_move move{}; move.size=sizeof(move); move.type=C2S_P_MOVE;
                move.direction=static_cast<std::uint8_t>((p.sentMoves%2) ? 2 : 3); move.move_time=++p.sentMoves;
                queue(p,move); p.nextMove=now+std::chrono::seconds(1);
            }
            descriptors.push_back({p.socket,static_cast<SHORT>(POLLRDNORM | ((p.connecting || !p.output.empty()) ? POLLWRNORM : 0)),0}); indices.push_back(i);
        }
        if(!descriptors.empty()) {
            const int polled=WSAPoll(descriptors.data(),static_cast<ULONG>(descriptors.size()),10);
            if(polled==SOCKET_ERROR) { ++s.errors; break; }
            for(std::size_t i=0;i<descriptors.size();++i) {
                auto& p=peers[indices[i]]; const auto events=descriptors[i].revents; if(!events) continue;
                if(events&(POLLERR|POLLNVAL)) { ++s.errors; close(p,s); continue; }
                if(p.connecting && (events&POLLWRNORM)) {
                    int error=0,len=sizeof(error);
                    if(getsockopt(p.socket,SOL_SOCKET,SO_ERROR,reinterpret_cast<char*>(&error),&len) || error) { ++s.errors; close(p,s); continue; }
                    p.connecting=false;
                }
                if(!p.connecting && (events&POLLWRNORM) && !p.output.empty()) {
                    const int n=send(p.socket,reinterpret_cast<const char*>(p.output.data()),static_cast<int>(p.output.size()),0);
                    if(n>0) p.output.erase(p.output.begin(),p.output.begin()+n);
                    else if(n==0 || WSAGetLastError()!=WSAEWOULDBLOCK) { ++s.errors; close(p,s); continue; }
                }
                if(events&(POLLRDNORM|POLLHUP)) {
                    char bytes[8192]; const int n=recv(p.socket,bytes,sizeof(bytes),0);
                    if(n>0) { p.input.insert(p.input.end(),bytes,bytes+n); if(!parse(p,s)) { ++s.protocol; close(p,s); } }
                    else if(n==0 || WSAGetLastError()!=WSAEWOULDBLOCK) { ++s.errors; close(p,s); }
                }
            }
        } else Sleep(1);
        if(s.active==c.count && holdStart==Clock::time_point{}) {
            holdStart=Clock::now(); std::cout<<"[LOAD] All clients logged in; hold begins"<<std::endl;
            std::cout<<"PHASE ms="<<load_trace::now()<<" event=hold_begin"<<std::endl;
        }
        if(holdStart!=Clock::time_point{} && s.active!=c.count) break;
        if(holdStart!=Clock::time_point{} && Clock::now()-holdStart>=std::chrono::seconds(c.hold)) { held=true; break; }
        if(s.errors || s.rejected || s.protocol || s.timeouts) break;
    }
    activeAtEnd=s.active;
    bool passed=held && s.accepted==c.count && s.active==c.count && s.moved==c.count && !s.errors && !s.rejected && !s.protocol && !s.timeouts;
    std::cout<<"PHASE ms="<<load_trace::now()<<" event=measurement_end passed="<<passed<<std::endl;
    const auto cleanup=finish(peers,s,c.drain);
    passed=passed && (!c.drain || (cleanup.errors==0 && cleanup.timeouts==0));
    WSACleanup();
    std::cout<<"RESULT {\"passed\":"<<(passed?"true":"false")<<",\"attempts\":"<<s.attempts<<",\"accepted\":"<<s.accepted<<",\"peak\":"<<s.peak<<",\"active_at_end\":"<<activeAtEnd<<",\"moved_clients\":"<<s.moved<<",\"rejected\":"<<s.rejected<<",\"transport_errors\":"<<s.errors<<",\"protocol_errors\":"<<s.protocol<<",\"timeouts\":"<<s.timeouts<<",\"hold_completed\":"<<(held?"true":"false")<<",\"closed\":"<<s.closed<<",\"drain_eof\":"<<cleanup.eof<<",\"drain_errors\":"<<cleanup.errors<<",\"drain_timeouts\":"<<cleanup.timeouts<<",\"drain_bytes\":"<<cleanup.bytes<<",\"elapsed_ms\":"<<std::chrono::duration_cast<std::chrono::milliseconds>(Clock::now()-began).count()<<"}"<<std::endl;
    return passed ? 0 : 1;
}
}
#ifndef GMSEV_BOUNDED_LOAD_TEST
int main(int argc,char** argv) {
    bounded_load::Config c;
    if(!bounded_load::config(argc,argv,c)) { std::cerr<<"Usage: bounded_load <1..300 clients> <first numeric account ID> <2..30 hold sec> <2..60 login deadline sec> <localhost port> [mixed|separate drain|immediate]\nWARNING: real servers may create/update these accounts.\n"; return 2; }
    return bounded_load::run(c);
}
#endif
