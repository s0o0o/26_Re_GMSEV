#pragma once

#include "../../shared/protocol.hpp"

// Shared data for players and monsters. No socket or rendering dependencies.
struct GameObjectState {
    int _id = -1;
    short _x = 0, _y = 0;
    short _hp = 0, _maxHp = 0, _level = 0, _exp = 0, _seedNum = 0;
    short _damage = 0;
    char _name[NAME_SIZE] = {};
};
