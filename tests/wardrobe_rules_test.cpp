#include "../shared/wardrobe_client_state.hpp"
#include <cassert>
#include <cstdio>
#include <climits>
int main() {
    using namespace wardrobe;
    using avatar_appearance::Value;
    const world_regions::Position home{world_regions::Id::Clubhouse,11,12},field{world_regions::Id::SunnyPeak,10,10};
    const Value original{},changed{1,4,2,2};
    assert(avatar_appearance::valid(changed));
    Journal journal; int writes=0;
    auto save=[&](Value value) { assert(value==changed); ++writes; return Result::Saved; };
    assert(journal.request(0,changed,home,save)==Result::InvalidRequest);
    assert(journal.request(1,{2,0,0,0},home,save)==Result::InvalidRequest);
    assert(journal.request(1,changed,field,save)==Result::NotInClubhouse && writes==0);
    assert(journal.request(1,changed,home,save)==Result::NotInClubhouse && writes==0);
    assert(journal.request(2,changed,home,save)==Result::Saved && writes==1);
    assert(journal.request(2,changed,home,save)==Result::Saved && writes==1);
    assert(journal.request(2,original,home,save)==Result::InvalidRequest && writes==1);
    assert(journal.request(1,changed,home,save)==Result::StaleRequest && writes==1);
    assert(journal.request(3,changed,home,[](Value){return Result::StorageFailure;})==Result::StorageFailure);
    assert(journal.request(4,changed,home,[](Value){return Result::StorageUnavailable;})==Result::StorageUnavailable);
    ClientState client; cs_packet_appearance request{};
    assert(!client.open(field,original) && client.open(home,original));
    assert(client.edit(changed) && client.baseline==original && client.draft==changed);
    assert(client.close() && client.draft==original); // Cancel before sending leaves confirmed state alone.
    assert(client.open(home,original) && client.edit(changed));
    assert(client.submit(100,request) && request.requestId==1 && request.appearance==changed);
    assert(!client.close() && !client.edit(original) && !client.submit(101,request));
    sc_packet_appearance_result reply{sizeof(reply),S2C_P_APPEARANCE_RESULT,2,Result::Saved,changed};
    assert(!client.receive(reply) && client.baseline==original);
    client.tick(5099); assert(client.pending()); client.tick(5100); assert(client.stage==Stage::TimedOut);
    reply.requestId=1; assert(!client.receive(reply));
    assert(client.submit(5200,request) && request.requestId==2);
    reply.requestId=2; reply.result=Result::StorageFailure; reply.appearance=original;
    assert(client.receive(reply) && client.stage==Stage::Rejected && client.baseline==original && client.draft==changed);
    assert(client.submit(5300,request) && request.requestId==3);
    reply.requestId=3; reply.result=Result::Saved; reply.appearance=changed;
    assert(client.receive(reply) && client.stage==Stage::Applied && client.baseline==changed);
    assert(client.close() && client.open(home,changed)); client.regionChanged(field); assert(!client.visible());
    assert(client.open(home,changed)); client.disconnect(); assert(!client.submit(5400,request)); client.close();
    assert(!client.open(home,changed));
    client={}; assert(client.open(home,original)); client.nextId=0xffffffffu;
    assert(client.submit(100,request)); client.tick(5100); assert(!client.submit(5200,request));
    assert(sizeof(Value)==4 && sizeof(cs_packet_appearance)==10 && sizeof(sc_packet_appearance_result)==11 && sizeof(sc_packet_appearance_change)==11);
    sc_packet_appearance_change change{sizeof(change),S2C_P_APPEARANCE_CHANGE,1,1,changed};
    assert(is_valid_server_packet(reinterpret_cast<const std::uint8_t*>(&change),sizeof(change)));
    change.id=MAX_USER; assert(!is_valid_server_packet(reinterpret_cast<const std::uint8_t*>(&change),sizeof(change)));
    change.id=1; change.appearance.eyes=255;
    assert(!is_valid_server_packet(reinterpret_cast<const std::uint8_t*>(&change),sizeof(change)));
    std::puts("wardrobe: range/journal/replay/storage error/draft/cancel/pending/id/timeout/region/disconnect/wire PASS");
}
