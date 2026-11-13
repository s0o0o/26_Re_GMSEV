#pragma once
#include "../../shared/world_layout.hpp"
namespace database_position {
struct Restored {
    world_regions::Position position;
    world_regions::LegacyPosition legacy;
    bool recovered;
};
inline Restored restoreLegacy(int x,int y) {
    if(world_layout::walkable(x,y)) return {world_regions::decode(x,y),{x,y},false};
    return {{world_regions::Id::Clubhouse,clubhouse::spawnX,clubhouse::spawnY},{clubhouse::spawnX,clubhouse::spawnY},true};
}
inline bool encodeLegacy(world_regions::Position position,world_regions::LegacyPosition& output) {
    world_regions::LegacyPosition candidate;
    if(!world_regions::encode(position,candidate) || !world_layout::walkable(candidate.x,candidate.y)) return false;
    output=candidate; return true;
}
}
