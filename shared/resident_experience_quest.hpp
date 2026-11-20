#pragma once
#include "town_walk_quest.hpp"
#include "field_investigation.hpp"
namespace resident_experience_quest {
using Stage=town_walk_quest::Stage;
class State {
    Stage stage_=Stage::Available;
    int inspectId_,experienceId_,residentType_;
public:
    explicit State(int inspectId=1,int experienceId=2,int residentType=100):inspectId_(inspectId),experienceId_(experienceId),residentType_(residentType){}
    int inspectId() const{return inspectId_;}int experienceId() const{return experienceId_;}int residentType() const{return residentType_;}
    Stage stage() const {return stage_;}
    void reset(){stage_=Stage::Available;}
    unsigned progress(const field_investigation::State& actions) const {
        return unsigned(actions.inspected(inspectId_))+unsigned(actions.completed(experienceId_));
    }
    void update(bool active,const field_investigation::State& actions){
        if(!active){reset();return;}
        if(stage_==Stage::Active && progress(actions)==2)stage_=Stage::Ready;
    }
    bool accept(bool active,bool validBoss,bool unlocked,const field_investigation::State& actions){
        if(!active || !validBoss || !unlocked || stage_!=Stage::Available || !field_investigation::byId(inspectId_) || !field_investigation::byId(experienceId_))return false;
        stage_=Stage::Active;update(active,actions);return true;
    }
    bool report(bool active,bool validBoss,const field_investigation::State& actions){
        update(active,actions);if(!active || !validBoss || stage_!=Stage::Ready)return false;
        stage_=Stage::Reported;return true;
    }
};
}
