#pragma once

#include "../../shared/protocol.hpp"
#include "../../shared/world_regions.hpp"

// Shared data for players and monsters. No socket or rendering dependencies.
struct GameObjectState {
    int _id = -1;
    short _x = 0, _y = 0;
    world_regions::Position location() const { return world_regions::decode(_x,_y); }
    short _hp = 0, _maxHp = 0, _level = 0, _exp = 0, _seedNum = 0;
    short _damage = 0;
    char _name[NAME_SIZE] = {};
    std::uint8_t _furColor = 0;
    std::uint8_t _eyeStyle = 0, _scarfStyle = 0;
    std::uint8_t _characterKind = 0;
};
