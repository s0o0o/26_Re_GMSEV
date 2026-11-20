#pragma once
#include "resident_experience_quest.hpp"
#include <array>
namespace resident_quest_log {
enum class Status {Locked,Available,Active,Ready,Reported};
struct Row {int id=0;Status status=Status::Available;unsigned progress=0;};
struct Snapshot {bool active=false;std::array<Row,3> rows{};};
inline Status status(town_walk_quest::Stage s){using S=town_walk_quest::Stage;return s==S::Active?Status::Active:s==S::Ready?Status::Ready:s==S::Reported?Status::Reported:Status::Available;}
inline bool running(Status s){return s==Status::Active || s==Status::Ready;}
inline Snapshot snapshot(bool active,const town_walk_quest::State& walk,const field_discovery::Record& record,const resident_experience_quest::State& boss,const resident_experience_quest::State& pash,const field_investigation::State& actions){
    Snapshot s;s.active=active;s.rows={{{1,status(walk.stage()),walk.progress(record)},
        {2,walk.stage()==town_walk_quest::Stage::Reported?status(boss.stage()):Status::Locked,boss.progress(actions)},
        {3,status(pash.stage()),pash.progress(actions)}}};return s;
}
}
