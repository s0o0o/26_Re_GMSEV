#pragma once
#include <cstdint>
#include "world_regions.hpp"

namespace world_travel {
enum class Action : std::uint8_t { EnterField=1, ReturnHome=2 };
enum class Result : std::uint8_t {
    Success=0, AlreadyThere=1, InvalidRequest=2, StaleRequest=3, Blocked=4
};
constexpr bool valid(Action action) {
    return action==Action::EnterField || action==Action::ReturnHome;
}
constexpr bool valid(Result result) {
    return static_cast<std::uint8_t>(result)<=static_cast<std::uint8_t>(Result::Blocked);
}
inline world_regions::Position destination(Action action) {
    if(action==Action::EnterField) return {world_regions::Id::SunnyPeak,10,10};
    if(action==Action::ReturnHome) return {world_regions::Id::Clubhouse,11,12};
    return {world_regions::Id::Invalid,0,0};
}

// One connection owns this journal. The caller serializes it with world state.
struct Journal {
    std::uint32_t lastId=0;
    Action lastAction=Action::EnterField;
    Result lastResult=Result::InvalidRequest;
    template<class Commit>
    Result request(std::uint32_t id,Action action,world_regions::Position source,Commit commit) {
        if(!id || !valid(action)) return Result::InvalidRequest;
        if(id==lastId) return action==lastAction ? lastResult : Result::InvalidRequest;
        if(id<lastId) return Result::StaleRequest;
        lastId=id; lastAction=action;
        const auto target=destination(action);
        if(!world_regions::valid(source)) lastResult=Result::Blocked;
        else if(source.region==target.region) lastResult=Result::AlreadyThere;
        else lastResult=commit(target) ? Result::Success : Result::Blocked;
        return lastResult;
    }
};
}
