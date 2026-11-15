#pragma once
#include <cstddef>
#include <cstdint>
#include <deque>
#include <vector>

// A nonblocking transport may accept only a prefix. Never interleave packets.
class PacketOutbox {
public:
    enum class Send { Progress, Wait, Failed };
    bool enqueue(const void* data,std::size_t size) {
        if(!data || size<2 || size>255 || queued_+size>65536) return false;
        const auto* bytes=static_cast<const std::uint8_t*>(data);
        if(bytes[0]!=size) return false;
        packets_.emplace_back(bytes,bytes+size); queued_+=size; return true;
    }
    template<class Sender> bool pump(Sender sender) {
        for(int budget=0;budget<16 && !packets_.empty();++budget) {
            auto& packet=packets_.front();
            const auto remaining=packet.size()-offset_;
            std::size_t sent=0;
            const auto status=sender(packet.data()+offset_,remaining,sent);
            if(sent>remaining || status==Send::Failed) { clear(); return false; }
            offset_+=sent; queued_-=sent;
            if(offset_==packet.size()) { packets_.pop_front(); offset_=0; }
            if(status==Send::Wait || sent==0) break;
        }
        return true;
    }
    void clear() { packets_.clear(); offset_=queued_=0; }
    std::size_t queued() const { return queued_; }
private:
    std::deque<std::vector<std::uint8_t>> packets_;
    std::size_t offset_=0,queued_=0;
};
