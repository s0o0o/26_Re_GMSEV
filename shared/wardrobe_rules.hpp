#pragma once
#include "appearance.hpp"
#include "world_regions.hpp"

namespace wardrobe {
enum class Result : std::uint8_t {
    Saved=0, InvalidRequest=1, NotInClubhouse=2, StaleRequest=3,
    StorageUnavailable=4, StorageFailure=5
};
constexpr bool valid(Result result) { return static_cast<std::uint8_t>(result)<=5; }
struct Journal {
    std::uint32_t lastId=0;
    avatar_appearance::Value lastValue;
    Result lastResult=Result::InvalidRequest;
    template<class SaveAndApply>
    Result request(std::uint32_t id,avatar_appearance::Value value,world_regions::Position position,SaveAndApply commit) {
        if(!id || !avatar_appearance::valid(value)) return Result::InvalidRequest;
        if(id==lastId) return value==lastValue ? lastResult : Result::InvalidRequest;
        if(id<lastId) return Result::StaleRequest;
        lastId=id; lastValue=value;
        if(!world_regions::valid(position) || position.region!=world_regions::Id::Clubhouse)
            return lastResult=Result::NotInClubhouse;
        return lastResult=commit(value);
    }
};
}
