#include "../TP_2023192003/TP_2023192003/monster_lifecycle.hpp"
#include "../TP_2023192003/TP_2023192003/world_map.hpp"
#include <cassert>
int main() {
    GameObjectState object;
    MonsterState monster;
    object._id=MAX_USER; object._hp=0; object._x=40; object._y=8;
    const auto oldTicket=monster.generation.load();
    markMonsterDead(monster,1000);
    const auto deadTicket=monster.generation.load();
    assert(!respawn_policy::accepts(oldTicket,deadTicket,true));
    assert(!reviveMonster(object,monster,30999));
    assert(reviveMonster(object,monster,31000));
    assert(object._hp == 5 && object._x == field_content::monsters[0].x && object._y == field_content::monsters[0].y);
    assert(!monster._isDie && !monster._isActive && !monster._isMonsterChase);
    assert(!respawn_policy::accepts(oldTicket,monster.generation.load(),false));
    assert(!respawn_policy::accepts(deadTicket,monster.generation.load(),false));
    assert(respawn_policy::accepts(monster.generation.load(),monster.generation.load(),false));
    assert(!reviveMonster(object,monster,999999));
    markMonsterDead(monster,40000);
    const auto deadline=monster.respawnAt;
    markMonsterDead(monster,41000);
    assert(monster.respawnAt == deadline);
    assert(reviveMonster(object,monster,70000));
    initializeFieldMap();
    auto seed=field_content::seeds[0];
    assert(collectSeed(seed.x,seed.y,100));
    assert(restoreSeeds(20099).empty());
    auto restored=restoreSeeds(20100);
    assert(restored.size() == 1 && mapTileAt(seed.x,seed.y) == 4);
    assert(restoreSeeds(99999).empty());
    assert(collectSeed(seed.x,seed.y,100000));
    assert(restoreSeeds(119999).empty());
    assert(restoreSeeds(120000).size() == 1);
}
