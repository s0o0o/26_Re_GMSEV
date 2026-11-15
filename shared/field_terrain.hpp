#pragma once
#include "world_regions.hpp"

// Authored, deterministic terrain: rendering and collision share this source.
namespace field_terrain {
enum class Tile : std::uint8_t { Grass, Trail, Water, Cliff, Bridge, StarterWall };
constexpr bool starter(int x,int y) { return x>=0 && x<20 && y>=0 && y<18; }
constexpr bool bridgeRow(int y) {
    return (y>=31 && y<=33) || (y>=95 && y<=97)
        || (y>=159 && y<=161) || (y>=223 && y<=225);
}
constexpr bool cliffPass(int x) {
    return (x>=31 && x<=33) || (x>=111 && x<=113)
        || (x>=191 && x<=193) || (x>=239 && x<=241);
}
constexpr Tile at(int x,int y) {
    if(x<1 || y<1 || x>=world_regions::fieldWidth-1 || y>=world_regions::fieldHeight-1) return Tile::Cliff;
    if(starter(x,y)) {
        if(x>=17 && y>=8 && y<=10) return Tile::Trail; // East connection, including old right-hand wall.
        return x>=2 && x<=17 && y>=5 && y<=12 && !(x>=9 && x<=13 && y==5)
            ? Tile::Grass : Tile::StarterWall;
    }
    if(x>=80 && x<=83) return bridgeRow(y) ? Tile::Bridge : Tile::Water;
    if(y>=128 && y<=130 && !cliffPass(x)) return Tile::Cliff;
    if((x>=20 && x<=33 && y>=8 && y<=10) || x%32==0 || y%32==0
       || (y>=128 && y<=130 && cliffPass(x))) return Tile::Trail;
    return Tile::Grass;
}
constexpr bool walkable(Tile tile) {
    return tile==Tile::Grass || tile==Tile::Trail || tile==Tile::Bridge;
}
constexpr bool walkable(int x,int y) { return walkable(at(x,y)); }
}
