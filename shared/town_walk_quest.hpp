#pragma once
#include "field_discovery.hpp"
namespace town_walk_quest {
enum class Stage { Available,Active,Ready,Reported };
class State {
    Stage stage_=Stage::Available;
public:
    Stage stage() const {return stage_;}
    void reset() {stage_=Stage::Available;}
    unsigned progress(const field_discovery::Record& record) const {return unsigned(record.visited(1))+unsigned(record.visited(2));}
    void update(const field_discovery::Record& record) {
        if(!record.active()){reset();return;}
        if(stage_==Stage::Active && progress(record)==2)stage_=Stage::Ready;
    }
    bool accept(const field_discovery::Record& record,bool validBoss) {
        if(!validBoss || !record.active() || stage_!=Stage::Available)return false;
        stage_=Stage::Active;update(record);return true;
    }
    bool report(const field_discovery::Record& record,bool validBoss) {
        update(record);if(!validBoss || !record.active() || stage_!=Stage::Ready)return false;
        stage_=Stage::Reported;return true;
    }
};
}
