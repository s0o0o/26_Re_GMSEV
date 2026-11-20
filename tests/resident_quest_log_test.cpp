#include "../shared/resident_quest_log.hpp"
#include <cassert>
int main(){using namespace resident_quest_log;town_walk_quest::State w;resident_experience_quest::State b,p{3,4,101};field_discovery::Record r;field_investigation::State a;
    auto s=snapshot(false,w,r,b,p,a);assert(!s.active && s.rows[1].status==Status::Locked);r.start();p.accept(true,true,true,a);s=snapshot(true,w,r,b,p,a);assert(s.rows[2].status==Status::Active);
    r.observe({world_regions::Id::SunnyPeak,32,32});r.observe({world_regions::Id::SunnyPeak,90,96});w.accept(r,true);w.report(r,true);s=snapshot(true,w,r,b,p,a);assert(s.rows[0].status==Status::Reported && s.rows[0].progress==2 && s.rows[1].status==Status::Available);assert(w.stage()==town_walk_quest::Stage::Reported && b.stage()==town_walk_quest::Stage::Available);
}
