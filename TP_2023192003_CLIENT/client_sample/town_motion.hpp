#pragma once
#include <algorithm>
#include <cmath>
namespace town_motion {
enum Facing { Down=0,Up=1,Left=2,Right=3 };
struct Point { float x=0,y=0; };
struct State {
    Point from,to; float started=0; Facing facing=Down; bool initialized=false;
    Point sample(float now) const {
        const auto t=(std::max)(0.f,(std::min)(1.f,(now-started)/.35f));
        return {from.x+(to.x-from.x)*t,from.y+(to.y-from.y)*t};
    }
    void move(int x,int y,float now) {
        const Point destination{static_cast<float>(x),static_cast<float>(y)};
        if(!initialized || std::abs(destination.x-to.x)+std::abs(destination.y-to.y)>1.1f) {
            from=to=destination; started=now; initialized=true; facing=Down; return;
        }
        if(destination.x==to.x && destination.y==to.y) return;
        facing=destination.x>to.x ? Right : destination.x<to.x ? Left : destination.y<to.y ? Up : Down;
        from=sample(now); to=destination; started=now;
    }
};
}
