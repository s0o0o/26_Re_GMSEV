#include "../shared/resident_quest_tracking.hpp"
#include <cassert>
int main(){using namespace resident_quest_log;resident_quest_tracking::State t;Snapshot s;s.active=true;s.rows={{{1,Status::Active,0},{2,Status::Locked,0},{3,Status::Active,0}}};assert(t.selected(s)==3 && !t.select(2,s));assert(t.select(1,s) && t.selected(s)==1);s.rows[0].status=Status::Reported;t.reconcile(s);assert(!t.pinned() && t.selected(s)==3);s.active=false;assert(!t.select(3,s));
    town_walk_quest::State w;resident_experience_quest::State b,p{3,4,101};field_discovery::Record r;r.start();field_investigation::State a;w.accept(r,true);auto log=snapshot(true,w,r,b,p,a);assert(t.select(1,log));auto d=t.decorate(log,r,b,p,a);assert(d.goal.kind==GoalKind::Visit && d.goal.target==1);t.select(0,log);assert(!t.pinned());
}
