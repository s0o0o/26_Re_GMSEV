#pragma once
#include "clubhouse_map.hpp"
namespace world_layout {
constexpr int fieldX = 32, fieldWidth = 20;
constexpr int width = fieldX+fieldWidth, height = 18;
constexpr bool outdoors(int x) { return x >= fieldX; }
constexpr bool fieldWalkable(int x,int y) {
    return x >= 2 && x <= 17 && y >= 5 && y <= 12
        && !(x >= 9 && x <= 13 && y == 5);
}
constexpr bool walkable(int x,int y) {
    return outdoors(x) ? fieldWalkable(x-fieldX,y) : clubhouse::walkable(x,y);
}
inline void safePosition(int& x,int& y) {
    if (!walkable(x,y)) { x=clubhouse::spawnX; y=clubhouse::spawnY; }
}
// Called only after a successful step. Arrival tiles are not portal tiles.
inline bool portal(int& x,int& y) {
    if (x == 10 && y == 14) { x=fieldX+10; y=10; return true; }
    if (x == fieldX+10 && y == 12) { x=10; y=12; return true; }
    return false;
}
constexpr int clamp(int value,int maximum) { return value < 0 ? 0 : (value > maximum ? maximum : value); }
constexpr int cameraX(int x) {
    return outdoors(x) ? fieldX+clamp(x-fieldX-9,fieldWidth-18) : clamp(x-9,clubhouse::width-18);
}
constexpr int cameraY(int y) { return clamp(y-8,height-16); }
}
