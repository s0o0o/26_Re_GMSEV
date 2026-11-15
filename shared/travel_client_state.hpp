#pragma once
#include "protocol.hpp"
#include <limits>

namespace world_travel {
enum class ClientStatus { Ready, Pending, Arrived, Rejected, TimedOut, Disconnected };
struct ClientState {
    std::uint32_t nextId=1,pendingId=0;
    std::uint64_t deadline=0;
    ClientStatus status=ClientStatus::Ready;
    Result lastResult=Result::Success;
    bool pending() const { return status==ClientStatus::Pending; }
    bool begin(world_regions::Position source,std::uint64_t now,cs_packet_teleport& packet) {
        if(pending() || status==ClientStatus::Disconnected || !world_regions::valid(source) || !nextId) return false;
        packet={sizeof(packet),C2S_P_TELEPORT,source.region==world_regions::Id::Clubhouse ? Action::EnterField : Action::ReturnHome,nextId};
        pendingId=nextId++;
        // Wrap to zero disables further requests on this connection rather than replaying old IDs.
        deadline=now+5000; status=ClientStatus::Pending; return true;
    }
    bool receive(const sc_packet_travel_result& packet) {
        if(!pending() || packet.requestId!=pendingId || !valid(packet.result)) return false;
        lastResult=packet.result;
        status=(lastResult==Result::Success || lastResult==Result::AlreadyThere)
            ? ClientStatus::Arrived : ClientStatus::Rejected;
        pendingId=0; return true;
    }
    void tick(std::uint64_t now) {
        if(pending() && now>=deadline) { status=ClientStatus::TimedOut; pendingId=0; }
    }
    void disconnect() { status=ClientStatus::Disconnected; pendingId=0; }
};
}
