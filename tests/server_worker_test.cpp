#include "legacy_position_fixture.hpp"
// Compile the production worker/parser, replacing only the DB link boundary.
#include <memory>
class GAMESESSION;
void workerTestObserve(const std::shared_ptr<GAMESESSION>&,const char*);
#define GMSEV_WORKER_TEST
#define main gmsev_production_main_not_executed
#include "../TP_2023192003/TP_2023192003/myserver.cpp"
#undef main
#include "../TP_2023192003/TP_2023192003/database_contract.hpp"
#include <cassert>
#include <condition_variable>
#include <map>
#include <set>

namespace fixture {
using Clock=std::chrono::steady_clock;
#if defined(GMSEV_CHUNK_TEST)
constexpr int spawnX=47,spawnY=9; // Local (15,9), adjacent to the first chunk boundary.
#elif defined(GMSEV_TRAVEL_TEST)
constexpr int spawnX=clubhouse::spawnX,spawnY=clubhouse::spawnY;
#elif defined(GMSEV_SECTOR_TEST)
constexpr int spawnX=19,spawnY=16; // Right/left crosses Clubhouse's LOCAL sector boundary at x=20.
#elif defined(GMSEV_WORLD_TEST) || defined(GMSEV_LUA_AI_TEST)
constexpr int spawnX=39,spawnY=7; // Field regression coordinates; now both lie in local sector (0,0).
#else
constexpr int spawnX=clubhouse::spawnX,spawnY=clubhouse::spawnY;
#endif
struct Snapshot {
    int id=-1,x=0,y=0,fragment=0,sectorRefs=0,otherViewRefs=0,account=0;
    std::size_t ownView=0,serial=0;
    std::uint64_t connection=0;
    STATE state=ST_FREE;
    std::weak_ptr<GAMESESSION> owner;
    std::unordered_set<int> visible;
    int hp=0,seeds=0,targetHp=-1,totalSeeds=0;
    bool targetDead=false;
    bool active=false,chasing=false,scriptDisabled=false;
    std::uint64_t generation=0;
};
std::mutex mutex;
std::condition_variable changed;
std::map<std::pair<std::uint64_t,std::string>,Snapshot> observed;
std::map<int,int> loads,saves;
std::map<int,GameObjectState> savedState;
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
struct Peer { SOCKET socket; Snapshot accepted; std::vector<sc_packet_attack_success> combat; int deaths=0,chats=0; };
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
    if(static_cast<std::uint8_t>(packet[1]) == S2C_P_ATTACK_PtoN) {
        sc_packet_attack_success hit{}; std::memcpy(&hit,packet.data(),sizeof(hit)); peer.combat.push_back(hit);
    }
    if(static_cast<std::uint8_t>(packet[1]) == S2C_P_MONSTER_DIE) ++peer.deaths;
    if(static_cast<std::uint8_t>(packet[1]) == S2C_P_CHAT) ++peer.chats;
    if(peer.accepted.id == 0) {
        if(static_cast<std::uint8_t>(packet[1]) == S2C_P_LEAVE) ++watcherLeaves;
        if(static_cast<std::uint8_t>(packet[1]) == S2C_P_ENTER) ++watcherEnters;
    }
    return packet;
}
template<class Packet> Packet decode(const std::vector<char>& bytes) {
    Packet packet{}; assert(bytes.size() == sizeof(packet));
    std::memcpy(&packet,bytes.data(),sizeof(packet));
    // Existing gameplay assertions use legacy state; validate local wire before adapting.
    if constexpr(std::is_same_v<Packet,sc_packet_avatar_info> || std::is_same_v<Packet,sc_packet_move> || std::is_same_v<Packet,sc_packet_enter>) {
        assert(packet_position::valid(packet)); assert(packet_position::restoreLegacy(packet));
    }
    return packet;
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
    assert(info.id == peer.accepted.id && info.x == spawnX && info.y == spawnY);
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
    assert(right.id == peer.accepted.id && right.move_time == token && right.x == spawnX+1 && right.y == spawnY);
    assert(left.id == peer.accepted.id && left.move_time == token+1 && left.x == spawnX && left.y == spawnY);
    for(int i=0;i<4;++i) expect(peer,S2C_P_MAPINFO);
    expect(peer,S2C_P_MONSTER_NEAR);
}
} // namespace fixture

void workerTestObserve(const std::shared_ptr<GAMESESSION>& session,const char* event) {
    WorldStateLock stateLock;
    fixture::Snapshot snapshot;
    snapshot.id=session->object._id; snapshot.connection=session->connectionId; snapshot.state=session->_state;
    snapshot.x=testLegacy(session->object).x; snapshot.y=testLegacy(session->object).y; snapshot.fragment=session->_prevRemainData; snapshot.owner=session;
    snapshot.account=session->_userInputId;
    snapshot.hp=session->object._hp; snapshot.seeds=session->object._seedNum;
    snapshot.active=session->monster._isActive; snapshot.chasing=session->monster._isMonsterChase;
    snapshot.scriptDisabled=session->monster.scriptDisabled; snapshot.generation=session->monster.generation;
    if(auto target=clients.get(MAX_USER)) { snapshot.targetHp=target->object._hp; snapshot.targetDead=target->monster._isDie; }
    for(const auto& entry : clients.snapshot()) if(entry.second && entry.first < MAX_USER && entry.second->_state == ST_INGAME)
        snapshot.totalSeeds+=entry.second->object._seedNum;
    { std::lock_guard<std::mutex> lock(session->_vlL); snapshot.ownView=session->_viewList.size(); snapshot.visible=session->_viewList; }
    snapshot.sectorRefs=worldSectors.memberships(snapshot.id);
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
    assert(!WorldStateLock::heldByCurrentThread());
    std::lock_guard<std::mutex> lock(fixture::mutex); ++fixture::loads[id];
    if(id == 404) return LoginLookupResult::Error;
    if(id == 405) {
        database_contract::Values wrongAccount{{406,11,12,10,1,0,10,10,1}},output{};
        database_contract::Lengths lengths; lengths.fill(4);
        if(!database_contract::acceptRow(id,wrongAccount,lengths,output)) return LoginLookupResult::Error;
        assert(false && "Wrong-account row must be rejected");
    }
    x=fixture::spawnX; y=fixture::spawnY; hp=maxHp=10; level=1; exp=0; seed=10; damage=1;
    return LoginLookupResult::Found;
}
bool insert_new_user(int,const char*) { assert(false && "Fixture accounts already exist"); return false; }
bool update_user_info(int id,int x,int y,int seed,int hp,int level,int exp,int maxHp,int damage) {
    assert(!WorldStateLock::heldByCurrentThread());
    std::lock_guard<std::mutex> lock(fixture::mutex); ++fixture::saves[id];
    auto& saved=fixture::savedState[id]; setTestLegacyX(saved,static_cast<short>(x)); setTestLegacyY(saved,static_cast<short>(y));
    saved._seedNum=static_cast<short>(seed); saved._hp=static_cast<short>(hp); saved._level=static_cast<short>(level);
    saved._exp=static_cast<short>(exp); saved._maxHp=static_cast<short>(maxHp); saved._damage=static_cast<short>(damage);
    return true;
}

