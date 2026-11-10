// Compile the production worker/parser, replacing only the DB link boundary.
#include <memory>
class GAMESESSION;
void workerTestObserve(const std::shared_ptr<GAMESESSION>&,const char*);
#define GMSEV_WORKER_TEST
#define main gmsev_production_main_not_executed
#include "../TP_2023192003/TP_2023192003/myserver.cpp"
#undef main
#include <cassert>
#include <condition_variable>
#include <map>
#include <set>

namespace fixture {
using Clock=std::chrono::steady_clock;
struct Snapshot {
    int id=-1,x=0,y=0,fragment=0,sectorRefs=0,otherViewRefs=0,account=0;
    std::size_t ownView=0,serial=0;
    std::uint64_t connection=0;
    STATE state=ST_FREE;
    std::weak_ptr<GAMESESSION> owner;
    std::unordered_set<int> visible;
};
std::mutex mutex;
std::condition_variable changed;
std::map<std::pair<std::uint64_t,std::string>,Snapshot> observed;
std::map<int,int> loads,saves;
std::size_t sequence=0;
std::size_t watcherLeaves=0,watcherEnters=0;

template<class Predicate>
Snapshot waitSnapshot(std::uint64_t connection,const char* event,Predicate predicate) {
    std::unique_lock<std::mutex> lock(mutex);
    const auto key=std::make_pair(connection,std::string(event));
    assert(changed.wait_for(lock,std::chrono::seconds(3),[&] {
        auto it=observed.find(key); return it != observed.end() && predicate(it->second);
    }));
    return observed.at(key);
}
int saveCount(int account) { std::lock_guard<std::mutex> lock(mutex); return saves[account]; }
int loadCount(int account) { std::lock_guard<std::mutex> lock(mutex); return loads[account]; }
struct Peer { SOCKET socket; Snapshot accepted; };
Peer connectPeer(const sockaddr_in& address,std::uint64_t after) {
    SOCKET socket=::socket(AF_INET,SOCK_STREAM,IPPROTO_TCP); assert(socket != INVALID_SOCKET);
    DWORD timeout=2000;
    assert(setsockopt(socket,SOL_SOCKET,SO_RCVTIMEO,reinterpret_cast<char*>(&timeout),sizeof(timeout)) == 0);
    assert(setsockopt(socket,SOL_SOCKET,SO_SNDTIMEO,reinterpret_cast<char*>(&timeout),sizeof(timeout)) == 0);
    assert(connect(socket,reinterpret_cast<const sockaddr*>(&address),sizeof(address)) == 0);
    std::unique_lock<std::mutex> lock(mutex);
    Snapshot accepted;
    assert(changed.wait_for(lock,std::chrono::seconds(3),[&] {
        for(const auto& entry : observed) if(entry.first.second == "accept" && entry.first.first > after) {
            accepted=entry.second; return true;
        }
        return false;
    }));
    assert(accepted.state == ST_ALLOC && accepted.fragment == 0 && accepted.ownView == 0 && accepted.sectorRefs == 0);
    return {socket,accepted};
}
void sendBytes(Peer& peer,const char* bytes,int length) {
    for(int sent=0;sent<length;) {
        int count=send(peer.socket,bytes+sent,length-sent,0); assert(count > 0); sent+=count;
    }
}
void readExact(SOCKET socket,char* bytes,int length) {
    for(int received=0;received<length;) {
        int count=recv(socket,bytes+received,length-received,0); assert(count > 0); received+=count;
    }
}
std::vector<char> receive(Peer& peer) {
    unsigned char size=0; readExact(peer.socket,reinterpret_cast<char*>(&size),1); assert(size >= 2);
    std::vector<char> packet(size); packet[0]=static_cast<char>(size);
    readExact(peer.socket,packet.data()+1,size-1);
    assert(is_valid_server_packet(reinterpret_cast<const std::uint8_t*>(packet.data()),packet.size()));
    if(peer.accepted.id == 0) {
        if(static_cast<std::uint8_t>(packet[1]) == S2C_P_LEAVE) ++watcherLeaves;
        if(static_cast<std::uint8_t>(packet[1]) == S2C_P_ENTER) ++watcherEnters;
    }
    return packet;
}
template<class Packet> Packet decode(const std::vector<char>& bytes) {
    Packet packet{}; assert(bytes.size() == sizeof(packet));
    std::memcpy(&packet,bytes.data(),sizeof(packet)); return packet;
}
std::vector<char> expect(Peer& peer,std::uint8_t type) {
    for(int i=0;i<128;++i) { auto packet=receive(peer); if(static_cast<std::uint8_t>(packet[1]) == type) return packet; }
    assert(false); return {};
}
cs_packet_login loginPacket(int account) {
    cs_packet_login packet{}; packet.size=sizeof(packet); packet.type=C2S_P_LOGIN; packet.inputId=account;
    strcpy_s(packet.name,"WorkerTest"); return packet;
}
void finishLogin(Peer& peer) {
    auto first=receive(peer);
    assert(static_cast<std::uint8_t>(first[1]) == S2C_P_AVATAR_INFO);
    auto info=decode<sc_packet_avatar_info>(first);
    assert(info.id == peer.accepted.id && info.x == clubhouse::spawnX && info.y == clubhouse::spawnY);
    assert(decode<sc_packet_avatar_login_ok>(expect(peer,S2C_P_AVATAR_LOGIN_OK)).id == info.id);
    for(int i=0;i<4;++i) expect(peer,S2C_P_MAPINFO);
    waitSnapshot(peer.accepted.connection,"login",[](const Snapshot& s) { return s.state == ST_INGAME && s.sectorRefs == 1; });
}
void login(Peer& peer,int account,bool fragmented) {
    auto packet=loginPacket(account); auto bytes=reinterpret_cast<const char*>(&packet);
    if(fragmented) {
        sendBytes(peer,bytes,1);
        waitSnapshot(peer.accepted.connection,"receive",[](const Snapshot& s) { return s.fragment == 1 && s.state == ST_ALLOC; });
        sendBytes(peer,bytes+1,4);
        waitSnapshot(peer.accepted.connection,"receive",[](const Snapshot& s) { return s.fragment == 5 && s.state == ST_ALLOC; });
        sendBytes(peer,bytes+5,sizeof(packet)-5);
    } else sendBytes(peer,bytes,sizeof(packet));
    finishLogin(peer);
}
Snapshot closePeer(Peer& peer) {
    assert(closesocket(peer.socket) == 0); peer.socket=INVALID_SOCKET;
    auto closed=waitSnapshot(peer.accepted.connection,"disconnect",[](const Snapshot& s) { return s.state == ST_FREE; });
    assert(closed.ownView == 0 && closed.sectorRefs == 0 && closed.otherViewRefs == 0);
    return closed;
}
void expectEnter(Peer& watcher,int id) {
    assert(decode<sc_packet_enter>(expect(watcher,S2C_P_ENTER)).id == id);
}
void expectLeave(Peer& watcher,int id) {
    assert(decode<sc_packet_leave>(expect(watcher,S2C_P_LEAVE)).id == id);
}
void movePair(Peer& peer,std::uint32_t token) {
    cs_packet_move packets[2]{};
    for(auto& p : packets) { p.size=sizeof(p); p.type=C2S_P_MOVE; }
    packets[0].direction=3; packets[0].move_time=token;
    packets[1].direction=2; packets[1].move_time=token+1;
    sendBytes(peer,reinterpret_cast<char*>(packets),sizeof(packets));
    auto ownMove=[&]() {
        for(int i=0;i<128;++i) {
            auto move=decode<sc_packet_move>(expect(peer,S2C_P_MOVE));
            if(move.id == peer.accepted.id) return move;
        }
        assert(false); return sc_packet_move{};
    };
    auto right=ownMove();
    auto left=ownMove();
    assert(right.id == peer.accepted.id && right.move_time == token && right.x == clubhouse::spawnX+1 && right.y == clubhouse::spawnY);
    assert(left.id == peer.accepted.id && left.move_time == token+1 && left.x == clubhouse::spawnX && left.y == clubhouse::spawnY);
    for(int i=0;i<4;++i) expect(peer,S2C_P_MAPINFO);
    expect(peer,S2C_P_MONSTER_NEAR);
}
} // namespace fixture

