#pragma once

#include "../../shared/protocol.hpp"
#include "../../shared/world_regions.hpp"

// Shared data for players and monsters. No socket or rendering dependencies.
struct GameObjectState {
    int _id = -1;
    world_regions::Position position{world_regions::Id::Clubhouse,0,0};
    world_regions::Position location() const { return position; }
    short _hp = 0, _maxHp = 0, _level = 0, _exp = 0, _seedNum = 0;
    short _damage = 0;
    char _name[NAME_SIZE] = {};
    std::uint8_t _furColor = 0;
    std::uint8_t _eyeStyle = 0, _scarfStyle = 0;
    std::uint8_t _characterKind = 0;
    avatar_appearance::Value appearance() const { return {_characterKind,_furColor,_eyeStyle,_scarfStyle}; }
    void setAppearance(avatar_appearance::Value value) {
        _characterKind=value.kind; _furColor=value.fur; _eyeStyle=value.eyes; _scarfStyle=value.scarf;
    }
};
