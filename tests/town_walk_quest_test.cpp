#include "../shared/town_walk_quest.hpp"
#include <cassert>
int main() {
    using town_walk_quest::Stage;town_walk_quest::State q;field_discovery::Record r;
    assert(!q.accept(r,true));r.start();assert(!q.accept(r,false));assert(!q.report(r,true));
    assert(q.accept(r,true) && q.stage()==Stage::Active);assert(!q.accept(r,true));
    assert(!q.report(r,true));r.observe({world_regions::Id::SunnyPeak,32,32});q.update(r);assert(q.progress(r)==1 && q.stage()==Stage::Active);
    r.observe({world_regions::Id::SunnyPeak,90,96});q.update(r);assert(q.stage()==Stage::Ready);
    assert(!q.report(r,false));assert(q.report(r,true) && q.stage()==Stage::Reported);assert(!q.report(r,true));
    r.reset();q.update(r);assert(q.stage()==Stage::Available);
    r.start();r.observe({world_regions::Id::SunnyPeak,32,32});r.observe({world_regions::Id::SunnyPeak,90,96});
    assert(q.accept(r,true) && q.stage()==Stage::Ready); // Same-connection visits before accepting are credited.
    q.reset();assert(q.stage()==Stage::Available);
}
