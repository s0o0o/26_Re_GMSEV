#pragma once
#include "protocol.hpp"
#include <array>
#include <cstring>

// Temporary rendering/GUI-stress boundary. Never modify the received wire frame.
class LegacyPacketAdapter {
    std::array<std::uint8_t,BUF_SIZE> copy_{};
public:
    std::uint8_t* decode(const void* input,std::size_t size) {
        if(size>copy_.size() || !is_valid_server_packet(static_cast<const std::uint8_t*>(input),size)) return nullptr;
        std::memcpy(copy_.data(),input,size);
        switch(copy_[1]) {
        case S2C_P_AVATAR_INFO: packet_position::restoreLegacy(*reinterpret_cast<sc_packet_avatar_info*>(copy_.data())); break;
        case S2C_P_MOVE: packet_position::restoreLegacy(*reinterpret_cast<sc_packet_move*>(copy_.data())); break;
        case S2C_P_ENTER: packet_position::restoreLegacy(*reinterpret_cast<sc_packet_enter*>(copy_.data())); break;
        case S2C_P_MAPINFO: {
            auto& map=*reinterpret_cast<sc_packet_mapInfo*>(copy_.data());
            map.left_x=static_cast<std::int16_t>(map.left_x+world_regions::describe(static_cast<world_regions::Id>(map.region)).originX); break;
        }
        default: break;
        }
        return copy_.data();
    }
};