namespace fixture {
void stopWorkers(std::vector<std::thread>& workers) {
    assert(closesocket(gCliSocket) == 0); assert(closesocket(gServerSocket) == 0);
    for(std::size_t i=0;i<workers.size();++i) assert(PostQueuedCompletionStatus(g_hiocp,0,0,nullptr));
    for(auto& worker : workers) worker.join();
}
#ifdef GMSEV_MULTIWORKER_TEST
void runSectorIndex(const sockaddr_in& address,std::vector<std::thread>& workers) {
    auto watcher=connectPeer(address,0); login(watcher,80000,true);
    auto peer=connectPeer(address,watcher.accepted.connection); login(peer,80001,true); expectEnter(watcher,peer.accepted.id);
    auto subject=peer.accepted.owner.lock(); auto observer=watcher.accepted.owner.lock(); assert(subject && observer);
    cs_packet_move move{}; move.size=sizeof(move); move.type=C2S_P_MOVE; move.direction=3; move.move_time=8080;
    sendBytes(peer,reinterpret_cast<char*>(&move),sizeof(move));
    auto response=decode<sc_packet_move>(expect(peer,S2C_P_MOVE)); assert(response.id==peer.accepted.id && response.x==20 && response.y==16);
    { WorldStateLock lock;
      region_sectors::Key key; assert(worldSectors.keyFor(subject->object.location(),key));
      assert(key.region==world_regions::Id::Clubhouse && key.x==1 && key.y==0);
      assert(worldSectors.memberships(subject->object._id)==1 && subject->getNearBySessions().count(observer->object._id));
      const auto actual=worldSectors.nearby(subject->object.location(),0);
      assert(actual.count(subject->object._id) && !actual.count(observer->object._id));
    }
    expect(peer,S2C_P_MONSTER_NEAR); // Drain map/visibility before the next command.
    { WorldStateLock lock; setTestLegacyX(subject->object,10); setTestLegacyY(subject->object,13); subject->updateSector(20,16); }
    move.direction=1; move.move_time=8081; sendBytes(peer,reinterpret_cast<char*>(&move),sizeof(move));
    response=decode<sc_packet_move>(expect(peer,S2C_P_MOVE)); assert(response.x==42 && response.y==10);
    expectLeave(watcher,peer.accepted.id); expect(peer,S2C_P_MONSTER_NEAR);
    { WorldStateLock lock;
      setTestLegacyX(observer->object,10); setTestLegacyY(observer->object,10); observer->updateSector(19,16);
      const auto club=worldSectors.nearby({world_regions::Id::Clubhouse,10,10},0);
      const auto field=worldSectors.nearby({world_regions::Id::SunnyPeak,10,10},0);
      assert(club==std::unordered_set<int>{observer->object._id} && field==std::unordered_set<int>{subject->object._id});
      assert(subject->object.location().x==observer->object.location().x && subject->object.location().y==observer->object.location().y);
      assert(!isCanSee(subject->object._id,observer->object._id));
    }
    // Exercise the real respawn service's re-registration, using an old bucket in another region.
    auto revived=std::make_shared<GAMESESSION>(); revived->object._id=MAX_USER;
    setTestLegacyX(revived->object,21); setTestLegacyY(revived->object,16); revived->object._hp=0; revived->_state=ST_INGAME;
    revived->monster._monType=1; revived->monster._isDie=true; revived->monster.respawnAt=1;
    auto script=npc_script::loadText("function set_uid(id) end; function choose_direction() return 0 end; function event_player_move(p,c) end",MAX_USER,{API_get_x,API_get_y,API_SendMessage});
    assert(script); revived->monster.script=std::move(script.state);
    { WorldStateLock lock;
      clients.set(MAX_USER,revived); assert(worldSectors.put(MAX_USER,revived->object.location()));
      serviceRespawns();
      assert(!revived->monster._isDie && revived->object._hp==5 && worldSectors.memberships(MAX_USER)==1);
      assert(worldSectors.nearby(revived->object.location(),0).count(MAX_USER));
      assert(!worldSectors.nearby({world_regions::Id::Clubhouse,21,16},0).count(MAX_USER));
    }
    const auto enter=decode<sc_packet_enter>(expect(peer,S2C_P_ENTER)); assert(enter.id==MAX_USER && enter.hp==5);
    waitSnapshot(revived->connectionId,"notify",[](const Snapshot&) { return true; });
    { WorldStateLock lock; worldSectors.remove(MAX_USER); clients.set(MAX_USER,nullptr); }
    closePeer(peer); assert(worldSectors.memberships(peer.accepted.id)==0 && worldSectors.memberships(watcher.accepted.id)==1);
    closePeer(watcher); subject.reset(); observer.reset();
    const auto deadline=Clock::now()+std::chrono::seconds(3);
    while((peer.accepted.owner.use_count()!=1 || watcher.accepted.owner.use_count()!=1) && Clock::now()<deadline) std::this_thread::yield();
    assert(peer.accepted.owner.use_count()==1 && watcher.accepted.owner.use_count()==1 && worldSectors.snapshot().empty() && worldSectors.bucketCount()==0);
    stopWorkers(workers); clients.clear();
    assert(peer.accepted.owner.expired() && watcher.accepted.owner.expired());
}
void runTown(const sockaddr_in& address,std::vector<std::thread>& workers) {
    auto peer=connectPeer(address,0); login(peer,70000,true);
    auto subject=peer.accepted.owner.lock(); assert(subject);
    std::set<int> entered;
    for(int i=0;i<3;++i) {
        const auto npc=decode<sc_packet_enter>(expect(peer,S2C_P_ENTER));
        const auto definition=town_npcs::byType(npc.monType);
        assert(definition && npc.id==definition->id && npc.x==definition->x && npc.y==definition->y && std::strcmp(npc.name,definition->name)==0);
        entered.insert(npc.id);
    }
    assert(entered.size()==3);
    // Inject clock time into the same production service called by OP_WORLD_TICK.
    const auto beforeMoves=peer.accepted.owner.lock()->object._seedNum;
    int autonomousMoves=0;
    for(std::uint64_t tick=1000;tick<=80000;tick+=1000) {
        WorldStateLock lock;
        const auto previous=clients.get(town_npcs::baseId)->object;
        serviceTownResidents(tick);
        const auto current=clients.get(town_npcs::baseId)->object;
        if(testLegacy(previous).x!=testLegacy(current).x || testLegacy(previous).y!=testLegacy(current).y) ++autonomousMoves;
        for(const auto& definition : town_npcs::residents) {
            const auto npc=clients.get(definition.id);
            assert(town_behaviour::allowed(definition,testLegacy(npc->object).x,testLegacy(npc->object).y));
            assert(!npc->monster._isActive && !npc->monster.script);
        }
    }
    assert(autonomousMoves>0 && beforeMoves==10);
    // Drain all move packets up to an ordered marker and confirm the client-facing stream moved.
    { WorldStateLock lock; subject->sendChatPacket(subject->object._id,"WALK-END"); }
    int receivedMoves=0; bool walkMarker=false;
    for(int i=0;i<512 && !walkMarker;++i) {
        auto packet=receive(peer);
        if(static_cast<std::uint8_t>(packet[1])==S2C_P_MOVE) {
            const auto move=decode<sc_packet_move>(packet); assert(town_npcs::isResident(clients.get(move.id)->monster._monType)); ++receivedMoves;
        }
        if(static_cast<std::uint8_t>(packet[1])==S2C_P_CHAT) walkMarker=std::strcmp(decode<sc_packet_chat>(packet).message,"WALK-END")==0;
    }
    assert(walkMarker && receivedMoves>=autonomousMoves);
    // Put a viewer exactly at Pashmina's visibility boundary; force a valid right step.
    { WorldStateLock lock;
      const auto oldX=testLegacy(subject->object).x,oldY=testLegacy(subject->object).y;
      setTestLegacyX(subject->object,6); setTestLegacyY(subject->object,16); subject->updateSector(oldX,oldY);
      for(const auto& definition : town_npcs::residents) {
          auto npc=clients.get(definition.id); const auto x=testLegacy(npc->object).x,y=testLegacy(npc->object).y;
          setTestLegacyX(npc->object,definition.x); setTestLegacyY(npc->object,definition.y); npc->updateSector(x,y);
          npc->monster.residentBehaviour.next=1000000;
      }
      // Isolate visibility from occupancy: keep Snoozer away from the forced boundary step.
      auto sleeper=clients.get(town_npcs::baseId+2); const auto sx=testLegacy(sleeper->object).x,sy=testLegacy(sleeper->object).y;
      setTestLegacyX(sleeper->object,19); setTestLegacyY(sleeper->object,12); sleeper->updateSector(sx,sy);
    }
    cs_packet_move boundary{}; boundary.size=sizeof(boundary); boundary.type=C2S_P_MOVE; boundary.direction=2;
    sendBytes(peer,reinterpret_cast<char*>(&boundary),sizeof(boundary));
    expect(peer,S2C_P_MONSTER_NEAR); // MOVE/map/initial visibility updates precede this barrier.
    { WorldStateLock lock;
      auto walker=clients.get(town_npcs::baseId+1);
      assert(subject->_viewList.count(walker->object._id));
      std::uint32_t seed=1;
      while((((seed*1664525u+1013904223u)>>8)%5)!=0) ++seed;
      walker->monster.residentBehaviour={seed,90000};
      serviceTownResidents(90000);
      assert(testLegacy(walker->object).x==13 && testLegacy(walker->object).y==15 && !subject->_viewList.count(walker->object._id));
      serviceTownResidents(94000); assert(testLegacy(walker->object).x==13); // Nobody sees it: pause, not catch-up.
    }
    assert(decode<sc_packet_leave>(expect(peer,S2C_P_LEAVE)).id==town_npcs::baseId+1);
    // Reset positions for the existing role/portal fixture, using proper sector updates.
    { WorldStateLock lock;
      for(const auto& definition : town_npcs::residents) {
          auto npc=clients.get(definition.id); const auto x=testLegacy(npc->object).x,y=testLegacy(npc->object).y;
          setTestLegacyX(npc->object,definition.x); setTestLegacyY(npc->object,definition.y); npc->updateSector(x,y);
      }
      const auto oldX=testLegacy(subject->object).x,oldY=testLegacy(subject->object).y;
      setTestLegacyX(subject->object,spawnX); setTestLegacyY(subject->object,spawnY); subject->updateSector(oldX,oldY);
    }
    movePair(peer,7050); // Player re-approach must restore ENTER for the previously hidden resident.
    { WorldStateLock lock; assert(subject->_viewList.count(town_npcs::baseId+1)); }
    auto resident=clients.get(town_npcs::baseId); assert(resident);
    { WorldStateLock lock;
      for(const auto& definition : town_npcs::residents) {
          auto npc=clients.get(definition.id); assert(npc && !npc->monster.script && !npc->monster._isActive);
          wakeUpMonster(definition.id,peer.accepted.id);
          assert(!npc->monster._isActive && !npc->monster.scriptDisabled);
      }
      resident->monster._isActive=true; // Even erroneous activation must not cause movement/timer work.
      doMonsterRandomMoving(resident->object._id);
      assert(testLegacy(resident->object).x==12 && testLegacy(resident->object).y==9);
      const int idx=get_timer_queue_index(resident->object._id);
      { std::lock_guard<std::mutex> timers(timerLocks[idx]);
        timerQueues[idx].push(TIMER_EVENT{resident->object._id,Clock::now(),EV_RANDOM_MOVE,0,resident->monster.generation.load()}); }
      assert(dispatchDueTimer(idx,Clock::now()) && timerQueues[idx].empty()); resident->monster._isActive=false; }
    auto attack=[&](AttackFeedback expected) {
        cs_packet_attack request{}; request.size=sizeof(request); request.type=C2S_P_ATTACK; request.damage=9999;
        sendBytes(peer,reinterpret_cast<char*>(&request),sizeof(request));
        auto feedback=decode<sc_packet_attack_feedback>(expect(peer,S2C_P_ATTACK_FEEDBACK));
        assert(feedback.reason==static_cast<std::uint8_t>(expected));
        WorldStateLock lock; assert(resident->object._hp==1 && subject->object._seedNum==10);
    };
    attack(AttackFeedback::SafeZone);
    { WorldStateLock lock; setTestLegacyX(subject->object,10); setTestLegacyY(subject->object,13); subject->updateSector(spawnX,spawnY); }
    cs_packet_move step{}; step.size=sizeof(step); step.type=C2S_P_MOVE; step.direction=1; step.move_time=7070;
    sendBytes(peer,reinterpret_cast<char*>(&step),sizeof(step));
    std::set<int> left; bool arrived=false,receivedNear=false;
    for(int n=0;n<128 && !receivedNear;++n) {
        auto packet=receive(peer); const auto type=static_cast<std::uint8_t>(packet[1]);
        if(type==S2C_P_MOVE) { auto move=decode<sc_packet_move>(packet);
            if(move.id==peer.accepted.id && move.move_time==7070) { assert(move.x==42 && move.y==10); arrived=true; } }
        if(type==S2C_P_LEAVE) left.insert(decode<sc_packet_leave>(packet).id);
        if(type==S2C_P_MONSTER_NEAR) { assert(packet[2]==0); receivedNear=true; }
    }
    assert(arrived && receivedNear && left==entered);
    { WorldStateLock lock;
      const auto boss=clients.get(town_npcs::baseId)->object;
      serviceTownResidents(1000000);
      assert(testLegacy(clients.get(town_npcs::baseId)->object).x==testLegacy(boss).x && testLegacy(clients.get(town_npcs::baseId)->object).y==testLegacy(boss).y); }
    { WorldStateLock lock;
      assert(!subject->_viewList.count(resident->object._id));
      const auto oldX=testLegacy(resident->object).x,oldY=testLegacy(resident->object).y;
      setTestLegacyX(resident->object,42); setTestLegacyY(resident->object,11); resident->updateSector(oldX,oldY);
      assert(subject->getNearBySessions().count(resident->object._id)); }
    attack(AttackFeedback::NoTarget); // Noncombat role is enforced even outside the safe zone.
    closePeer(peer); subject.reset(); resident.reset();
    const auto deadline=Clock::now()+std::chrono::seconds(3);
    while(peer.accepted.owner.use_count()!=1 && Clock::now()<deadline) std::this_thread::yield();
    assert(peer.accepted.owner.use_count()==1); stopWorkers(workers); clients.clear(); assert(peer.accepted.owner.expired());
}
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
#ifdef GMSEV_WORLD_TEST
void runChunks(const sockaddr_in& address,std::vector<std::thread>& workers,std::shared_ptr<GAMESESSION> target) {
    using world_regions::Id; using world_regions::Position;
    auto old=connectPeer(address,0);
    auto oldLogin=loginPacket(62001); oldLogin.version=3;
    sendBytes(old,reinterpret_cast<char*>(&oldLogin),sizeof(oldLogin));
    expect(old,S2C_P_LOGIN_FAIL); closePeer(old); assert(loadCount(62001)==0 && saveCount(62001)==0);
    auto peer=connectPeer(address,old.accepted.connection); login(peer,62000,true);
    auto subject=peer.accepted.owner.lock(); assert(subject);
    { WorldStateLock lock;
      target->object.position={Id::SunnyPeak,31,9}; target->updateSector(0,0);
      assert(!target->monster._isActive); }
    std::uint32_t token=0;
    auto step=[&](int direction,Position expected) {
        cs_packet_move packet{sizeof(packet),C2S_P_MOVE,static_cast<std::uint8_t>(direction),++token};
        sendBytes(peer,reinterpret_cast<char*>(&packet),sizeof(packet));
        sc_packet_move own{};
        for(int n=0;n<128;++n) {
            const auto bytes=expect(peer,S2C_P_MOVE); std::memcpy(&own,bytes.data(),sizeof(own));
            if(own.id==peer.accepted.id) break;
        }
        assert(own.id==peer.accepted.id && own.move_time==token && own.region==static_cast<std::uint8_t>(expected.region));
        assert(own.x==expected.x && own.y==expected.y);
        for(int chunk=0;chunk<4;++chunk) {
            const auto bytes=expect(peer,S2C_P_MAPINFO); sc_packet_mapInfo map{}; std::memcpy(&map,bytes.data(),sizeof(map));
            assert(map.region==own.region && map.left_x==world_layout::cameraX(expected));
            assert(map.top_y==world_layout::cameraY(expected)+chunk*4);
            for(int y=0;y<4;++y) for(int x=0;x<18;++x)
                assert(map.tiles[y][x]==mapTileAt(Position{expected.region,map.left_x+x,map.top_y+y}));
        }
        expect(peer,S2C_P_MONSTER_NEAR);
        WorldStateLock lock; assert(worldSectors.memberships(subject->object._id)==1);
    };
    for(int x=16;x<=48;++x) {
        step(3,{Id::SunnyPeak,x,9});
        if(x==24) { WorldStateLock lock; assert(target->monster._isActive); }
    }
    { WorldStateLock lock; assert(!target->monster._isActive); }
    for(int y=10;y<=32;++y) step(1,{Id::SunnyPeak,48,y});
    for(int x=49;x<=85;++x) step(3,{Id::SunnyPeak,x,32}); // Real bridge crossing, not a fixture teleport.
    { WorldStateLock lock; subject->object.position={Id::SunnyPeak,79,30}; subject->updateSector(0,0); }
    step(3,{Id::SunnyPeak,79,30}); // Water blocks actual movement.
    { WorldStateLock lock; subject->object.position={Id::SunnyPeak,254,254}; subject->updateSector(0,0); }
    step(3,{Id::SunnyPeak,254,254}); // Boundary cliff; camera/map must remain clamped.
    cs_packet_teleport request{sizeof(request),C2S_P_TELEPORT,world_travel::Action::ReturnHome,1};
    sendBytes(peer,reinterpret_cast<char*>(&request),sizeof(request));
    const auto result=decode<sc_packet_travel_result>(expect(peer,S2C_P_TRAVEL_RESULT));
    assert(result.result==world_travel::Result::Success);
    { WorldStateLock lock; assert(subject->object.position.region==Id::Clubhouse && worldSectors.memberships(subject->object._id)==1); }
    closePeer(peer); subject.reset(); stopWorkers(workers); clients.clear();
    { std::lock_guard<std::mutex> lock(mutex); assert(savedState[62000].position.region==Id::Clubhouse && saves[62000]==2); }
    std::puts("field worker: chunk/sector movement, terrain maps, bridge, water/edge collision, NPC activation/sleep, return/DB-stub PASS");
}
void runTravel(const sockaddr_in& address,std::vector<std::thread>& workers,std::shared_ptr<GAMESESSION> target) {
    using namespace world_travel;
    auto home=connectPeer(address,0); login(home,61000,true);
    auto field=connectPeer(address,home.accepted.connection); login(field,61001,false);
    expectEnter(home,field.accepted.id);
    auto request=[&](Peer& peer,std::uint32_t sequence,Action action,Result expected,bool moved) {
        cs_packet_teleport packet{sizeof(packet),C2S_P_TELEPORT,action,sequence};
        // Exercise framing, not direct processPacket calls.
        sendBytes(peer,reinterpret_cast<char*>(&packet),1);
        sendBytes(peer,reinterpret_cast<char*>(&packet)+1,sizeof(packet)-1);
        bool ownMove=false;
        for(int i=0;i<128;++i) {
            auto bytes=receive(peer);
            if(static_cast<std::uint8_t>(bytes[1])==S2C_P_MOVE) {
                sc_packet_move move{}; std::memcpy(&move,bytes.data(),sizeof(move));
                if(move.id==peer.accepted.id) {
                    const auto destination=world_travel::destination(action);
                    assert(move.region==static_cast<std::uint8_t>(destination.region));
                    assert(move.x==destination.x && move.y==destination.y); ownMove=true;
                }
            }
            if(static_cast<std::uint8_t>(bytes[1])==S2C_P_TRAVEL_RESULT) {
                auto result=decode<sc_packet_travel_result>(bytes);
                assert(result.requestId==sequence && result.result==expected && ownMove==moved); return;
            }
        }
        assert(false);
    };
    request(field,1,Action::EnterField,Result::Success,true);
    expectLeave(home,field.accepted.id);
    auto peer=connectPeer(address,field.accepted.connection); login(peer,61002,true);
    expectEnter(home,peer.accepted.id);
    auto subject=peer.accepted.owner.lock(); assert(subject);
    request(peer,0,Action::EnterField,Result::InvalidRequest,false);
    request(peer,1,static_cast<Action>(99),Result::InvalidRequest,false);
    request(peer,1,Action::EnterField,Result::Success,true);
    expectLeave(home,peer.accepted.id); expectEnter(field,peer.accepted.id);
    request(peer,1,Action::EnterField,Result::Success,false);
    request(peer,1,Action::ReturnHome,Result::InvalidRequest,false);
    request(peer,2,Action::EnterField,Result::AlreadyThere,false);
    request(peer,1,Action::EnterField,Result::StaleRequest,false);
    std::uint64_t generation;
    { WorldStateLock lock;
      target->monster.chaseTarget=peer.accepted.id; target->monster._isMonsterChase=true;
      target->monster._isActive=true; target->_viewList.insert(peer.accepted.id);
      generation=target->monster.generation;
      assert(!transitionRegion(*subject,{world_regions::Id::Invalid,0,0}));
      assert(worldSectors.memberships(subject->object._id)==1); }
    request(peer,3,Action::ReturnHome,Result::Success,true);
    expectLeave(field,peer.accepted.id); expectEnter(home,peer.accepted.id);
    { WorldStateLock lock;
      assert(target->monster.generation>generation && target->monster.chaseTarget==-1);
      assert(!target->monster._isMonsterChase && !target->_viewList.count(peer.accepted.id));
      assert(!subject->_viewList.count(field.accepted.id));
      assert(worldSectors.memberships(subject->object._id)==1); }
    request(peer,4,Action::ReturnHome,Result::AlreadyThere,false);
    closePeer(peer); closePeer(field); closePeer(home); subject.reset();
    { std::lock_guard<std::mutex> guard(mutex);
      assert(saves[61002]==2 && savedState[61002].position.region==world_regions::Id::Clubhouse);
      assert(savedState[61002].position.x==11 && savedState[61002].position.y==12); }
    stopWorkers(workers); clients.clear();
    std::puts("travel worker: TCP/authority/observers/replay/chase/sector/DB-stub save PASS");
}
void runRegions(const sockaddr_in& address,std::vector<std::thread>& workers,std::shared_ptr<GAMESESSION> target) {
    auto peer=connectPeer(address,0); login(peer,60000,true);
    auto watcher=connectPeer(address,peer.accepted.connection); login(watcher,60001,true);
    auto subject=peer.accepted.owner.lock(); assert(subject);
    { WorldStateLock lock; auto L=target->monster.script.get();
      assert(subject->getNearBySessions().count(target->object._id));
      lua_getglobal(L,"API_SendMessage"); lua_pushinteger(L,MAX_USER); lua_pushinteger(L,peer.accepted.id); lua_pushliteral(L,"LOCAL");
      assert(lua_pcall(L,3,0,0)==LUA_OK && lua_gettop(L)==0); }
    bool localMessage=false;
    for(int n=0;n<128 && !localMessage;++n) {
        auto packet=receive(peer);
        if(static_cast<std::uint8_t>(packet[1])==S2C_P_CHAT) localMessage=std::strcmp(decode<sc_packet_chat>(packet).message,"LOCAL")==0;
    }
    assert(localMessage);
    std::uint64_t oldGeneration;
    { WorldStateLock lock;
      assert(isCanSee(subject->object._id,target->object._id));
      assert(isMonSee(target->object._id,subject->object._id));
      setTestLegacyX(subject->object,31); setTestLegacyY(subject->object,7); // Same sector/range as field NPC, but invalid region.
      assert(!isCanSee(subject->object._id,target->object._id) && !isMonSee(target->object._id,subject->object._id));
      assert(!subject->getNearBySessions().count(target->object._id));
      setTestLegacyX(subject->object,42); setTestLegacyY(subject->object,11); subject->updateSector(39,7);
      target->monster.chaseTarget=subject->object._id; target->monster._isActive=true; target->monster._isMonsterChase=true;
      target->_viewList.insert(subject->object._id); target->monster._nextMonX=7; target->monster._nextMonY=6;
      oldGeneration=target->monster.generation.load(); }
    cs_packet_move step{}; step.size=sizeof(step); step.type=C2S_P_MOVE; step.direction=1; step.move_time=6060;
    sendBytes(peer,reinterpret_cast<char*>(&step),sizeof(step));
    bool arrived=false;
    for(int n=0;n<128;++n) { auto packet=receive(peer);
        if(static_cast<std::uint8_t>(packet[1])!=S2C_P_MOVE) continue;
        auto move=decode<sc_packet_move>(packet);
        if(move.id==peer.accepted.id && move.move_time==6060) { assert(move.x==10 && move.y==12); arrived=true; break; }
        assert(n<127);
    }
    assert(arrived);
    expectLeave(watcher,peer.accepted.id);
    { WorldStateLock lock;
      assert(subject->object.location().region==world_regions::Id::Clubhouse);
      assert(!subject->_viewList.count(target->object._id) && !watcher.accepted.owner.lock()->_viewList.count(subject->object._id));
      assert(target->monster.generation>oldGeneration && !target->monster._isMonsterChase && target->monster.chaseTarget==-1);
      assert(!target->_viewList.count(subject->object._id));
      assert(!subject->getNearBySessions().count(watcher.accepted.id)); }
    auto stale=new OVER_EXP; stale->_compType=OP_MONSTER_CHASE; stale->generation=oldGeneration; stale->_playerId=peer.accepted.id;
    assert(PostQueuedCompletionStatus(g_hiocp,1,MAX_USER,&stale->_over));
    waitSnapshot(target->connectionId,"stale-ai",[](const Snapshot& s) { return s.x==38 && s.y==6; });
    std::size_t beforeNotify;
    { std::lock_guard<std::mutex> lock(mutex); beforeNotify=sequence; }
    { WorldStateLock lock; auto L=target->monster.script.get();
      assert(luaL_dostring(L,"calls=0; function event_player_move(p,c) calls=calls+1 end")==LUA_OK);
      target->monster._isActive=true;
      auto notification=new OVER_EXP; notification->_compType=OP_PLAYER_MOVE;
      notification->generation=target->monster.generation; notification->_playerId=peer.accepted.id;
      notification->playerConnectionId=peer.accepted.connection;
      assert(PostQueuedCompletionStatus(g_hiocp,1,MAX_USER,&notification->_over)); }
    waitSnapshot(target->connectionId,"notify",[&](const Snapshot& s) { return s.serial>beforeNotify; });
    { WorldStateLock lock; auto L=target->monster.script.get();
      lua_getglobal(L,"calls"); assert(lua_tointeger(L,-1)==0); lua_pop(L,1); }
    // Inject a cross-region Lua message then a marker on the same transport. Only marker may arrive.
    { WorldStateLock lock; auto L=target->monster.script.get();
      lua_getglobal(L,"API_SendMessage"); lua_pushinteger(L,MAX_USER); lua_pushinteger(L,peer.accepted.id); lua_pushliteral(L,"CROSS");
      assert(lua_pcall(L,3,0,0)==LUA_OK && lua_gettop(L)==0);
      subject->sendChatPacket(subject->object._id,"MARK"); }
    bool marker=false;
    for(int n=0;n<128 && !marker;++n) {
        auto packet=receive(peer);
        if(static_cast<std::uint8_t>(packet[1])==S2C_P_CHAT) {
            const auto chat=decode<sc_packet_chat>(packet);
            assert(std::strcmp(chat.message,"CROSS")!=0); marker=std::strcmp(chat.message,"MARK")==0;
        }
    }
    assert(marker);
    cs_packet_attack attack{}; attack.size=sizeof(attack); attack.type=C2S_P_ATTACK;
    sendBytes(peer,reinterpret_cast<char*>(&attack),sizeof(attack));
    const auto feedback=decode<sc_packet_attack_feedback>(expect(peer,S2C_P_ATTACK_FEEDBACK));
    assert(feedback.reason==static_cast<std::uint8_t>(AttackFeedback::SafeZone));
    { WorldStateLock lock; assert(target->object._hp==5 && subject->object._seedNum==10); }
    closePeer(peer); closePeer(watcher); subject.reset();
    const auto deadline=Clock::now()+std::chrono::seconds(3);
    while((peer.accepted.owner.use_count()!=1 || watcher.accepted.owner.use_count()!=1) && Clock::now()<deadline) std::this_thread::yield();
    assert(peer.accepted.owner.use_count()==1 && watcher.accepted.owner.use_count()==1);
    stopWorkers(workers); clients.clear(); assert(peer.accepted.owner.expired() && watcher.accepted.owner.expired());
}
void runLuaAI(const sockaddr_in& address,std::vector<std::thread>& workers,std::shared_ptr<GAMESESSION> target) {
    auto peer=connectPeer(address,0); login(peer,50000,true);
    const auto connection=target->connectionId;
    auto tick=[&](EVENT_TYPE type) {
        std::size_t before;
        { std::lock_guard<std::mutex> lock(mutex); before=sequence; }
        const int idx=get_timer_queue_index(MAX_USER);
        { WorldStateLock lock; std::lock_guard<std::mutex> timers(timerLocks[idx]);
          while(!timerQueues[idx].empty()) timerQueues[idx].pop();
          timerQueues[idx].push(TIMER_EVENT{MAX_USER,Clock::now(),type,peer.accepted.id,target->monster.generation.load()}); }
        assert(dispatchDueTimer(idx,Clock::now()));
        return waitSnapshot(connection,"ai",[&](const Snapshot& s) { return s.serial>before; });
    };
    int expected=-1; std::string error;
    { WorldStateLock lock;
      assert(npc_script::direction(target->monster.script.get(),expected,error));
      assert(npc_script::invoke(target->monster.script.get(),"set_uid",{MAX_USER},0,error));
      setTestLegacyX(target->object,38); setTestLegacyY(target->object,7);
      target->monster._isActive=true; target->monster._isMonsterChase=false; }
    auto moved=tick(EV_RANDOM_MOVE);
    const int dx[]={1,-1,0,0},dy[]={0,0,1,-1};
    assert(moved.x==38+dx[expected] && moved.y==7+dy[expected] && !moved.scriptDisabled);
    { WorldStateLock lock;
      setTestLegacyX(target->object,spawnX); setTestLegacyY(target->object,spawnY);
      target->monster._isMonsterChase=true; target->monster.chaseTarget=peer.accepted.id;
      target->monster._nextMonX=static_cast<short>(world_regions::decode(spawnX,spawnY).x); target->monster._nextMonY=spawnY; }
    auto resumed=tick(EV_CHASE); assert(resumed.active && !resumed.chasing);
    { WorldStateLock lock;
      auto broken=npc_script::loadText("calls=0; function set_uid(id) end; function event_player_move(p) end; function choose_direction() calls=calls+1; while true do end end",MAX_USER,{API_get_x,API_get_y,API_SendMessage});
      assert(broken); target->monster.script=std::move(broken.state); }
    auto failed=tick(EV_RANDOM_MOVE); assert(failed.scriptDisabled);
    tick(EV_RANDOM_MOVE);
    { WorldStateLock lock; auto L=target->monster.script.get();
      lua_getglobal(L,"calls"); assert(lua_tointeger(L,-1)==1); lua_pop(L,1);
      assert(lua_gettop(L)==0 && target->monster.scriptError.find("budget")!=std::string::npos); }
    movePair(peer,555); // Lua failure has not stranded the world lock/worker.
    closePeer(peer);
    auto sleeping=tick(EV_RANDOM_MOVE); assert(!sleeping.active && !sleeping.chasing);
    { WorldStateLock lock;
      const int idx=get_timer_queue_index(MAX_USER);
      target->monster._isActive=true; // Isolate generation rejection from the inactive check.
      { std::lock_guard<std::mutex> timers(timerLocks[idx]);
        timerQueues[idx].push(TIMER_EVENT{MAX_USER,Clock::now(),EV_RANDOM_MOVE,0,sleeping.generation-1}); }
      assert(dispatchDueTimer(idx,Clock::now()));
      assert(timerQueues[idx].empty()); target->monster._isActive=false; }
    const auto deadline=Clock::now()+std::chrono::seconds(3);
    while(peer.accepted.owner.use_count()!=1 && Clock::now()<deadline) std::this_thread::yield();
    assert(peer.accepted.owner.use_count()==1);
    stopWorkers(workers); clients.clear(); assert(peer.accepted.owner.expired());
}
void runWorldState(const sockaddr_in& address,std::vector<std::thread>& workers) {
    constexpr int count=4;
    std::vector<Peer> peers;
    std::uint64_t last=0;
    for(int i=0;i<count;++i) {
        peers.push_back(connectPeer(address,last)); last=peers.back().accepted.connection;
        login(peers.back(),40000+i,true);
    }
    // All four move simultaneously; each response must retain its own coordinates and token.
    parallelPeers(count,[&](int i) {
        for(int step=0;step<20;++step) movePair(peers[i],static_cast<std::uint32_t>(10000+i*100+step*2));
    });
    auto attack=[&](int i) {
        cs_packet_attack request{}; request.size=sizeof(request); request.type=C2S_P_ATTACK;
        request.damage=9999; request.seedNum=9999; // Client claims must not override authoritative state.
        sendBytes(peers[i],reinterpret_cast<char*>(&request),sizeof(request));
        for(int n=0;n<128;++n) {
            auto response=receive(peers[i]); const auto type=static_cast<std::uint8_t>(response[1]);
            if(type == S2C_P_MONSTER_NEAR || type == S2C_P_ATTACK_FEEDBACK) return;
        }
        assert(false);
    };
    // Two attackers target the same five-HP NPC while the other two keep moving.
    parallelPeers(count,[&](int i) {
        if(i<2) for(int n=0;n<8;++n) attack(i);
        else for(int n=0;n<10;++n) movePair(peers[i],static_cast<std::uint32_t>(20000+i*100+n*2));
    });
    for(int i=0;i<count;++i) movePair(peers[i],static_cast<std::uint32_t>(30000+i*2));
    const auto reference=peers[0].combat;
    assert(reference.size() == 5);
    std::map<int,int> spent;
    for(int i=0;i<5;++i) {
        assert(reference[i].targetId == MAX_USER && reference[i].targetHp == 4-i);
        ++spent[reference[i].attackerId];
        assert(reference[i].seedNum == 10-spent[reference[i].attackerId]);
    }
    // A request after death must not consume another seed or emit another death.
    attack(0);
    for(int i=0;i<count;++i) {
        auto& peer=peers[i]; assert(peer.combat.size() == reference.size() && peer.deaths == 1);
        for(std::size_t n=0;n<reference.size();++n) assert(std::memcmp(&peer.combat[n],&reference[n],sizeof(reference[n])) == 0);
        auto probe=new OVER_EXP; probe->_compType=OP_LOGIN_FAIL; probe->owner=peer.accepted.owner.lock();
        assert(PostQueuedCompletionStatus(g_hiocp,1,peer.accepted.id,&probe->_over));
        const auto expectedSeeds=10-spent[peer.accepted.id];
        waitSnapshot(peer.accepted.connection,"probe",[&](const Snapshot& s) {
            return s.x == spawnX && s.y == spawnY && s.hp == 10 && s.seeds == expectedSeeds &&
                s.targetHp == 0 && s.targetDead && s.totalSeeds == 35 && s.ownView == count-1 &&
                !s.visible.count(MAX_USER) && s.sectorRefs == 1;
        });
    }
    parallelPeers(count,[&](int i) { closePeer(peers[i]); });
    for(int i=0;i<count;++i) {
        std::lock_guard<std::mutex> lock(mutex);
        const auto& saved=savedState.at(40000+i);
        assert(testLegacy(saved).x == spawnX && testLegacy(saved).y == spawnY && saved._hp == 10 && saved._seedNum == 10-spent[peers[i].accepted.id]);
    }
    const auto deadline=Clock::now()+std::chrono::seconds(3);
    auto drained=[&] { for(const auto& peer : peers) if(peer.accepted.owner.use_count() != 1) return false; return true; };
    while(!drained() && Clock::now()<deadline) std::this_thread::yield(); assert(drained());
    stopWorkers(workers); clients.clear();
    for(const auto& peer : peers) assert(peer.accepted.owner.expired());
}
#endif
#endif
}

