#pragma once
#include "field_discovery.hpp"
namespace exploration_guidance {
class State {
    bool active_=false;
    int target_=0;
    static int next(const field_discovery::Record& record) {for(const auto& p:field_landmarks::all)if(!record.visited(p.id))return p.id;return 0;}
public:
    bool active() const {return active_;}
    int target() const {return target_;}
    const field_landmarks::Landmark* landmark() const {return target_>=1 && target_<=5?&field_landmarks::all[target_-1]:nullptr;}
    void reset() {active_=false;target_=0;}
    bool select(const field_discovery::Record& record) {if(!record.active()){reset();return false;}active_=true;target_=next(record);return true;}
    void update(const field_discovery::Record& record) {
        if(!record.active()){reset();return;}
        if(active_ && target_ && record.visited(target_))target_=next(record);
    }
    int distance(world_regions::Position p) const {
        const auto* goal=landmark();if(!goal || p.region!=world_regions::Id::SunnyPeak || !world_regions::valid(p))return -1;
        return (p.x>goal->x?p.x-goal->x:goal->x-p.x)+(p.y>goal->y?p.y-goal->y:goal->y-p.y);
    }
    const char* direction(world_regions::Position p) const {
        const auto* goal=landmark();if(!goal || distance(p)<0)return "";
        const int dx=goal->x-p.x,dy=goal->y-p.y;
        if(!dx && !dy)return "HERE";
        if(dy<0)return dx<0?"NW":dx>0?"NE":"N";
        if(dy>0)return dx<0?"SW":dx>0?"SE":"S";
        return dx<0?"W":"E";
    }
};
}
