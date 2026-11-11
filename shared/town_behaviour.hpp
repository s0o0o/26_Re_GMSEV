#pragma once
#include "town_npcs.hpp"
#include "respawn_policy.hpp"
namespace town_behaviour {
struct State { std::uint32_t random=1; respawn_policy::Tick next=0; };
struct Step { int x,y; bool moved; };
inline std::uint32_t nextRandom(State& s) { s.random=s.random*1664525u+1013904223u; return s.random; }
inline bool allowed(const town_npcs::Resident& home,int x,int y) {
    return world_regions::decode(x,y).region==world_regions::Id::Clubhouse && clubhouse::walkable(x,y)
        && x>=home.x-2 && x<=home.x+2 && y>=home.y-2 && y<=home.y+2
        && !(x==10 && y==14) && !(x==clubhouse::spawnX && y==clubhouse::spawnY);
}
template<class Occupied>
Step tick(const town_npcs::Resident& home,int x,int y,State& state,respawn_policy::Tick now,Occupied occupied) {
    if(home.type==102 || (state.next && now<state.next)) return {x,y,false};
    const auto random=nextRandom(state);
    state.next=now+2000+(random%3)*1000;
    const int dx[]={1,-1,0,0,0},dy[]={0,0,1,-1,0};
    const auto choice=(random>>8)%5; // Include deliberate rests, not constant marching.
    const int nx=x+dx[choice],ny=y+dy[choice];
    if((nx==x && ny==y) || !allowed(home,nx,ny) || occupied(nx,ny)) return {x,y,false};
    return {nx,ny,true};
}
}