int main() {
    using namespace fixture;
    WSADATA data{}; assert(WSAStartup(MAKEWORD(2,2),&data) == 0);
    initializeMap(); // No NPC Lua or timer threads.
#ifdef GMSEV_TOWN_TEST
    initializeTownResidents();
#endif
#ifdef GMSEV_WORLD_TEST
    auto target=std::make_shared<GAMESESSION>();
    target->object._id=MAX_USER; setTestLegacyX(target->object,38); setTestLegacyY(target->object,6);
    target->object._hp=target->object._maxHp=5; target->_state=ST_INGAME; target->monster._monType=1;
#ifdef GMSEV_LUA_AI_TEST
    auto loaded=npc_script::loadFile("TP_2023192003/TP_2023192003/npc.lua",MAX_USER,{API_get_x,API_get_y,API_SendMessage});
#else
    auto loaded=npc_script::loadText("function set_uid(id) end; function choose_direction() return 0 end; function event_player_move(player) local x=API_get_x(player); local y=API_get_y(player) end",MAX_USER,{API_get_x,API_get_y,API_SendMessage});
#endif
    assert(loaded); target->monster.script=std::move(loaded.state);
    auto L=target->monster.script.get();
    for(lua_CFunction function : {API_get_x,API_get_y,API_SendMessage}) {
        lua_pushcfunction(L,function); lua_pushliteral(L,"bad-id");
        assert(lua_pcall(L,1,1,0) != LUA_OK); lua_pop(L,1);
        assert(!WorldStateLock::heldByCurrentThread()); // Lua argument errors cannot strand a C++ guard.
    }
    clients.set(MAX_USER,target);
    { auto L=target->monster.script.get();
      lua_getglobal(L,"API_get_x"); lua_pushinteger(L,MAX_USER);
      assert(lua_pcall(L,1,1,0)==LUA_OK && lua_tointeger(L,-1)==6); lua_pop(L,1); // Local, not legacy 38.
      lua_getglobal(L,"API_get_y"); lua_pushinteger(L,MAX_USER);
      assert(lua_pcall(L,1,1,0)==LUA_OK && lua_tointeger(L,-1)==6); lua_pop(L,1);
      for(const char* name : {"API_get_x","API_get_y"}) {
        lua_getglobal(L,name); lua_pushinteger(L,MAX_USER+100);
        assert(lua_pcall(L,1,1,0)==LUA_OK && lua_isnil(L,-1)); lua_pop(L,1);
        lua_getglobal(L,name); lua_pushinteger(L,-1);
        assert(lua_pcall(L,1,1,0)!=LUA_OK); lua_pop(L,1);
      }
      lua_getglobal(L,"API_SendMessage"); lua_pushinteger(L,MAX_USER+1); lua_pushinteger(L,0); lua_pushliteral(L,"forged");
      assert(lua_pcall(L,3,0,0)!=LUA_OK); lua_pop(L,1);
      assert(lua_gettop(L)==0 && !WorldStateLock::heldByCurrentThread()); }
    assert(worldSectors.put(MAX_USER,target->object.location()));
#endif
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
#ifdef GMSEV_SECTOR_TEST
    runSectorIndex(address,workers);
#elif defined(GMSEV_TOWN_TEST)
    runTown(address,workers);
#elif defined(GMSEV_WORLD_TEST)
#ifdef GMSEV_CHUNK_TEST
    runChunks(address,workers,target);
#elif defined(GMSEV_TRAVEL_TEST)
    runTravel(address,workers,target);
#elif defined(GMSEV_REGION_TEST)
    runRegions(address,workers,target);
#elif defined(GMSEV_LUA_AI_TEST)
    runLuaAI(address,workers,target);
#else
    runWorldState(address,workers);
#endif
    target->monster.script.reset();
#else
    runMultiworker(address,workers);
#endif
#else
    workers.emplace_back(workerThread,g_hiocp);
    {
        // One actual worker must pass SEND while another thread holds the original owner's lock.
        auto blocked=std::make_shared<GAMESESSION>(); blocked->object._id=9; blocked->_state=ST_ALLOC;
        auto probe=std::make_shared<GAMESESSION>(); probe->object._id=8; probe->_state=ST_ALLOC;
        std::unique_lock<std::recursive_mutex> held(blocked->completionMutex);
        auto completed=new OVER_EXP; completed->_compType=OP_SEND; completed->_wasBuf.len=6; completed->owner=blocked;
        std::weak_ptr<GAMESESSION> original=blocked;
        assert(PostQueuedCompletionStatus(g_hiocp,6,9,&completed->_over));
        auto marker=new OVER_EXP; marker->_compType=OP_LOGIN_FAIL; marker->owner=probe;
        assert(PostQueuedCompletionStatus(g_hiocp,1,8,&marker->_over));
        const auto firstProbe=waitSnapshot(probe->connectionId,"probe",[](const Snapshot&) {return true;});
        assert(!blocked->disconnectStarted && !probe->disconnectStarted);
        blocked->disconnectStarted=true;
        auto canceled=new OVER_EXP; canceled->_compType=OP_SEND; canceled->_wasBuf.len=6; canceled->owner=blocked;
        assert(PostQueuedCompletionStatus(g_hiocp,0,9,&canceled->_over));
        auto secondMarker=new OVER_EXP; secondMarker->_compType=OP_LOGIN_FAIL; secondMarker->owner=probe;
        assert(PostQueuedCompletionStatus(g_hiocp,1,8,&secondMarker->_over));
        waitSnapshot(probe->connectionId,"probe",[&](const Snapshot& s){return s.serial>firstProbe.serial;});
        assert(blocked->_state==ST_ALLOC); // SEND did not take over an already-owned disconnect.
        held.unlock(); blocked.reset(); assert(original.expired());
        auto partial=std::make_shared<GAMESESSION>(); partial->object._id=7; partial->_state=ST_ALLOC;
        auto truncated=new OVER_EXP; truncated->_compType=OP_SEND; truncated->_wasBuf.len=6; truncated->owner=partial;
        assert(PostQueuedCompletionStatus(g_hiocp,3,7,&truncated->_over));
        waitSnapshot(partial->connectionId,"disconnect",[](const Snapshot& s){return s.state==ST_FREE;});
        assert(partial->disconnectStarted && partial->transportFailed);
    }
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
        auto oldVersion=loginPacket(403); oldVersion.version=1;
        sendBytes(subject,reinterpret_cast<char*>(&oldVersion),sizeof(oldVersion));
        assert(static_cast<std::uint8_t>(receive(subject)[1])==S2C_P_LOGIN_FAIL);
        assert(loadCount(403)==0 && saveCount(403)==0);
        for(int account : {404,405}) {
            auto fail=loginPacket(account); sendBytes(subject,reinterpret_cast<char*>(&fail),sizeof(fail));
            assert(static_cast<std::uint8_t>(receive(subject)[1]) == S2C_P_LOGIN_FAIL && saveCount(account) == 0);
        }
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
#ifdef GMSEV_SECTOR_TEST
    std::cout << "Regional sectors: local boundary MOVE, cross-region portal, identical local coordinates isolated, disconnect prunes buckets passed (4 workers, DB stub)\n";
#elif defined(GMSEV_TOWN_TEST)
    std::cout << "Town residents: autonomous walk, MOVE/visibility/re-entry, no monster AI/attack, field role defense passed (4 workers, DB stub)\n";
#elif defined(GMSEV_WORLD_TEST)
#ifdef GMSEV_CHUNK_TEST
    std::cout << "Field chunks: real worker wide-world checks passed (4 workers, DB stub)\n";
#elif defined(GMSEV_TRAVEL_TEST)
    std::cout << "Travel: explicit requests/results and region transition passed (4 workers, DB stub)\n";
#elif defined(GMSEV_REGION_TEST)
    std::cout << "Regions: real portal, observer leave, chase cancellation, stale AI, cross-region Lua chat and safe-zone attack passed (4 workers, DB stub)\n";
#elif defined(GMSEV_LUA_AI_TEST)
    std::cout << "Lua AI: actual policy movement, same-tile chase recovery, budget fallback, sleep and stale timer passed (4 workers, DB stub)\n";
#else
    std::cout << "World state: 4 workers, concurrent movement/combat, five hits/one death, DB outside world gate passed\n";
#endif
#else
    std::cout << "Production worker: 4 workers, 16 stationary peers x 5 batches including login/logout overlap passed (DB stub)\n";
#endif
#else
    std::cout << "Production worker: fragmented login, coalesced moves, cleanup and 30 reconnect cycles passed (DB stub, one worker)\n";
#endif
}
