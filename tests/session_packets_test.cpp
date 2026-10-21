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
    clients[123]->object._furColor = 3;
    clients[123]->object._eyeStyle = 1;
    clients[123]->object._scarfStyle = 2;
    clients[123]->object._characterKind = 1;
    receiver.sendAddPlayerPacket(123);
    sc_packet_enter entered{};
    std::memcpy(&entered, sent.back().data(), sizeof(entered));
    assert(entered.fur_color == 3);
    assert(entered.eye_style == 1 && entered.scarf_style == 2);
    assert(entered.character_kind == 1);
    receiver.object._furColor = 4;
    receiver.object._eyeStyle = 2;
    receiver.object._scarfStyle = 1;
    receiver.object._characterKind = 1;
    receiver.sendSessionLoginPacket();
    sc_packet_avatar_info avatar{};
    std::memcpy(&avatar, sent.back().data(), sizeof(avatar));
    assert(avatar.fur_color == 4);
    assert(avatar.eye_style == 2 && avatar.scarf_style == 1);
    assert(avatar.character_kind == 1);
    assert(validCharacterKind(0) && validCharacterKind(1) && !validCharacterKind(2) && !validCharacterKind(255));
    assert(validEyeStyle(2) && !validEyeStyle(3) && !validEyeStyle(255));
    assert(validScarfStyle(2) && !validScarfStyle(3) && !validScarfStyle(255));
    assert(validFurColor(0) && validFurColor(4) && !validFurColor(5) && !validFurColor(255));
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
    receiver.sendCombatUpdate(23,7,MAX_USER,2);
    sc_packet_attack_success combat{};
    std::memcpy(&combat,sent.back().data(),sizeof(combat));
    assert(combat.attackerId == 23 && combat.seedNum == 7 && combat.targetId == MAX_USER && combat.targetHp == 2);
    receiver.sendAttackFeedback(AttackFeedback::NoSeeds);
    assert(is_valid_server_packet(reinterpret_cast<const std::uint8_t*>(sent.back().data()),sent.back().size()));
    sent.back()[2]=static_cast<char>(255);
    assert(!is_valid_server_packet(reinterpret_cast<const std::uint8_t*>(sent.back().data()),sent.back().size()));
    clients[MAX_USER]=std::make_shared<GAMESESSION>();
    clients[MAX_USER]->object._hp=2; clients[MAX_USER]->object._maxHp=5;
    receiver.sendAddPlayerPacket(MAX_USER);
    std::memcpy(&entered,sent.back().data(),sizeof(entered));
    assert(entered.hp == 2 && entered.max_hp == 5);
    const auto beforeDead=sent.size();
    clients[MAX_USER]->monster._isDie=true;
    receiver.sendAddPlayerPacket(MAX_USER);
    assert(sent.size() == beforeDead);

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
        assert(packet.left_x == 0);
        assert(packet.top_y == chunk * 4);
        if (chunk == 0) assert(packet.tiles[0][0] == 6);
    }
}
