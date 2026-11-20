#include "../shared/resident_experience_quest.hpp"
#include <cassert>
int main(){using namespace resident_experience_quest;State q;field_investigation::State a;
    assert(!q.accept(false,true,true,a) && !q.accept(true,false,true,a) && !q.accept(true,true,false,a));
    assert(q.accept(true,true,true,a) && q.stage()==Stage::Active && !q.report(true,true,a));
    assert(a.open({world_regions::Id::SunnyPeak,32,32},true));q.update(true,a);assert(q.progress(a)==1 && q.stage()==Stage::Active);
    assert(a.open({world_regions::Id::SunnyPeak,90,96},true));q.update(true,a);assert(q.progress(a)==1);
    assert(a.begin({world_regions::Id::SunnyPeak,90,96},true,1000));a.tick(1899,{world_regions::Id::SunnyPeak,90,96},true);q.update(true,a);assert(q.stage()==Stage::Active);
    a.tick(1900,{world_regions::Id::SunnyPeak,90,96},true);q.update(true,a);assert(q.stage()==Stage::Ready && !q.report(true,false,a));
    assert(q.report(true,true,a) && !q.report(true,true,a));q.reset();assert(q.accept(true,true,true,a) && q.stage()==Stage::Ready);q.update(false,a);assert(q.stage()==Stage::Available);
}
