#include "../TP_2023192003/TP_2023192003/world_events.hpp"
#include "../TP_2023192003/TP_2023192003/pathfinder.hpp"
#include "../TP_2023192003/TP_2023192003/world_map.hpp"
#include <cassert>
#include <map>
#include <vector>
static std::map<int,std::vector<int>> packets;
void GAMESESSION::registerSend(void* input) { packets[object._id].push_back(static_cast<unsigned char*>(input)[1]); }
static std::shared_ptr<GAMESESSION> add(int id,int x,int y) {
    auto s=std::make_shared<GAMESESSION>(); s->object._id=id; s->object._x=x; s->object._y=y; s->_state=ST_INGAME;
    clients.set(id,s); return s;
}
int main() {
    auto field=add(0,42,10),club=add(1,10,10),npc=add(MAX_USER,42,10),invalid=add(2,31,10);
    assert(field->object.location().x==club->object.location().x);
    field->sendAddPlayerPacket(MAX_USER); club->sendAddPlayerPacket(MAX_USER); invalid->sendAddPlayerPacket(MAX_USER);
    assert(packets[0].size()==1 && packets[1].empty() && packets[2].empty());
    publishCombat(*npc,0,4,false,{0,1,2});
    assert(packets[0].size()==2 && packets[1].empty() && packets[2].empty());
    npc->monster.chaseTarget=0; npc->monster._isMonsterChase=true; npc->monster._isActive=true;
    npc->_viewList.insert(0); const auto before=npc->monster.generation.load();
    cancelRegionChases(0);
    assert(npc->monster.generation==before+1 && npc->monster.chaseTarget==-1 && !npc->monster._isActive && !npc->_viewList.count(0));
    assert(!respawn_policy::accepts(before,npc->monster.generation,false));
    initializeFieldMap();
    assert(findPathAStar(10,12,42,10).empty());
    assert(!findPathAStar(38,7,39,7).empty());
    clients.clear();
}
