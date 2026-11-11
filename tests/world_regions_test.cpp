#include "../shared/world_regions.hpp"
#include "../shared/world_layout.hpp"
#include <cassert>
#include <climits>
#include <initializer_list>
int main() {
    using namespace world_regions;
    for(auto id : {Id::Clubhouse,Id::SunnyPeak}) {
        const auto d=describe(id);
        for(int y=0;y<d.height;++y) for(int x=0;x<d.width;++x) {
            LegacyPosition legacy; assert(encode({id,x,y},legacy));
            auto p=decode(legacy.x,legacy.y); assert(p.region==id && p.x==x && p.y==y);
        }
    }
    assert(!same(Position{Id::Clubhouse,10,10},Position{Id::SunnyPeak,10,10}));
    assert(!inRange({Id::Clubhouse,23,8},{Id::SunnyPeak,0,8},100));
    assert(inRange({Id::SunnyPeak,3,8},{Id::SunnyPeak,4,9},1));
    for(auto p : {LegacyPosition{-1,8},{24,8},{31,8},{52,8},{10,-1},{10,18},{INT_MAX,INT_MIN}})
        assert(decode(p.x,p.y).region==Id::Invalid && !same(p.x,p.y,p.x,p.y));
    LegacyPosition unchanged{12,13};
    for(auto p : {Position{Id::Invalid,0,0},{Id::Clubhouse,24,0},{Id::SunnyPeak,20,0},{Id::SunnyPeak,0,-1}}) {
        assert(!encode(p,unchanged)); assert(unchanged.x==12 && unchanged.y==13);
    }
    assert(!combatAllowed(10,10) && combatAllowed(42,10) && !combatAllowed(999,10));
    int x=10,y=14; assert(world_layout::portal(x,y) && decode(x,y).region==Id::SunnyPeak);
    y=12; assert(world_layout::portal(x,y) && decode(x,y).region==Id::Clubhouse);
}
