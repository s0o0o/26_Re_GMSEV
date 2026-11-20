#include "../shared/resident_experience_guidance.hpp"
#include <cassert>
int main(){using namespace resident_experience_quest;State boss,pash{3,4,101};field_investigation::State a;
    assert(!pash.accept(false,true,true,a) && !pash.accept(true,false,true,a));assert(pash.accept(true,true,true,a));assert(pash.stage()==Stage::Active && !pash.report(true,true,a));
    a.open({world_regions::Id::SunnyPeak,112,132},true);assert(pash.progress(a)==1 && boss.progress(a)==0);assert(resident_experience_guidance::next(true,pash,a).target==4);
    a.open({world_regions::Id::SunnyPeak,192,192},true);a.begin({world_regions::Id::SunnyPeak,192,192},true,1000);a.tick(1900,{world_regions::Id::SunnyPeak,192,192},true);pash.update(true,a);assert(pash.stage()==Stage::Ready);assert(!pash.report(true,false,a));
    assert(resident_experience_guidance::next(true,pash,a).residentType==101);boss.accept(true,true,true,a);assert(&resident_experience_guidance::preferred(true,boss,pash,a)==&boss);
    a.open({world_regions::Id::SunnyPeak,32,32},true);a.open({world_regions::Id::SunnyPeak,90,96},true);a.begin({world_regions::Id::SunnyPeak,90,96},true,3000);a.tick(3900,{world_regions::Id::SunnyPeak,90,96},true);assert(boss.report(true,true,a));assert(&resident_experience_guidance::preferred(true,boss,pash,a)==&pash);
    assert(pash.report(true,true,a) && !pash.report(true,true,a));pash.reset();assert(pash.inspectId()==3 && pash.experienceId()==4 && pash.residentType()==101);assert(pash.accept(true,true,true,a) && pash.stage()==Stage::Ready);pash.update(false,a);assert(pash.stage()==Stage::Available);
}
