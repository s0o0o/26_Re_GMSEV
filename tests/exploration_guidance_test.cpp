#include "../shared/exploration_guidance.hpp"
#include <cassert>
#include <cstring>
int main() {
    field_discovery::Record record;exploration_guidance::State guide;
    assert(!guide.select(record) && !guide.active() && guide.distance({world_regions::Id::SunnyPeak,10,10})==-1);
    record.start();assert(guide.select(record) && guide.target()==1);
    assert(guide.distance({world_regions::Id::SunnyPeak,10,10})==44 && !std::strcmp(guide.direction({world_regions::Id::SunnyPeak,10,10}),"SE"));
    assert(guide.distance({world_regions::Id::Clubhouse,10,10})==-1 && !std::strcmp(guide.direction({world_regions::Id::Clubhouse,10,10}),""));
    assert(guide.distance({world_regions::Id::SunnyPeak,256,10})==-1);
    assert(guide.select(record) && guide.target()==1);
    record.observe({world_regions::Id::SunnyPeak,32,32});guide.update(record);assert(guide.target()==2);
    record.observe({world_regions::Id::Clubhouse,4,16});guide.update(record);assert(guide.target()==2);
    for(const auto& p:field_landmarks::all) {record.observe({world_regions::Id::SunnyPeak,p.x,p.y});guide.update(record);}
    assert(guide.active() && guide.target()==0 && !guide.landmark());
    record.reset();guide.update(record);assert(!guide.active() && !guide.target());
}
