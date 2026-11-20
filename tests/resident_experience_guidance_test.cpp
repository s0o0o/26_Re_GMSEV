#include "../shared/resident_experience_guidance.hpp"
#include <cassert>
int main(){using namespace resident_experience_guidance;resident_experience_quest::State q;field_investigation::State a;
    assert(next(true,q,a).kind==Kind::None);q.accept(true,true,true,a);assert(next(true,q,a).kind==Kind::Inspect && next(true,q,a).target==1);assert(next(false,q,a).kind==Kind::None);
    a.open({world_regions::Id::SunnyPeak,32,32},true);assert(next(true,q,a).kind==Kind::Experience && next(true,q,a).target==2);
    a.open({world_regions::Id::SunnyPeak,90,96},true);a.begin({world_regions::Id::SunnyPeak,90,96},true,1000);a.tick(1900,{world_regions::Id::SunnyPeak,90,96},true);assert(next(true,q,a).kind==Kind::Report);q.report(true,true,a);assert(next(true,q,a).kind==Kind::None);
}
