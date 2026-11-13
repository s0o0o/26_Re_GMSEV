#pragma once
#include <mutex>
#include "../../shared/load_trace.hpp"

// Safety-first world transaction gate. Async sends may be submitted inside it;
// DB calls and waits must be outside. Order: own completion -> world -> NPC/view/sector/transport.
inline std::recursive_mutex worldStateMutex;
class WorldStateLock {
    std::unique_lock<std::recursive_mutex> lock_;
    inline static thread_local unsigned depth_=0;
    std::uint64_t began_=0;
    void acquired(std::uint64_t before) {
        if(depth_++==0 && load_trace::enabled()) {
            began_=load_trace::now();
            if(began_-before>=20) load_trace::event("world_wait",-1,0,began_-before);
        }
    }
    void released() {
        if(--depth_==0 && began_ && load_trace::enabled()) {
            const auto held=load_trace::now()-began_;
            if(held>=20) load_trace::event("world_hold",-1,0,held);
        }
        began_=0;
    }
public:
    WorldStateLock() : lock_(worldStateMutex,std::defer_lock) { lock(); }
    explicit WorldStateLock(std::defer_lock_t) : lock_(worldStateMutex,std::defer_lock) {}
    ~WorldStateLock() { if(lock_.owns_lock()) released(); }
    WorldStateLock(const WorldStateLock&)=delete;
    WorldStateLock& operator=(const WorldStateLock&)=delete;
    void lock() { const auto before=load_trace::enabled() ? load_trace::now() : 0; lock_.lock(); acquired(before); }
    void unlock() { released(); lock_.unlock(); }
    static bool heldByCurrentThread() { return depth_ != 0; }
};
