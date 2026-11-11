#include "../shared/town_behaviour.hpp"
#include "../TP_2023192003_CLIENT/client_sample/town_motion.hpp"
#include <cassert>
#include <cmath>
int main() {
    for(const auto& home : town_npcs::residents) {
        town_behaviour::State state{static_cast<std::uint32_t>(home.id),0}; int x=home.x,y=home.y,moves=0,rests=0;
        for(std::uint64_t time=1000;time<3600000;time+=1000) {
            auto step=town_behaviour::tick(home,x,y,state,time,[](int px,int py) { return px==13 && py==10; });
            assert(town_behaviour::allowed(home,step.x,step.y));
            assert(!(step.x==13 && step.y==10));
            assert(std::abs(step.x-x)+std::abs(step.y-y)<=1);
            auto again=town_behaviour::tick(home,step.x,step.y,state,time,[](int,int){return false;});
            assert(!again.moved); if(step.moved) ++moves; else ++rests; x=step.x; y=step.y;
        }
        if(home.type==102) assert(moves==0 && x==home.x && y==home.y);
        else assert(moves>50 && rests>50);
        town_behaviour::State blocked{123,0};
        for(std::uint64_t time=1000;time<100000;time+=4000)
            assert(!town_behaviour::tick(home,home.x,home.y,blocked,time,[](int,int){return true;}).moved);
    }
    town_motion::State motion;
    motion.move(12,9,1); assert(motion.sample(1).x==12);
    motion.move(13,9,2); assert(motion.facing==town_motion::Right);
    assert(std::abs(motion.sample(2.175f).x-12.5f)<.001f);
    const auto interrupted=motion.sample(2.175f); motion.move(13,10,2.175f);
    assert(motion.facing==town_motion::Down && motion.sample(2.175f).x==interrupted.x);
    assert(motion.sample(3).x==13 && motion.sample(3).y==10);
    motion.move(40,10,4); assert(motion.sample(4).x==40); // Distant updates snap, not glide across the world.
}
