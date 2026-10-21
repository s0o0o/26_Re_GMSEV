#include "../TP_2023192003/TP_2023192003/world_events.hpp"
#include "../TP_2023192003/TP_2023192003/monster_lifecycle.hpp"
#include "../TP_2023192003/TP_2023192003/world_map.hpp"
#include "../shared/combat_rules.hpp"
#include <cassert>
#include <cstring>
#include <map>
#include <vector>

// In-process integration of production packet writers/fan-out/lifecycle/cleanup.
// No sockets, timer threads, DB or graphical clients are started.
static std::map<int,std::vector<std::vector<char>>> packets;
void GAMESESSION::registerSend(void* packet) {
    auto p=static_cast<char*>(packet);
    packets[object._id].emplace_back(p,p+static_cast<unsigned char>(p[0]));
}
template<class Packet> Packet read(int id,std::size_t index) {
    Packet p{};
    assert(packets[id][index].size() == sizeof(p));
    std::memcpy(&p,packets[id][index].data(),sizeof(p)); return p;
}
std::shared_ptr<GAMESESSION> add(int id) {
    auto s=std::make_shared<GAMESESSION>(); s->object._id=id; s->_state=ST_INGAME;
    clients[id]=s; return s;
}
int main() {
    auto a=add(1),b=add(2),closed=add(3),login=add(4),npc=add(MAX_USER),other=add(MAX_USER+1);
    closed->disconnectStarted=true; login->_state=ST_ALLOC;
    const std::unordered_set<int> audience{1,2,3,4,MAX_USER,999};
    npc->object._hp=npc->object._maxHp=5; a->object._seedNum=3;
    a->_viewList.insert(MAX_USER); b->_viewList.insert(MAX_USER);
    auto hit=combat_rules::apply(npc->object._hp,a->object._seedNum,5);
    assert(hit.applied && hit.died);
    const auto oldAi=npc->monster.generation.load();
    markMonsterDead(npc->monster,1000);
    publishCombat(*npc,1,a->object._seedNum,hit.died,audience);
    for(int id : {1,2}) {
        assert(packets[id].size() == 2);
        auto attack=read<sc_packet_attack_success>(id,0);
        assert(attack.attackerId == 1 && attack.targetId == MAX_USER && attack.targetHp == 0 && attack.seedNum == 2);
        assert(read<sc_packet_monster_die>(id,1).id == MAX_USER);
        assert(!clients[id]->_viewList.count(MAX_USER));
    }
    assert(packets[3].empty() && packets[4].empty() && packets[MAX_USER].empty() && packets[999].empty());
    assert(!reviveMonster(npc->object,npc->monster,30999));
    assert(reviveMonster(npc->object,npc->monster,31000));
    assert(!respawn_policy::accepts(oldAi,npc->monster.generation,false));
    forWorldPlayers(audience,[&](GAMESESSION& viewer) { viewer.sendAddPlayerPacket(MAX_USER); });
    for(int id : {1,2}) {
        auto enter=read<sc_packet_enter>(id,2);
        assert(enter.hp == 5 && enter.max_hp == 5 && enter.x == field_content::monsters[0].x);
        assert(clients[id]->_viewList.count(MAX_USER));
    }
    initializeFieldMap(); const auto seed=field_content::seeds[0];
    assert(collectSeed(seed.x,seed.y,1000));
    assert(restoreSeeds(21000).size() == 1);
    forWorldPlayers(audience,[&](GAMESESSION& viewer) { viewer.sendMapInfo(viewer.object._id,seed.x,seed.y); });
    // Both observers get the exact same four restored map chunks.
    for(std::size_t i=3;i<7;++i) {
        assert(packets[1][i] == packets[2][i]);
        assert(read<sc_packet_mapInfo>(1,i).type == S2C_P_MAPINFO);
    }
    const auto connection=a->connectionId;
    assert(currentWorldPlayer(1,connection));
    b->_viewList.insert(1); // Asymmetric: a does not list b.
    closed->_viewList.insert(1); npc->_viewList.insert(1);
    npc->monster.chaseTarget=1; npc->monster._isMonsterChase=true; npc->monster._isActive=true;
    other->monster.chaseTarget=2; other->monster._isActive=true;
    auto chaseTicket=npc->monster.generation.load();
    std::unordered_set<int> sector{1,2,MAX_USER};
    a->disconnectStarted=true; a->transportFailed=true;
    const auto before=packets[2].size();
    detachWorldPresence(*a,[&](int id) { sector.erase(id); });
    assert(!sector.count(1) && sector.count(2));
    assert(a->_viewList.empty() && !b->_viewList.count(1) && !closed->_viewList.count(1) && !npc->_viewList.count(1));
    assert(read<sc_packet_leave>(2,before).id == 1);
    assert(npc->monster.chaseTarget == -1 && !npc->monster._isActive && !npc->monster._isMonsterChase);
    assert(!respawn_policy::accepts(chaseTicket,npc->monster.generation,false));
    assert(other->monster.chaseTarget == 2 && other->monster._isActive);
    detachWorldPresence(*a,[&](int id) { sector.erase(id); });
    assert(packets[2].size() == before+1); // No duplicate leave.
    auto reconnected=add(1);
    assert(reconnected->connectionId != connection);
    assert(!currentWorldPlayer(1,connection));
    assert(currentWorldPlayer(1,reconnected->connectionId));
}
