#pragma once
#include "resident_experience_quest.hpp"
namespace resident_experience_guidance {
enum class Kind {None,Inspect,Experience,Report};
struct Goal {Kind kind=Kind::None;int target=0;int residentType=100;};
inline Goal next(bool active,const resident_experience_quest::State& quest,const field_investigation::State& actions){
    using Stage=resident_experience_quest::Stage;
    if(!active || quest.stage()==Stage::Available || quest.stage()==Stage::Reported)return {};
    if(!actions.inspected(quest.inspectId()))return {Kind::Inspect,quest.inspectId(),quest.residentType()};
    if(!actions.completed(quest.experienceId()))return {Kind::Experience,quest.experienceId(),quest.residentType()};
    return {Kind::Report,0,quest.residentType()};
}
inline const resident_experience_quest::State& preferred(bool active,const resident_experience_quest::State& boss,const resident_experience_quest::State& pashmina,const field_investigation::State& actions){
    return next(active,boss,actions).kind!=Kind::None?boss:next(active,pashmina,actions).kind!=Kind::None?pashmina:boss;
}
}
