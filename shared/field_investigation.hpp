#pragma once
#include "field_discovery.hpp"
#include <cstdlib>
#include <algorithm>
namespace field_investigation {
inline const field_landmarks::Landmark* byId(int id){return id>=1 && id<=5?&field_landmarks::all[id-1]:nullptr;}
inline bool eligible(world_regions::Position p,int id){
    const auto* l=byId(id);if(!l || p.region!=world_regions::Id::SunnyPeak || !world_regions::valid(p) || !field_terrain::walkable(p.x,p.y))return false;
    const int dx=l->x-p.x,dy=l->y-p.y,d=std::abs(dx)+std::abs(dy);if(d>2)return false;if(d<2)return true;
    return (dx && field_terrain::walkable(p.x+(dx>0?1:-1),p.y)) || (dy && field_terrain::walkable(p.x,p.y+(dy>0?1:-1)));
}
inline int nearest(world_regions::Position p){int best=0,distance=99;for(const auto& l:field_landmarks::all)if(eligible(p,l.id)){const int d=std::abs(l.x-p.x)+std::abs(l.y-p.y);if(d<distance){distance=d;best=l.id;}}return best;}
class State {
    int target_=0;std::uint8_t inspected_=0,completed_=0;bool running_=false;std::uint64_t began_=0,next_=0;
public:
    bool visible()const{return target_!=0;}
    int target()const{return target_;}
    bool running()const{return running_;}
    bool cooling(std::uint64_t now)const{return now<next_;}
    bool completed(int id)const{return byId(id) && (completed_&(1u<<(id-1)))!=0;}
    float progress(std::uint64_t now)const{return running_?static_cast<float>((std::min)(std::uint64_t{900},now>=began_?now-began_:0))/900.f:0.f;}
    void cancel(){running_=false;}
    bool begin(world_regions::Position p,bool active,std::uint64_t now){if(!active || !visible() || !eligible(p,target_) || running_ || now<next_)return false;running_=true;began_=now;next_=now+1500;return true;}
    bool tick(std::uint64_t now,world_regions::Position p,bool active){reconcile(p,active);if(!running_)return false;if(progress(now)<1)return false;running_=false;completed_=static_cast<std::uint8_t>(completed_|(1u<<(target_-1)));return true;}
    bool inspected(int id)const{return byId(id) && (inspected_&(1u<<(id-1)))!=0;}
    void close(){target_=0;cancel();}
    void reset(){close();inspected_=completed_=0;began_=next_=0;}
    bool open(world_regions::Position p,bool active){const auto id=active?nearest(p):0;if(!id)return false;target_=id;inspected_=static_cast<std::uint8_t>(inspected_|(1u<<(id-1)));return true;}
    void reconcile(world_regions::Position p,bool active){if(!active || !eligible(p,target_))close();}
};
}
