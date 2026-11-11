#pragma once
#include "clubhouse_map.hpp"
#include <cstdint>

// Region identity is explicit; legacy X/Y is a temporary storage/wire adapter.
namespace world_regions {
enum class Id : std::uint8_t { Invalid=0,Clubhouse=1,SunnyPeak=2 };
constexpr int fieldOrigin=32,fieldWidth=20,fieldHeight=18;
struct Position { Id region=Id::Invalid; int x=0,y=0; };
struct LegacyPosition { int x=0,y=0; };
struct Descriptor { Id id; int originX,width,height; const char* name; bool combat; };
constexpr Descriptor describe(Id id) {
    switch(id) {
    case Id::Clubhouse: return {id,0,clubhouse::width,clubhouse::height,"Club House",false};
    case Id::SunnyPeak: return {id,fieldOrigin,fieldWidth,fieldHeight,"Sunny Peak",true};
    default: return {Id::Invalid,0,0,0,"Unknown",false};
    }
}
constexpr bool valid(Position p) {
    const auto d=describe(p.region);
    return d.id!=Id::Invalid && p.x>=0 && p.y>=0 && p.x<d.width && p.y<d.height;
}
constexpr Position decode(int x,int y) {
    if(x>=0 && x<clubhouse::width && y>=0 && y<clubhouse::height) return {Id::Clubhouse,x,y};
    if(x>=fieldOrigin && x<fieldOrigin+fieldWidth && y>=0 && y<fieldHeight) return {Id::SunnyPeak,x-fieldOrigin,y};
    return {};
}
inline bool encode(Position p,LegacyPosition& result) {
    if(!valid(p)) return false; // Do not silently turn an invalid position into the origin.
    result={describe(p.region).originX+p.x,p.y}; return true;
}
constexpr bool same(Position a,Position b) { return valid(a) && valid(b) && a.region==b.region; }
constexpr bool same(int ax,int ay,int bx,int by) { return same(decode(ax,ay),decode(bx,by)); }
constexpr bool inRange(Position a,Position b,int range) {
    if(!same(a,b) || range<0) return false;
    const auto dx=a.x-b.x,dy=a.y-b.y; // Valid local coordinates are bounded by descriptors.
    return dx>=-range && dx<=range && dy>=-range && dy<=range;
}
constexpr bool combatAllowed(int x,int y) {
    const auto p=decode(x,y); return valid(p) && describe(p.region).combat;
}
}
