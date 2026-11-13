#include "../TP_2023192003/TP_2023192003/world_state.hpp"
#include <cassert>
#include <sstream>
#include <thread>
#include <vector>
#include <atomic>
#include <cstdio>
int main() {
    const char* path="build/tests/load-trace-fixture.log";
    std::remove(path); // Only this test's generated fixture file, never a user log.
    assert(_putenv_s("GMSEV_TRACE_FILE",path)==0);
    assert(load_trace::enabled());
    std::vector<std::thread> workers;
    for(int i=0;i<8;++i) workers.emplace_back([i] { for(int n=0;n<100;++n) load_trace::event("fixture",i,static_cast<std::uint64_t>(i+1),n); });
    for(auto& worker:workers) worker.join();
    std::atomic_bool started{false};
    WorldStateLock outer; assert(WorldStateLock::heldByCurrentThread());
    { WorldStateLock nested; assert(WorldStateLock::heldByCurrentThread()); }
    std::thread waiter([&] { started=true; WorldStateLock lock; assert(WorldStateLock::heldByCurrentThread()); });
    while(!started) std::this_thread::yield();
    std::this_thread::sleep_for(std::chrono::milliseconds(70));
    outer.unlock(); assert(!WorldStateLock::heldByCurrentThread()); waiter.join();
    outer.lock(); outer.unlock(); assert(!WorldStateLock::heldByCurrentThread());
    std::ifstream input(path); std::string line; int fixtures=0,holds=0,waits=0;
    while(std::getline(input,line)) {
        std::istringstream stream(line); std::string ms,event,session,connection,value,extra;
        assert(stream>>ms>>event>>session>>connection>>value); assert(!(stream>>extra));
        assert(ms.rfind("ms=",0)==0 && session.rfind("session=",0)==0 && connection.rfind("connection=",0)==0 && value.rfind("value=",0)==0);
        if(event=="event=fixture") ++fixtures;
        if(event=="event=world_hold") ++holds;
        if(event=="event=world_wait") ++waits;
    }
    assert(fixtures==800 && holds>=1 && waits>=1);
}
