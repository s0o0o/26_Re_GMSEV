#pragma once
#include "resident_quest_log.hpp"
#include "resident_experience_guidance.hpp"
namespace resident_quest_tracking {
class State {
    int pinned_=0;
public:
    int pinned()const{return pinned_;}void reset(){pinned_=0;}
    static bool eligible(int id,const resident_quest_log::Snapshot& s){return s.active && id>=1 && id<=3 && resident_quest_log::running(s.rows[id-1].status);}
    bool select(int id,const resident_quest_log::Snapshot& s){if(id==0){reset();return true;}if(!eligible(id,s))return false;pinned_=id;return true;}
    void reconcile(const resident_quest_log::Snapshot& s){if(pinned_ && !eligible(pinned_,s))reset();}
    int selected(const resident_quest_log::Snapshot& s)const{if(eligible(pinned_,s))return pinned_;for(int id:{2,3,1})if(eligible(id,s))return id;return 0;}
    resident_quest_log::Snapshot decorate(resident_quest_log::Snapshot s,const field_discovery::Record& r,const resident_experience_quest::State& b,const resident_experience_quest::State& p,const field_investigation::State& a){
        reconcile(s);s.pinned=pinned_;const auto id=selected(s);using K=resident_quest_log::GoalKind;
        if(!id)return s;if(id==1){const auto target=!r.visited(1)?1:!r.visited(2)?2:0;s.goal={1,target,100,target?K::Visit:K::Report};return s;}
        const auto goal=resident_experience_guidance::next(s.active,id==2?b:p,a);using E=resident_experience_guidance::Kind;
        s.goal={id,goal.target,goal.residentType,goal.kind==E::Inspect?K::Inspect:goal.kind==E::Experience?K::Experience:goal.kind==E::Report?K::Report:K::None};return s;
    }
};
}
