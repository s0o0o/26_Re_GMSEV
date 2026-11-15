#pragma once
#include "field_terrain.hpp"
#include <array>
#include <list>
#include <cstddef>
#include <algorithm>

namespace field_chunks {
constexpr int side=16;
struct Stats { std::size_t resident=0,hits=0,misses=0,evictions=0; };
struct Chunk {
    int x=0,y=0;
    std::array<field_terrain::Tile,side*side> cells{};
};
inline Chunk generate(int cx,int cy) {
    Chunk chunk; chunk.x=cx; chunk.y=cy;
    if(cx<0 || cy<0 || cx>=(world_regions::fieldWidth+side-1)/side || cy>=(world_regions::fieldHeight+side-1)/side) {
        chunk.cells.fill(field_terrain::Tile::Cliff); return chunk;
    }
    for(int y=0;y<side;++y) for(int x=0;x<side;++x)
        chunk.cells[y*side+x]=field_terrain::at(cx*side+x,cy*side+y);
    return chunk;
}
// Immutable terrain only. Dynamic pickups must live outside this evictable cache.
// Caller owns synchronization; the server uses mapMutex.
class Cache {
    std::list<Chunk> lru_;
    std::size_t capacity_;
    Stats stats_;
public:
    explicit Cache(std::size_t capacity=16) : capacity_((std::max)(std::size_t{1},capacity)) {}
    void clear() { lru_.clear(); stats_={}; }
    Stats stats() const { auto s=stats_; s.resident=lru_.size(); return s; }
    field_terrain::Tile at(world_regions::Position p) {
        if(p.region!=world_regions::Id::SunnyPeak || !world_regions::valid(p)) return field_terrain::Tile::Cliff;
        const int cx=p.x/side,cy=p.y/side;
        auto found=std::find_if(lru_.begin(),lru_.end(),[&](const Chunk& c) { return c.x==cx && c.y==cy; });
        if(found!=lru_.end()) { ++stats_.hits; lru_.splice(lru_.begin(),lru_,found); }
        else {
            ++stats_.misses;
            if(lru_.size()==capacity_) { lru_.pop_back(); ++stats_.evictions; }
            lru_.push_front(generate(cx,cy));
        }
        return lru_.front().cells[(p.y%side)*side+p.x%side];
    }
};
struct Bounds { int left=0,top=0,right=-1,bottom=-1; };
inline Bounds visible(int left,int top,int columns=18,int rows=16) {
    if(columns<=0 || rows<=0 || left<0 || top<0 || left>=world_regions::fieldWidth || top>=world_regions::fieldHeight) return {};
    // Clamp lengths before adding, so malformed dimensions cannot overflow.
    return {left/side,top/side,
        (left+(std::min)(columns,world_regions::fieldWidth-left)-1)/side,
        (top+(std::min)(rows,world_regions::fieldHeight-top)-1)/side};
}
inline bool onScreen(world_regions::Position p,world_regions::Id region,int left,int top,int columns=18,int rows=16,int margin=0) {
    const auto d=world_regions::describe(region);
    return world_regions::valid(p) && p.region==region && left>=0 && top>=0 && left<d.width && top<d.height
        && columns>0 && rows>0 && margin>=0 && margin<=2
        && p.x>=left-margin && p.y>=top-margin
        && p.x-left<(std::min)(columns,d.width)+margin && p.y-top<(std::min)(rows,d.height)+margin;
}
}