void workerTestObserve(const std::shared_ptr<GAMESESSION>& session,const char* event) {
    // Called on workers; stationary-player mode does not mutate coordinates after login.
    fixture::Snapshot snapshot;
    snapshot.id=session->object._id; snapshot.connection=session->connectionId; snapshot.state=session->_state;
    snapshot.x=session->object._x; snapshot.y=session->object._y; snapshot.fragment=session->_prevRemainData; snapshot.owner=session;
    snapshot.account=session->_userInputId;
    { std::lock_guard<std::mutex> lock(session->_vlL); snapshot.ownView=session->_viewList.size(); snapshot.visible=session->_viewList; }
    for(int x=0;x<MAX_SECTOR_X;++x) for(int y=0;y<MAX_SECTOR_Y;++y) {
        std::lock_guard<std::mutex> lock(sectorLock[x][y]); snapshot.sectorRefs+=static_cast<int>(sectors[x][y].count(snapshot.id));
    }
    for(const auto& entry : clients.snapshot()) if(entry.second && entry.second != session) {
        std::lock_guard<std::mutex> lock(entry.second->_vlL);
        snapshot.otherViewRefs+=static_cast<int>(entry.second->_viewList.count(snapshot.id));
    }
    { std::lock_guard<std::mutex> lock(fixture::mutex);
      snapshot.serial=++fixture::sequence; fixture::observed[{snapshot.connection,event}]=snapshot; }
    fixture::changed.notify_all();
}

