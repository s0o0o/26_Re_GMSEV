#include "../shared/protocol.hpp"
#include "../shared/world_layout.hpp"
#include "../shared/travel_client_state.hpp"
#include "../shared/packet_outbox.hpp"
#include <cassert>
#include <cstdio>
int main() {
    using namespace world_travel;
    using namespace world_regions;
    Journal journal;
    Position position{Id::Clubhouse,11,12};
    int commits=0;
    bool blocked=false;
    auto commit=[&](Position target) {
        ++commits; if(blocked) return false;
        assert(world_layout::walkable(target)); position=target; return true;
    };
    assert(journal.request(0,Action::EnterField,position,commit)==Result::InvalidRequest);
    assert(journal.request(1,static_cast<Action>(0),position,commit)==Result::InvalidRequest);
    assert(commits==0);
    assert(journal.request(1,Action::EnterField,position,commit)==Result::Success);
    assert(position.region==Id::SunnyPeak && position.x==10 && position.y==10 && commits==1);
    assert(journal.request(1,Action::EnterField,position,commit)==Result::Success && commits==1);
    assert(journal.request(1,Action::ReturnHome,position,commit)==Result::InvalidRequest);
    assert(journal.request(2,Action::EnterField,position,commit)==Result::AlreadyThere && commits==1);
    assert(journal.request(1,Action::EnterField,position,commit)==Result::StaleRequest);
    blocked=true;
    assert(journal.request(3,Action::ReturnHome,position,commit)==Result::Blocked && commits==2);
    blocked=false;
    assert(journal.request(3,Action::ReturnHome,position,commit)==Result::Blocked && commits==2);
    assert(journal.request(4,Action::ReturnHome,position,commit)==Result::Success && commits==3);
    assert(position.region==Id::Clubhouse && position.x==11 && position.y==12);
    assert(journal.request(5,Action::EnterField,{Id::Invalid,0,0},commit)==Result::Blocked && commits==3);
    assert(sizeof(cs_packet_teleport)==7 && sizeof(sc_packet_travel_result)==7);
    assert(!is_valid_client_packet(C2S_P_TELEPORT,2));
    sc_packet_travel_result result{sizeof(result),S2C_P_TRAVEL_RESULT,4,Result::Success};
    assert(is_valid_server_packet(reinterpret_cast<const std::uint8_t*>(&result),sizeof(result)));
    result.result=static_cast<Result>(255);
    assert(!is_valid_server_packet(reinterpret_cast<const std::uint8_t*>(&result),sizeof(result)));
    std::puts("travel rules: sequence/replay/stale/blocked/position/wire PASS");
    ClientState client;
    cs_packet_teleport request{};
    assert(client.begin(position,100,request) && request.action==Action::EnterField && request.requestId==1);
    assert(!client.begin(position,101,request));
    sc_packet_travel_result response{sizeof(response),S2C_P_TRAVEL_RESULT,2,Result::Success};
    assert(!client.receive(response) && client.pending());
    client.tick(5099); assert(client.pending());
    client.tick(5100); assert(client.status==ClientStatus::TimedOut);
    assert(!client.receive(response));
    assert(client.begin(position,5200,request) && request.requestId==2);
    response.result=Result::Blocked;
    assert(client.receive(response) && client.status==ClientStatus::Rejected);
    assert(client.begin(position,5300,request));
    response.requestId=3; response.result=Result::AlreadyThere;
    assert(client.receive(response) && client.status==ClientStatus::Arrived);
    client.disconnect(); assert(!client.begin(position,5400,request));
    client=ClientState{}; client.nextId=0xffffffffu;
    assert(client.begin(position,5500,request));
    client.tick(10500); assert(!client.begin(position,10501,request));
    PacketOutbox outbox;
    std::vector<std::uint8_t> expected,actual;
    assert(outbox.enqueue(&request,sizeof(request)));
    assert(outbox.enqueue(&response,sizeof(response)));
    auto append=[&](const void* p,std::size_t size) {
        const auto* b=static_cast<const std::uint8_t*>(p); expected.insert(expected.end(),b,b+size);
    };
    append(&request,sizeof(request)); append(&response,sizeof(response));
    int calls=0;
    while(outbox.queued()) assert(outbox.pump([&](const std::uint8_t* data,std::size_t size,std::size_t& sent) {
        if(++calls%3==0) return PacketOutbox::Send::Wait;
        sent=(std::min)(std::size_t{2},size); actual.insert(actual.end(),data,data+sent);
        return PacketOutbox::Send::Progress;
    }));
    assert(actual==expected);
    assert(outbox.enqueue(&request,sizeof(request)));
    assert(!outbox.pump([](const auto*,auto,auto&) { return PacketOutbox::Send::Failed; }));
    assert(outbox.queued()==0);
    std::puts("travel client/outbox: pending/id/timeout/retry/disconnect/wrap/partial/order PASS");
}
