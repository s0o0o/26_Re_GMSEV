#include "../shared/field_investigation.hpp"
#include <cassert>
int main(){using namespace field_investigation;State s;
    for(const auto& l:field_landmarks::all){world_regions::Position p{world_regions::Id::SunnyPeak,l.x,l.y};assert(eligible(p,l.id));assert(!s.open(p,false));assert(s.open(p,true) && s.inspected(l.id));s.reconcile({world_regions::Id::Clubhouse,12,10},true);assert(!s.visible() && s.inspected(l.id));assert(!eligible({world_regions::Id::SunnyPeak,l.x+3,l.y},l.id));}
    assert(!eligible({world_regions::Id::SunnyPeak,0,0},0));s.reset();assert(!s.inspected(1));
}
