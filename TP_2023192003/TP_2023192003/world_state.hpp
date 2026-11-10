#pragma once
#include <mutex>

// Safety-first world transaction gate. Async sends may be submitted inside it;
// DB calls and waits must be outside. Order: own completion -> world -> NPC/view/sector/transport.
inline std::recursive_mutex worldStateMutex;
class WorldStateLock {
    std::unique_lock<std::recursive_mutex> lock_;
    inline static thread_local unsigned depth_=0;
public:
    WorldStateLock() : lock_(worldStateMutex) { ++depth_; }
    explicit WorldStateLock(std::defer_lock_t) : lock_(worldStateMutex,std::defer_lock) {}
    ~WorldStateLock() { if(lock_.owns_lock()) --depth_; }
    WorldStateLock(const WorldStateLock&)=delete;
    WorldStateLock& operator=(const WorldStateLock&)=delete;
    void lock() { lock_.lock(); ++depth_; }
    void unlock() { lock_.unlock(); --depth_; }
    static bool heldByCurrentThread() { return depth_ != 0; }
};
