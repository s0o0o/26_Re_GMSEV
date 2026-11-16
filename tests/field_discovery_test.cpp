#include "../shared/field_discovery.hpp"
#include <cassert>
int main() {
    field_discovery::Record r;
    const auto a=field_landmarks::all[0];
    assert(!r.observe({world_regions::Id::SunnyPeak,a.x,a.y}) && !r.count());
    r.start();assert(r.active() && !r.visited(0) && !r.visited(6));
    assert(!r.observe({world_regions::Id::Clubhouse,a.x,a.y}));
    assert(!r.observe({world_regions::Id::SunnyPeak,-1,0}));
    assert(!r.observe({world_regions::Id::SunnyPeak,256,256}));
    assert(!r.observe({world_regions::Id::SunnyPeak,a.x+3,a.y}));
    assert(r.observe({world_regions::Id::SunnyPeak,a.x+2,a.y})==1 && r.count()==1);
    assert(!r.observe({world_regions::Id::SunnyPeak,a.x,a.y}) && r.count()==1);
    r.start();assert(r.count()==1); // Duplicate avatar info is not a new connection.
    assert(!r.observe({world_regions::Id::Clubhouse,10,10}) && r.count()==1);
    for(const auto& p:field_landmarks::all) r.observe({world_regions::Id::SunnyPeak,p.x,p.y});
    assert(r.count()==5);for(int id=1;id<=5;++id)assert(r.visited(id));
    r.reset();r.start();
    assert(!r.observe({world_regions::Id::SunnyPeak,110,130}) && !r.count()); // Cliff near site3, but blocked.
    for(int y=0;y<256;++y) for(int x=0;x<256;++x) {
        r.observe({world_regions::Id::SunnyPeak,x,y});assert(r.count()<=5);
    }
    assert(r.count()==5);r.reset();assert(!r.active() && !r.count());
}
