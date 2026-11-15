#pragma once
#include "../TP_2023192003/TP_2023192003/game_object.hpp"
// Preserve historical test scenarios without adding legacy storage to production objects.
inline world_regions::LegacyPosition testLegacy(const GameObjectState& object) {
    world_regions::LegacyPosition result;
    if(world_regions::encode(object.location(),result)) return result;
    return {object.position.x,object.position.y}; // Explicitly malformed fixture coordinates.
}
inline void setTestLegacy(GameObjectState& object,int x,int y) {
    auto position=world_regions::decode(x,y);
    if(!world_regions::valid(position)) position={world_regions::Id::Invalid,x,y};
    object.position=position;
}
inline void setTestLegacyX(GameObjectState& object,int x) { setTestLegacy(object,x,testLegacy(object).y); }
inline void setTestLegacyY(GameObjectState& object,int y) { setTestLegacy(object,testLegacy(object).x,y); }