// No game_database.cpp or ODBC library is linked into this executable.
bool initializeDB() { assert(false && "Production DB startup must not run"); return false; }
void finalizeDB() { assert(false && "Production DB shutdown must not run"); }
LoginLookupResult get_DB_login_info(int& id,int& x,int& y,int& hp,int& level,int& exp,int& seed,int& maxHp,int& damage) {
    std::lock_guard<std::mutex> lock(fixture::mutex); ++fixture::loads[id];
    if(id == 404) return LoginLookupResult::Error;
    x=clubhouse::spawnX; y=clubhouse::spawnY; hp=maxHp=10; level=1; exp=0; seed=10; damage=1;
    return LoginLookupResult::Found;
}
bool insert_new_user(int,const char*) { assert(false && "Fixture accounts already exist"); return false; }
bool update_user_info(int id,int,int,int,int,int,int,int,int) {
    std::lock_guard<std::mutex> lock(fixture::mutex); ++fixture::saves[id]; return true;
}

namespace fixture {
void stopWorkers(std::vector<std::thread>& workers) {
    assert(closesocket(gCliSocket) == 0); assert(closesocket(gServerSocket) == 0);
    for(std::size_t i=0;i<workers.size();++i) assert(PostQueuedCompletionStatus(g_hiocp,0,0,nullptr));
    for(auto& worker : workers) worker.join();
}
#ifdef GMSEV_MULTIWORKER_TEST
template<class Action> void parallelPeers(int count,Action action) {
    std::mutex gate; std::condition_variable ready;
    int arrived=0; bool start=false; std::vector<std::thread> threads;
    for(int i=0;i<count;++i) threads.emplace_back([&,i] {
        { std::unique_lock<std::mutex> lock(gate); ++arrived; ready.notify_all(); ready.wait(lock,[&] { return start; }); }
        action(i);
    });
    { std::unique_lock<std::mutex> lock(gate); ready.wait(lock,[&] { return arrived == count; }); start=true; ready.notify_all(); }
    for(auto& thread : threads) thread.join();
}
void runMultiworker(const sockaddr_in& address,std::vector<std::thread>& workers) {
    constexpr int count=16,rounds=5;
    std::vector<std::weak_ptr<GAMESESSION>> previous;
    for(int round=0;round<rounds;++round) {
        std::size_t before;
        { std::lock_guard<std::mutex> lock(mutex); before=sequence; }
        std::vector<Peer> peers(count);
        parallelPeers(count,[&](int i) {
            auto socket=::socket(AF_INET,SOCK_STREAM,IPPROTO_TCP); assert(socket != INVALID_SOCKET);
            DWORD timeout=3000;
            assert(setsockopt(socket,SOL_SOCKET,SO_RCVTIMEO,reinterpret_cast<char*>(&timeout),sizeof(timeout)) == 0);
            assert(setsockopt(socket,SOL_SOCKET,SO_SNDTIMEO,reinterpret_cast<char*>(&timeout),sizeof(timeout)) == 0);
            assert(connect(socket,reinterpret_cast<const sockaddr*>(&address),sizeof(address)) == 0);
            peers[i].socket=socket;
        });
        {
            std::unique_lock<std::mutex> lock(mutex);
            assert(changed.wait_for(lock,std::chrono::seconds(3),[&] {
                int accepted=0;
                for(const auto& entry : observed) if(entry.first.second == "accept" && entry.second.serial > before) {
                    assert(entry.second.state == ST_ALLOC && entry.second.ownView == 0 && entry.second.sectorRefs == 0);
                    ++accepted;
                }
                return accepted == count;
            }));
        }
        for(const auto& weak : previous) assert(weak.expired()); previous.clear();
        const int initial=(round == rounds-1) ? count/2 : count;
        parallelPeers(initial,[&](int i) {
            auto request=loginPacket(20000+round*100+i);
            sendBytes(peers[i],reinterpret_cast<char*>(&request),sizeof(request));
        });
        std::set<int> ids;
        auto finishLogins=[&](int begin,int end) {
        for(int i=begin;i<end;++i) {
            auto info=decode<sc_packet_avatar_info>(receive(peers[i]));
            assert(info.type == S2C_P_AVATAR_INFO && info.x == clubhouse::spawnX && info.y == clubhouse::spawnY);
            assert(info.id >= 0 && info.id < count && ids.insert(info.id).second);
            assert(decode<sc_packet_avatar_login_ok>(expect(peers[i],S2C_P_AVATAR_LOGIN_OK)).id == info.id);
            for(int chunk=0;chunk<4;++chunk) expect(peers[i],S2C_P_MAPINFO);
            std::unique_lock<std::mutex> lock(mutex);
            assert(changed.wait_for(lock,std::chrono::seconds(3),[&] {
                for(const auto& entry : observed) if(entry.first.second == "login" && entry.second.account == 20000+round*100+i) {
                    peers[i].accepted=entry.second; return true;
                }
                return false;
            }));
            assert(peers[i].accepted.id == info.id && peers[i].accepted.state == ST_INGAME);
        }
        };
        auto probe=[&](Peer& peer,const std::set<int>& live) {
            auto operation=new OVER_EXP; operation->_compType=OP_LOGIN_FAIL; operation->owner=peer.accepted.owner.lock();
            assert(operation->owner && PostQueuedCompletionStatus(g_hiocp,1,peer.accepted.id,&operation->_over));
            waitSnapshot(peer.accepted.connection,"probe",[&](const Snapshot& s) {
                return s.state == ST_INGAME && s.ownView == live.size()-1 && s.sectorRefs == 1 &&
                    !s.visible.count(peer.accepted.id) && std::all_of(s.visible.begin(),s.visible.end(),[&](int id) { return live.count(id) != 0; });
            });
        };
        finishLogins(0,initial);
        for(int i=0;i<initial;++i) {
            auto& peer=peers[i];
            std::set<int> seen;
            for(int n=0;n<initial-1;++n) {
                auto enter=decode<sc_packet_enter>(expect(peer,S2C_P_ENTER));
                assert(enter.id != peer.accepted.id && ids.count(enter.id) && seen.insert(enter.id).second);
            }
            probe(peer,ids);
        }
        auto close=[&](int i) { assert(closesocket(peers[i].socket) == 0); peers[i].socket=INVALID_SOCKET; };
        if(initial != count) {
            // Eight logouts race with eight logins, not just separate phases.
            parallelPeers(count,[&](int i) {
                if(i<initial) close(i);
                else {
                    auto request=loginPacket(20000+round*100+i);
                    sendBytes(peers[i],reinterpret_cast<char*>(&request),sizeof(request));
                }
            });
            for(int i=0;i<initial;++i) waitSnapshot(peers[i].accepted.connection,"disconnect",[](const Snapshot& s) { return s.state == ST_FREE; });
            finishLogins(initial,count);
            std::set<int> live;
            for(int i=initial;i<count;++i) live.insert(peers[i].accepted.id);
            for(int i=initial;i<count;++i) probe(peers[i],live);
            parallelPeers(count-initial,[&](int i) { close(i+initial); });
        } else {
            parallelPeers(count,close);
        }
        for(int i=0;i<count;++i) {
            auto closed=waitSnapshot(peers[i].accepted.connection,"disconnect",[](const Snapshot& s) { return s.state == ST_FREE; });
            assert(closed.ownView == 0 && closed.sectorRefs == 0 && closed.otherViewRefs == 0);
            assert(saveCount(20000+round*100+i) == 2); previous.push_back(closed.owner);
        }
        auto drained=[&] { for(const auto& weak : previous) if(weak.use_count() != 1) return false; return true; };
        const auto deadline=Clock::now()+std::chrono::seconds(3);
        while(!drained() && Clock::now()<deadline) std::this_thread::yield();
        assert(drained()); // Only FREE registry slots retain connections; no operation owns them.
    }
    stopWorkers(workers);
    for(const auto& entry : clients.snapshot()) assert(entry.second && entry.second->_state == ST_FREE);
    clients.clear(); for(const auto& weak : previous) assert(weak.expired());
    for(const auto& entry : observed) assert(entry.second.owner.expired());
}
#endif
}

