#pragma once
#include "game_object.hpp"
#include "monster_state.hpp"
#include "../../shared/field_content.hpp"

// Caller holds monster.combatMutex for both AI dispatch and lifecycle changes.
inline void markMonsterDead(MonsterState& monster,respawn_policy::Tick now) {
    if(monster._isDie) return;
    ++monster.generation;
    monster._isDie=true; monster._isActive=false; monster._isMonsterChase=false;
    monster.respawnAt=now+respawn_policy::monsterDelay;
}
inline bool reviveMonster(GameObjectState& object,MonsterState& monster,respawn_policy::Tick now) {
    const int index=object._id-MAX_USER;
    if(index < 0 || index >= static_cast<int>(field_content::monsters.size()) ||
        !monster._isDie || !respawn_policy::due(monster.respawnAt,now)) return false;
    auto home=field_content::monsters[index];
    object._x=static_cast<short>(home.x); object._y=static_cast<short>(home.y);
    object._hp=object._maxHp=5;
    monster._nextMonX=object._x; monster._nextMonY=object._y;
    monster._isActive=false; monster._isMonsterChase=false;
    monster.respawnAt=0; ++monster.generation;
    monster._isDie=false;
    return true;
}
