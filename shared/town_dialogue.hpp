#pragma once
#include "town_npcs.hpp"
#include <array>
#include <cstdlib>
namespace town_dialogue {
struct Candidate { int id=-1,type=0;world_regions::Position position{};bool present=false; };
using Candidates=std::array<Candidate,3>;
inline bool eligible(world_regions::Position player,const Candidate& c) {
    const auto* resident=town_npcs::byType(c.type);
    if(!c.present || !resident || resident->id!=c.id || player.region!=world_regions::Id::Clubhouse
        || c.position.region!=player.region || !world_regions::valid(player) || !world_regions::valid(c.position)
        || !clubhouse::walkable(player.x,player.y) || !clubhouse::walkable(c.position.x,c.position.y)) return false;
    const int dx=std::abs(player.x-c.position.x),dy=std::abs(player.y-c.position.y);
    if(dx+dy>2) return false;
    if(dx==2) return clubhouse::walkable((player.x+c.position.x)/2,player.y);
    if(dy==2) return clubhouse::walkable(player.x,(player.y+c.position.y)/2);
    if(dx==1 && dy==1) return clubhouse::walkable(player.x,c.position.y) || clubhouse::walkable(c.position.x,player.y);
    return true;
}
inline const Candidate* nearest(world_regions::Position player,const Candidates& candidates) {
    const Candidate* result=nullptr;int distance=3;
    for(const auto& c:candidates) if(eligible(player,c)) {
        const int d=std::abs(player.x-c.position.x)+std::abs(player.y-c.position.y);
        if(d<distance || (d==distance && (!result || c.id<result->id))) {result=&c;distance=d;}
    }
    return result;
}
struct State {
    int target=-1,type=0,page=0;
    bool visible() const {return target>=0;}
    void close() {target=-1;type=page=0;}
    bool open(world_regions::Position player,const Candidates& candidates) {
        const auto* c=nearest(player,candidates);if(!c)return false;
        target=c->id;type=c->type;page=0;return true;
    }
    void reconcile(world_regions::Position player,const Candidates& candidates,bool connected) {
        if(!visible())return;
        for(const auto& c:candidates) if(c.id==target && c.type==type && connected && eligible(player,c)) return;
        close();
    }
    void next() {if(!visible())return;if(page==0)page=1;else close();}
};
}