int main() {
    using namespace fixture;
    WSADATA data{}; assert(WSAStartup(MAKEWORD(2,2),&data) == 0);
    initializeMap(); // No NPC Lua or timer threads.
    gServerSocket=WSASocket(AF_INET,SOCK_STREAM,0,nullptr,0,WSA_FLAG_OVERLAPPED); assert(gServerSocket != INVALID_SOCKET);
    sockaddr_in address{}; address.sin_family=AF_INET; address.sin_addr.s_addr=htonl(INADDR_LOOPBACK);
    assert(bind(gServerSocket,reinterpret_cast<sockaddr*>(&address),sizeof(address)) == 0);
    assert(listen(gServerSocket,SOMAXCONN) == 0);
    int length=sizeof(address); assert(getsockname(gServerSocket,reinterpret_cast<sockaddr*>(&address),&length) == 0);
    g_hiocp=CreateIoCompletionPort(INVALID_HANDLE_VALUE,nullptr,0,0); assert(g_hiocp);
    assert(CreateIoCompletionPort(reinterpret_cast<HANDLE>(gServerSocket),g_hiocp,9999,0));
    gCliSocket=WSASocket(AF_INET,SOCK_STREAM,0,nullptr,0,WSA_FLAG_OVERLAPPED); assert(gCliSocket != INVALID_SOCKET);
    gOverExp._compType=OP_ACCEPT;
    const auto accepted=AcceptEx(gServerSocket,gCliSocket,gOverExp._sendBuf,0,sizeof(sockaddr_in)+16,sizeof(sockaddr_in)+16,nullptr,&gOverExp._over);
    assert(accepted || WSAGetLastError() == WSA_IO_PENDING);
    std::vector<std::thread> workers;
#ifdef GMSEV_MULTIWORKER_TEST
    for(int i=0;i<4;++i) workers.emplace_back(workerThread,g_hiocp);
    runMultiworker(address,workers);
#else
    workers.emplace_back(workerThread,g_hiocp);
    auto watcher=connectPeer(address,0); assert(watcher.accepted.id == 0); login(watcher,10,false);
    std::uint64_t last=watcher.accepted.connection;
    // Unauthenticated MOVE + LOGIN in one TCP write must not produce a MOVE response first.
    {
        auto subject=connectPeer(address,last); last=subject.accepted.connection;
        cs_packet_move move{}; move.size=sizeof(move); move.type=C2S_P_MOVE; move.direction=3;
        auto request=loginPacket(100);
        std::array<char,sizeof(move)+sizeof(request)> bytes{};
        std::memcpy(bytes.data(),&move,sizeof(move)); std::memcpy(bytes.data()+sizeof(move),&request,sizeof(request));
        sendBytes(subject,bytes.data(),static_cast<int>(bytes.size())); finishLogin(subject);
        expectEnter(watcher,subject.accepted.id);
        auto repeated=loginPacket(9999); sendBytes(subject,reinterpret_cast<char*>(&repeated),sizeof(repeated));
        movePair(subject,42); assert(loadCount(9999) == 0 && saveCount(9999) == 0);
        closePeer(subject); expectLeave(watcher,subject.accepted.id);
        assert(saveCount(100) == 2);
    }
    // Disconnect an incomplete login, then reject a malformed size without DB saves.
    for(int prefix : {1,5,0,-1}) {
        auto subject=connectPeer(address,last); last=subject.accepted.connection;
        if(prefix <= 0) {
            const char bytes[]{static_cast<char>(prefix == 0 ? 1 : 2),static_cast<char>(prefix == 0 ? C2S_P_LOGIN : 255)};
            sendBytes(subject,bytes,sizeof(bytes));
            auto closed=waitSnapshot(last,"disconnect",[](const Snapshot& s) { return s.state == ST_FREE; });
            assert(closed.sectorRefs == 0 && closed.ownView == 0);
            closesocket(subject.socket); subject.socket=INVALID_SOCKET;
        } else {
            auto request=loginPacket(200); sendBytes(subject,reinterpret_cast<char*>(&request),prefix);
            waitSnapshot(last,"receive",[&](const Snapshot& s) { return s.fragment == prefix; }); closePeer(subject);
        }
        assert(saveCount(0) == 0 && saveCount(200) == 0);
    }
    {
        auto subject=connectPeer(address,last); last=subject.accepted.connection;
        auto fail=loginPacket(404); sendBytes(subject,reinterpret_cast<char*>(&fail),sizeof(fail));
        assert(static_cast<std::uint8_t>(receive(subject)[1]) == S2C_P_LOGIN_FAIL && saveCount(404) == 0);
        login(subject,300,true); expectEnter(watcher,subject.accepted.id);
        closePeer(subject); expectLeave(watcher,subject.accepted.id); assert(saveCount(300) == 2);
    }
    std::shared_ptr<GAMESESSION> previous;
    std::weak_ptr<GAMESESSION> previousWeak;
    for(int i=0;i<30;++i) {
        auto subject=connectPeer(address,last); last=subject.accepted.connection; assert(subject.accepted.id == 1);
        if(previous) {
            auto late=new OVER_EXP; late->_compType=OP_LOGIN_OK; late->owner=previous;
            auto lateSend=new OVER_EXP; lateSend->_compType=OP_SEND; lateSend->owner=previous;
            const auto previousConnection=previous->connectionId;
            assert(PostQueuedCompletionStatus(g_hiocp,1,1,&late->_over));
            assert(PostQueuedCompletionStatus(g_hiocp,0,1,&lateSend->_over)); previous.reset();
            waitSnapshot(previousConnection,"ignored",[](const Snapshot&) { return true; });
            const auto deadline=Clock::now()+std::chrono::seconds(3);
            while(!previousWeak.expired() && Clock::now()<deadline) std::this_thread::yield();
            assert(previousWeak.expired());
        }
        // The new slot is still ST_ALLOC when stale LOGIN_OK/zero-byte SEND are injected.
        login(subject,1000+i,true); expectEnter(watcher,1);
        movePair(subject,static_cast<std::uint32_t>(100+i*2));
        previous=subject.accepted.owner.lock(); assert(previous);
        previousWeak=previous;
        closePeer(subject); expectLeave(watcher,1); assert(saveCount(1000+i) == 2);
        movePair(watcher,static_cast<std::uint32_t>(1000+i*2)); // Surviving connection still responds.
        assert(watcherLeaves == static_cast<std::size_t>(i+3) && watcherEnters == static_cast<std::size_t>(i+3));
    }
    previous.reset();
    closePeer(watcher); assert(saveCount(10) == 2);
    const auto drainDeadline=Clock::now()+std::chrono::seconds(3);
    while((previousWeak.use_count() != 1 || watcher.accepted.owner.use_count() != 1) && Clock::now()<drainDeadline)
        std::this_thread::yield();
    // The free registry slot is now the only strong reference; no session operation remains.
    assert(previousWeak.use_count() == 1 && watcher.accepted.owner.use_count() == 1);
    // No active session remains. Stop the worker with an internal, non-network control post.
    stopWorkers(workers);
    for(const auto& entry : clients.snapshot()) assert(!entry.second || entry.second->_state == ST_FREE);
    clients.clear(); assert(previousWeak.expired() && watcher.accepted.owner.expired());
    for(const auto& entry : observed) assert(entry.second.owner.expired());
#endif
    CloseHandle(g_hiocp); WSACleanup();
#ifdef GMSEV_MULTIWORKER_TEST
    std::cout << "Production worker: 4 workers, 16 stationary peers x 5 batches including login/logout overlap passed (DB stub)\n";
#else
    std::cout << "Production worker: fragmented login, coalesced moves, cleanup and 30 reconnect cycles passed (DB stub, one worker)\n";
#endif
}
