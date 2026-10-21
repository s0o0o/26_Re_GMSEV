#include "../TP_2023192003/TP_2023192003/game_session.hpp"
#include "../TP_2023192003/TP_2023192003/session_registry.hpp"
#include "../TP_2023192003/TP_2023192003/world_map.hpp"
#include <cassert>
#include <cstring>
#include <vector>

// Capture packets without opening a socket or touching the database.
static std::vector<std::vector<char>> sent;
void GAMESESSION::registerSend(void* packet) {
    auto bytes = static_cast<char*>(packet);
    sent.emplace_back(bytes, bytes + static_cast<unsigned char>(bytes[0]));
}

int main() {
    GAMESESSION receiver;
    receiver.sendMovePacket(123);
    receiver.sendAddPlayerPacket(123);
    assert(sent.empty());
    clients[123] = nullptr;
    receiver.sendMovePacket(123);
    receiver.sendAddPlayerPacket(123);
    assert(sent.empty());

    clients[123] = std::make_shared<GAMESESSION>();
    receiver.sendMovePacket(123, 9876);
    sc_packet_move move{};
    std::memcpy(&move, sent.back().data(), sizeof(move));
    assert(move.move_time == 9876);
    receiver.sendMovePacket(123);
    std::memcpy(&move, sent.back().data(), sizeof(move));
    assert(move.move_time == 0);
    sent.clear();

    receiver._viewList.insert(999);
    receiver.sendRemoveSessionPacket(999);
    assert(sent.size() == 1 && receiver._viewList.count(999) == 0);
    receiver.sendAttackPacket_PtoN(999, 42);
    assert(sent.size() == 2);
    receiver.sendAttackPacket_NtoP(999);
    assert(sent.size() == 2);

    sent.clear();
    receiver.sendMonsterNearPacket(nullptr, 1);
    assert(sent.empty());
    receiver.sendMonsterNearPacket(nullptr, 0);
    assert(sent.size() == 1);

    sent.clear();
    map_data[0][0] = 6;
    receiver.sendMapInfo(0, 0, 0);
    assert(sent.size() == 4);
    for (int chunk = 0; chunk < 4; ++chunk) {
        sc_packet_mapInfo packet{};
        assert(sent[chunk].size() == sizeof(packet));
        std::memcpy(&packet, sent[chunk].data(), sizeof(packet));
        assert(packet.type == S2C_P_MAPINFO);
        assert(packet.left_x == -9);
        assert(packet.top_y == -8 + chunk * 4);
        assert(packet.tiles[0][0] == 8);
        if (chunk == 2) assert(packet.tiles[0][9] == 6);
    }
}
