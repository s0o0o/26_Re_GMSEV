#pragma once
#include "field_landmarks.hpp"
namespace field_discovery {
class Record {
    std::uint8_t visited_=0;
    bool active_=false;
public:
    void start() { active_=true; } // Repeated avatar information must not erase this connection's progress.
    void reset() { visited_=0;active_=false; }
    bool active() const { return active_; }
    bool visited(int id) const { return id>=1 && id<=5 && (visited_ & (1u<<(id-1)))!=0; }
    unsigned count() const { unsigned count=0;for(int id=1;id<=5;++id) if(visited(id))++count;return count; }
    std::uint8_t observe(world_regions::Position confirmed) {
        if(!active_ || confirmed.region!=world_regions::Id::SunnyPeak || !world_regions::valid(confirmed)
            || !field_terrain::walkable(confirmed.x,confirmed.y)) return 0;
        const auto before=visited_;
        for(const auto& p:field_landmarks::all) if(confirmed.x>=p.x-2 && confirmed.x<=p.x+2 && confirmed.y>=p.y-2 && confirmed.y<=p.y+2)
            visited_=static_cast<std::uint8_t>(visited_ | (1u<<(p.id-1)));
        return static_cast<std::uint8_t>(visited_ ^ before);
    }
};
}
