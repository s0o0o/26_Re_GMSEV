#include "../TP_2023192003/TP_2023192003/session_registry.hpp"
#include <cassert>
#include <atomic>
#include <thread>

static std::atomic<int> moves{0},stats{0};
void GAMESESSION::registerSend(void* input) {
    assert(WorldStateLock::heldByCurrentThread());
    const auto bytes=static_cast<const std::uint8_t*>(input);
    if(bytes[1] == S2C_P_MOVE) {
        auto p=static_cast<sc_packet_move*>(input);
        assert(p->id == 1 && p->x == -p->y); ++moves;
    } else if(bytes[1] == S2C_P_STAT_CHANGE) {
        auto p=static_cast<sc_packet_stat_change*>(input);
        assert(p->id == 1 && p->hp+p->seedNum == 100); ++stats;
    } else assert(false);
}
int main() {
    assert(!WorldStateLock::heldByCurrentThread());
    {
        WorldStateLock outer;
        { WorldStateLock inner; assert(WorldStateLock::heldByCurrentThread()); }
        assert(WorldStateLock::heldByCurrentThread());
        outer.unlock(); assert(!WorldStateLock::heldByCurrentThread());
        outer.lock(); assert(WorldStateLock::heldByCurrentThread());
    }
    assert(!WorldStateLock::heldByCurrentThread());
    auto subject=std::make_shared<GAMESESSION>(); subject->object._id=1; subject->object._hp=100;
    clients.set(1,subject);
    std::atomic<bool> start{false};
    std::thread writer([&] {
        while(!start.load()) std::this_thread::yield();
        for(int i=0;i<5000;++i) {
            WorldStateLock lock;
            subject->object._x=static_cast<short>(i); std::this_thread::yield();
            subject->object._y=static_cast<short>(-i);
            subject->object._hp=static_cast<short>(i%101); std::this_thread::yield();
            subject->object._seedNum=static_cast<short>(100-i%101);
        }
    });
    auto read=[&] {
        GAMESESSION receiver;
        while(!start.load()) std::this_thread::yield();
        for(int i=0;i<2000;++i) {
            receiver.sendMovePacket(1); subject->sendStatChange(4);
            auto snapshot=subject->snapshotObject();
            assert(snapshot._x == -snapshot._y && snapshot._hp+snapshot._seedNum == 100);
        }
    };
    std::thread a(read),b(read); start=true;
    writer.join(); a.join(); b.join();
    assert(moves == 4000 && stats == 4000 && !WorldStateLock::heldByCurrentThread());
    clients.clear();
}
