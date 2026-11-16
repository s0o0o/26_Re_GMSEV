#pragma once
#include "field_terrain.hpp"
#include <array>
namespace field_landmarks {
struct Landmark { int id,x,y; const char* name; };
inline constexpr std::array<Landmark,5> all={{{1,32,32,"Flower Walk"},{2,90,96,"Riverside Rest"},
    {3,112,132,"Cliff Foot"},{4,192,192,"Stump Glade"},{5,240,224,"East Meadow"}}};
constexpr int zone(int x,int y) { return (y>=128 ? 2 : 0)+(x>=84 ? 1 : 0); }
constexpr unsigned detail(int x,int y) {
    if(field_terrain::starter(x,y) || field_terrain::at(x,y)!=field_terrain::Tile::Grass) return 0;
    const auto h=static_cast<unsigned>(x)*73856093u ^ static_cast<unsigned>(y)*19349663u;
    return h%13==0 ? 1+h%3 : 0;
}
constexpr bool nearby(const Landmark& p,int left,int top) {
    return p.x>=left-3 && p.x<left+21 && p.y>=top-3 && p.y<top+19;
}
}
