#pragma once
#include "field_investigation.hpp"
namespace field_journal {
inline unsigned inspected(const field_investigation::State& s){unsigned n=0;for(int i=1;i<=5;++i)if(s.inspected(i))++n;return n;}
inline unsigned completed(const field_investigation::State& s){unsigned n=0;for(int i=1;i<=5;++i)if(s.completed(i))++n;return n;}
inline int next(const field_discovery::Record& record,const field_investigation::State& s){if(!record.active())return 0;for(int i=1;i<=5;++i)if(!s.completed(i))return i;return 0;}
inline int distance(world_regions::Position p,int id){auto l=field_investigation::byId(id);return l && p.region==world_regions::Id::SunnyPeak && world_regions::valid(p)?std::abs(l->x-p.x)+std::abs(l->y-p.y):-1;}
}
