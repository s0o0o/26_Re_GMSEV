#pragma once
#include "world_regions.hpp"
namespace world_layout {
constexpr int fieldX = world_regions::fieldOrigin, fieldWidth = world_regions::fieldWidth;
constexpr int width = fieldX+fieldWidth, height = 18;
constexpr bool outdoors(int x) { return x >= fieldX; }
constexpr bool fieldWalkable(int x,int y) {
    return x >= 2 && x <= 17 && y >= 5 && y <= 12
        && !(x >= 9 && x <= 13 && y == 5);
}
constexpr bool walkable(int x,int y) {
    const auto p=world_regions::decode(x,y);
    return p.region==world_regions::Id::SunnyPeak ? fieldWalkable(p.x,p.y)
        : p.region==world_regions::Id::Clubhouse && clubhouse::walkable(p.x,p.y);
}
inline void safePosition(int& x,int& y) {
    if (!walkable(x,y)) { x=clubhouse::spawnX; y=clubhouse::spawnY; }
}
// Called only after a successful step. Arrival tiles are not portal tiles.
inline bool portal(int& x,int& y) {
    const auto current=world_regions::decode(x,y);
    world_regions::Position destination;
    if(current.region==world_regions::Id::Clubhouse && current.x==10 && current.y==14)
        destination={world_regions::Id::SunnyPeak,10,10};
    else if(current.region==world_regions::Id::SunnyPeak && current.x==10 && current.y==12)
        destination={world_regions::Id::Clubhouse,10,12};
    else return false;
    world_regions::LegacyPosition encoded;
    if(!world_regions::encode(destination,encoded)) return false;
    x=encoded.x; y=encoded.y; return true;
}
constexpr int clamp(int value,int maximum) { return value < 0 ? 0 : (value > maximum ? maximum : value); }
constexpr int cameraX(int x) {
    return outdoors(x) ? fieldX+clamp(x-fieldX-9,fieldWidth-18) : clamp(x-9,clubhouse::width-18);
}
constexpr int cameraY(int y) { return clamp(y-8,height-16); }
}
