#include "../shared/resident_quest_log.hpp"
#include <cassert>
int main(){using namespace resident_quest_log;Snapshot s;s.active=true;s.rows={{{1,Status::Reported,2},{2,Status::Active,0},{3,Status::Ready,2}}};auto v=visible(s,true);assert(v.count==2 && v.indices[0]==1 && v.indices[1]==2 && v.position(2)==1);assert(visible(s,false).count==3);s.rows[1].status=Status::Locked;s.rows[2].status=Status::Available;assert(!visible(s,true).count);s.active=false;assert(!visible(s,false).count);}
