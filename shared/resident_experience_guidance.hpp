#pragma once
#include "resident_experience_quest.hpp"
namespace resident_experience_guidance {
enum class Kind {None,Inspect,Experience,Report};
struct Goal {Kind kind=Kind::None;int target=0;};
inline Goal next(bool active,const resident_experience_quest::State& quest,const field_investigation::State& actions){
    using Stage=resident_experience_quest::Stage;
    if(!active || quest.stage()==Stage::Available || quest.stage()==Stage::Reported)return {};
    if(!actions.inspected(1))return {Kind::Inspect,1};
    if(!actions.completed(2))return {Kind::Experience,2};
    return {Kind::Report,0};
}
}
