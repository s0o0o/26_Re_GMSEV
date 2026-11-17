#include "../shared/field_journal.hpp"
#include <cassert>
int main(){field_discovery::Record r;field_investigation::State s;assert(field_journal::next(r,s)==0);r.start();assert(field_journal::next(r,s)==1);
    for(const auto& l:field_landmarks::all){world_regions::Position p{world_regions::Id::SunnyPeak,l.x,l.y};r.observe(p);s.open(p,true);s.begin(p,true,l.id*2000);s.tick(l.id*2000+900,p,true);assert(field_journal::completed(s)==static_cast<unsigned>(l.id));}
    assert(r.count()==5 && field_journal::inspected(s)==5 && field_journal::next(r,s)==0);assert(field_journal::distance({world_regions::Id::Clubhouse,12,10},1)==-1);assert(field_journal::distance({world_regions::Id::SunnyPeak,32,32},1)==0);s.reset();r.reset();assert(!field_journal::completed(s));}
