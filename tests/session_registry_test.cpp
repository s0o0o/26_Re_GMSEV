#include "../TP_2023192003/TP_2023192003/session_registry.hpp"
#include <cassert>
#include <condition_variable>
#include <thread>
#include <set>

struct StartGate {
    std::mutex mutex; std::condition_variable changed; int ready=0; bool start=false;
    void await() { std::unique_lock<std::mutex> lock(mutex); ++ready; changed.notify_all(); changed.wait(lock,[&] { return start; }); }
    void release(int count) { std::unique_lock<std::mutex> lock(mutex); changed.wait(lock,[&] { return ready == count; }); start=true; changed.notify_all(); }
};
int main() {
    SessionRegistry registry;
    assert(!registry.get(123) && registry.snapshot().empty());
    std::weak_ptr<GAMESESSION> weak;
    {
        auto old=std::make_shared<GAMESESSION>(); old->object._id=123; weak=old;
        registry.set(123,old); auto copy=registry.get(123); auto list=registry.snapshot();
        registry.set(123,nullptr); old.reset(); assert(!weak.expired());
        registry.clear(); assert(copy->object._id == 123 && list[0].second == copy);
    }
    assert(weak.expired());
    // Replace and clear while readers copy pointers and full snapshots.
    StartGate gate;
    std::mutex weakMutex; std::vector<std::weak_ptr<GAMESESSION>> retired;
    std::vector<std::thread> threads;
    for(int worker=0;worker<4;++worker) threads.emplace_back([&,worker] {
        gate.await();
        for(int i=0;i<1000;++i) {
            const int id=(i+worker)%32;
            auto s=std::make_shared<GAMESESSION>(); s->object._id=id; s->object._hp=77;
            { std::lock_guard<std::mutex> lock(weakMutex); retired.push_back(s); }
            registry.set(id,s);
        }
    });
    for(int worker=0;worker<4;++worker) threads.emplace_back([&,worker] {
        gate.await();
        for(int i=0;i<1000;++i) {
            auto s=registry.get((i+worker)%32); if(s) assert(s->object._hp == 77);
            for(const auto& entry : registry.snapshot()) {
                assert(entry.second && entry.first == entry.second->object._id && entry.second->object._hp == 77);
            }
        }
    });
    threads.emplace_back([&] { gate.await(); for(int i=0;i<1000;++i) registry.clear(); });
    gate.release(9); for(auto& thread : threads) thread.join(); threads.clear();
    registry.clear(); for(const auto& s : retired) assert(s.expired());
    // Select slot and publish ST_ALLOC atomically under competing allocators.
    StartGate allocateGate;
    std::mutex resultMutex; std::set<int> ids; std::vector<std::shared_ptr<GAMESESSION>> allocated;
    for(int worker=0;worker<8;++worker) threads.emplace_back([&] {
        allocateGate.await();
        for(int i=0;i<32;++i) {
            auto s=std::make_shared<GAMESESSION>(); const int id=registry.allocatePlayer(s);
            assert(id >= 0 && s->object._id == id && s->_state == ST_ALLOC);
            std::lock_guard<std::mutex> lock(resultMutex); assert(ids.insert(id).second); allocated.push_back(s);
        }
    });
    allocateGate.release(8); for(auto& thread : threads) thread.join();
    assert(ids.size() == 256 && registry.allocatePlayer(nullptr) == -1);
    assert(registry.allocatePlayer(allocated[0]) == -1);
    auto old=registry.get(0); const auto oldConnection=old->connectionId; old->_state=ST_FREE;
    auto replacement=std::make_shared<GAMESESSION>();
    assert(registry.allocatePlayer(replacement) == 0 && replacement->connectionId != oldConnection);
    assert(old->connectionId == oldConnection && registry.get(0) == replacement);
    registry.clear(); allocated.clear(); old.reset(); replacement.reset();
    for(int id=0;id<MAX_USER;++id) {
        auto s=std::make_shared<GAMESESSION>(); s->object._id=id; s->_state=ST_ALLOC; registry.set(id,std::move(s));
    }
    auto full=std::make_shared<GAMESESSION>();
    assert(registry.allocatePlayer(full) == -1 && full->object._id == -1 && full->_state == ST_FREE);
    registry.clear();
}
