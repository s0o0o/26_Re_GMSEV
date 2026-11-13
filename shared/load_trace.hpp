#pragma once
#include <chrono>
#include <cstdlib>
#include <fstream>
#include <mutex>
#include <cstdint>
// Opt-in, dedicated diagnostic stream. Never log account/name/DSN/packet contents.
namespace load_trace {
inline std::uint64_t now() {
    return static_cast<std::uint64_t>(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count());
}
struct Writer {
    std::mutex mutex; std::ofstream stream;
    Writer() {
        char* path=nullptr; std::size_t size=0;
#ifdef _WIN32
        if(_dupenv_s(&path,&size,"GMSEV_TRACE_FILE")==0 && path && *path) stream.open(path,std::ios::app);
        std::free(path);
#else
        const char* configured=std::getenv("GMSEV_TRACE_FILE");
        if(configured && *configured) stream.open(configured,std::ios::app);
#endif
    }
};
inline Writer& writer() { static Writer instance; return instance; }
inline bool enabled() { return writer().stream.is_open(); }
inline void event(const char* name,int session,std::uint64_t connection,std::uint64_t value=0) {
    auto& sink=writer(); if(!sink.stream.is_open()) return;
    std::lock_guard<std::mutex> lock(sink.mutex);
    sink.stream<<"ms="<<now()<<" event="<<name<<" session="<<session<<" connection="<<connection<<" value="<<value<<'\n';
    sink.stream.flush();
}
}
