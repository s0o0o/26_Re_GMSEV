#pragma once
#include "protocol.hpp"

namespace wardrobe {
enum class Stage { Closed, Editing, Pending, Applied, Rejected, TimedOut, Disconnected };
struct ClientState {
    Stage stage=Stage::Closed;
    avatar_appearance::Value baseline,draft;
    Result lastResult=Result::Saved;
    std::uint32_t nextId=1,pendingId=0;
    std::uint64_t deadline=0;
    bool connected=true;
    bool visible() const { return stage!=Stage::Closed; }
    bool pending() const { return stage==Stage::Pending; }
    bool open(world_regions::Position position,avatar_appearance::Value current) {
        if(!connected || visible() || !world_regions::valid(position)
            || position.region!=world_regions::Id::Clubhouse || !avatar_appearance::valid(current)) return false;
        baseline=draft=current; stage=Stage::Editing; return true;
    }
    bool edit(avatar_appearance::Value value) {
        if(!connected || !visible() || pending() || !avatar_appearance::valid(value)) return false;
        draft=value; stage=Stage::Editing; return true;
    }
    bool close() {
        if(pending()) return false; // A submitted durable save cannot be cancelled locally.
        draft=baseline; stage=Stage::Closed; return true;
    }
    bool submit(std::uint64_t now,cs_packet_appearance& packet) {
        if(!connected || !visible() || pending() || !nextId || !avatar_appearance::valid(draft)) return false;
        packet={sizeof(packet),C2S_P_APPEARANCE,nextId++,draft};
        pendingId=packet.requestId; deadline=now+5000; stage=Stage::Pending; return true;
    }
    bool receive(const sc_packet_appearance_result& packet) {
        if(!pending() || packet.requestId!=pendingId || !valid(packet.result)
            || !avatar_appearance::valid(packet.appearance)) return false;
        baseline=packet.appearance; lastResult=packet.result; pendingId=0;
        if(packet.result==Result::Saved) { draft=baseline; stage=Stage::Applied; }
        else stage=Stage::Rejected;
        return true;
    }
    void tick(std::uint64_t now) {
        if(pending() && now>=deadline) { pendingId=0; stage=Stage::TimedOut; }
    }
    void regionChanged(world_regions::Position position) {
        if(position.region!=world_regions::Id::Clubhouse || !world_regions::valid(position)) {
            pendingId=0; draft=baseline; stage=Stage::Closed;
        }
    }
    void disconnect() { connected=false; pendingId=0; if(visible()) stage=Stage::Disconnected; }
};
}
