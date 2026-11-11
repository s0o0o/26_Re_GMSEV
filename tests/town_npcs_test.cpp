#include "../shared/town_npcs.hpp"
#include <cassert>
int main() {
    for(const auto& npc : town_npcs::residents) {
        assert(town_npcs::isResident(npc.type) && !town_npcs::hostile(npc.type));
        assert(npc.id>=MAX_USER+200000 && world_layout::walkable(npc.x,npc.y));
        assert(world_regions::decode(npc.x,npc.y).region==world_regions::Id::Clubhouse);
        assert(!(npc.x==10 && npc.y==14) && !(npc.x==clubhouse::spawnX && npc.y==clubhouse::spawnY));
        for(const auto& other : town_npcs::residents) if(&npc!=&other)
            assert(npc.id!=other.id && npc.type!=other.type && (npc.x!=other.x || npc.y!=other.y));
    }
    assert(town_npcs::hostile(1) && town_npcs::hostile(2) && !town_npcs::hostile(0) && !town_npcs::hostile(999));
    assert(!town_npcs::isResident(2) && !town_npcs::isResident(103) && !town_npcs::byType(999));
}
