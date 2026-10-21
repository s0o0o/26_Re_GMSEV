#pragma once
#include "world_layout.hpp"
#include <array>
namespace field_content {
struct Point { int x,y; };
constexpr std::array<Point,3> seeds = {{{36,8},{46,7},{47,11}}};
constexpr std::array<Point,3> monsters = {{{36,6},{47,6},{48,10}}};
constexpr bool monsterCanStand(int x,int y) {
    return world_layout::outdoors(x) && world_layout::walkable(x,y)
        && !(x == 42 && y >= 10); // Arrival / exit corridor stays clear.
}
}
