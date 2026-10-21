#pragma once
namespace clubhouse {
constexpr int width = 24, height = 18;
constexpr int spawnX = 11, spawnY = 12;
constexpr int sourceTile = 16;
// Lower clubhouse floor. Walls, stairs, furniture and atlas padding stay blocked.
constexpr bool walkable(int x,int y) {
    return x >= 2 && x <= 21 && y >= 8 && y <= 16
        && !(y == 8 && x >= 14)
        && !(x >= 3 && x <= 9 && y >= 8 && y <= 14)
        && !(x >= 15 && x <= 18 && y >= 10 && y <= 13)
        && !(x >= 16 && x <= 18 && y >= 14);
}
inline void safePosition(int& x,int& y) {
    if (!walkable(x,y)) { x=spawnX; y=spawnY; }
}
}
