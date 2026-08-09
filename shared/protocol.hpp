#pragma once

#include <cstddef>
#include <cstdint>
#include <limits>
#include <type_traits>

using ObjectId = std::int32_t;

constexpr std::uint16_t GAME_PORT = 3000;

constexpr std::size_t MAX_CHAT_LENGTH = 30;
constexpr std::size_t MAX_ID_LENGTH = 20;
constexpr std::size_t NAME_SIZE = 20;
constexpr std::size_t BUF_SIZE = 300;

constexpr ObjectId MAX_USER = 10000;

constexpr std::size_t SCREEN_WIDTH = 18;
constexpr std::size_t SCREEN_HEIGHT = 16;
constexpr std::size_t PACKET_HEADER_SIZE = 2;
constexpr std::size_t MAX_NEAR_MONSTERS = 5;

constexpr std::uint8_t S2C_P_AVATAR_INFO = 1;
constexpr std::uint8_t S2C_P_MOVE = 2;
constexpr std::uint8_t S2C_P_ENTER = 3;
constexpr std::uint8_t S2C_P_LEAVE = 4;
constexpr std::uint8_t C2S_P_LOGIN = 5;
constexpr std::uint8_t C2S_P_MOVE = 6;
constexpr std::uint8_t S2C_P_CHAT = 7;
constexpr std::uint8_t S2C_P_STAT_CHANGE = 8;
constexpr std::uint8_t S2C_P_LOGIN_FAIL = 9;
constexpr std::uint8_t C2S_P_ATTACK = 10;
constexpr std::uint8_t C2S_P_CHAT = 11;
constexpr std::uint8_t C2S_P_TELEPORT = 12;
constexpr std::uint8_t S2C_P_AVATAR_LOGIN_OK = 13;
constexpr std::uint8_t S2C_P_ATTACK_PtoN = 14;
constexpr std::uint8_t S2C_P_ATTACK_NtoP = 15;
constexpr std::uint8_t S2C_P_MONSTER_DIE = 16;
constexpr std::uint8_t S2C_P_MONSTER_NEAR = 17;
constexpr std::uint8_t S2C_P_MAPINFO = 18;

constexpr std::uint8_t MOVE_UP = 1;
constexpr std::uint8_t MOVE_DOWN = 2;
constexpr std::uint8_t MOVE_LEFT = 3;
constexpr std::uint8_t MOVE_RIGHT = 4;

#pragma pack(push, 1)

struct sc_packet_avatar_info {
	std::uint8_t size;
	std::uint8_t type;
	ObjectId id;
	std::int16_t x, y;
	std::int16_t max_hp;
	std::int16_t hp;
	std::int16_t level;
	std::int32_t exp;
	std::int32_t seedNum;
};

struct sc_packet_avatar_login_ok {
	std::uint8_t size;
	std::uint8_t type;
	ObjectId id;
};

struct sc_packet_move {
	std::uint8_t size;
	std::uint8_t type;
	ObjectId id;
	std::int16_t x, y;
	std::uint32_t move_time;
};

struct sc_packet_enter {
	std::uint8_t size;
	std::uint8_t type;
	ObjectId id;
	char name[MAX_ID_LENGTH];
	std::int32_t monType;
	std::int16_t x, y;
};

struct sc_packet_leave {
	std::uint8_t size;
	std::uint8_t type;
	ObjectId id;
};

struct sc_packet_chat {
	std::uint8_t size;
	std::uint8_t type;
	ObjectId id;
	char message[MAX_CHAT_LENGTH];
};

struct sc_packet_stat_change {
	std::uint8_t size;
	std::uint8_t type;
	ObjectId id;
	std::int16_t hp;
	std::int16_t level;
	std::int32_t exp;
	std::int32_t seedNum;
	std::int32_t mapType;
};

struct sc_packet_login_fail {
	std::uint8_t size;
	std::uint8_t type;
	ObjectId id;
	std::uint8_t reason;
};

struct sc_packet_attack_success {
	std::uint8_t size;
	std::uint8_t type;
	ObjectId targetId;
	ObjectId attackerId;
	std::int16_t targetHp;
	std::int32_t seedNum;
};

struct sc_packet_monster_die {
	std::uint8_t size;
	std::uint8_t type;
	ObjectId id;
};

struct nearMonsterInfo {
	ObjectId monId;
	std::int32_t monHp;
	std::int32_t monType;
};

struct sc_packet_monster_near {
	std::uint8_t size;
	std::uint8_t type;
	std::int32_t monCount;
	std::int32_t monType;
	nearMonsterInfo monsters[MAX_NEAR_MONSTERS];
};

struct sc_packet_mapInfo {
	std::uint8_t size;
	std::uint8_t type;
	std::int16_t left_x, top_y;
	std::uint8_t tiles[4][SCREEN_WIDTH];
};

struct cs_packet_login {
	std::uint8_t size;
	std::uint8_t type;
	char name[MAX_ID_LENGTH];
	std::int32_t inputId;
};

struct cs_packet_move {
	std::uint8_t size;
	std::uint8_t type;
	std::uint8_t direction;
	std::uint32_t move_time;
};

struct cs_packet_attack {
	std::uint8_t size;
	std::uint8_t type;
	ObjectId id;
	std::int32_t x, y;
	std::int32_t hp;
	std::int32_t seedNum;
	std::int32_t damage;
};

struct cs_packet_chat {
	std::uint8_t size;
	std::uint8_t type;
	ObjectId id;
	char message[MAX_CHAT_LENGTH];
};

struct cs_packet_teleport {
	std::uint8_t size;
	std::uint8_t type;
};

#pragma pack(pop)

template <typename Packet>
constexpr bool valid_packet_layout()
{
	return std::is_trivially_copyable<Packet>::value &&
		sizeof(Packet) <= (std::numeric_limits<std::uint8_t>::max)();
}

static_assert(valid_packet_layout<sc_packet_avatar_info>(), "invalid packet layout");
static_assert(valid_packet_layout<sc_packet_avatar_login_ok>(), "invalid packet layout");
static_assert(valid_packet_layout<sc_packet_move>(), "invalid packet layout");
static_assert(valid_packet_layout<sc_packet_enter>(), "invalid packet layout");
static_assert(valid_packet_layout<sc_packet_leave>(), "invalid packet layout");
static_assert(valid_packet_layout<sc_packet_chat>(), "invalid packet layout");
static_assert(valid_packet_layout<sc_packet_stat_change>(), "invalid packet layout");
static_assert(valid_packet_layout<sc_packet_login_fail>(), "invalid packet layout");
static_assert(valid_packet_layout<sc_packet_attack_success>(), "invalid packet layout");
static_assert(valid_packet_layout<sc_packet_monster_die>(), "invalid packet layout");
static_assert(valid_packet_layout<sc_packet_monster_near>(), "invalid packet layout");
static_assert(valid_packet_layout<sc_packet_mapInfo>(), "invalid packet layout");
static_assert(valid_packet_layout<cs_packet_login>(), "invalid packet layout");
static_assert(valid_packet_layout<cs_packet_move>(), "invalid packet layout");
static_assert(valid_packet_layout<cs_packet_attack>(), "invalid packet layout");
static_assert(valid_packet_layout<cs_packet_chat>(), "invalid packet layout");
static_assert(valid_packet_layout<cs_packet_teleport>(), "invalid packet layout");

inline bool is_valid_client_packet(std::uint8_t type, std::size_t size) noexcept
{
	switch (type) {
	case C2S_P_LOGIN: return size == sizeof(cs_packet_login);
	case C2S_P_MOVE: return size == sizeof(cs_packet_move);
	case C2S_P_ATTACK: return size == sizeof(cs_packet_attack);
	case C2S_P_CHAT: return size == sizeof(cs_packet_chat);
	case C2S_P_TELEPORT: return size == sizeof(cs_packet_teleport);
	default: return false;
	}
}

inline bool is_valid_server_packet(const std::uint8_t* packet, std::size_t size) noexcept
{
	if (packet == nullptr || size < PACKET_HEADER_SIZE || packet[0] != size) {
		return false;
	}

	switch (packet[1]) {
	case S2C_P_AVATAR_INFO: return size == sizeof(sc_packet_avatar_info);
	case S2C_P_MOVE: return size == sizeof(sc_packet_move);
	case S2C_P_ENTER: return size == sizeof(sc_packet_enter);
	case S2C_P_LEAVE: return size == sizeof(sc_packet_leave);
	case S2C_P_CHAT: return size == sizeof(sc_packet_chat);
	case S2C_P_STAT_CHANGE: return size == sizeof(sc_packet_stat_change);
	case S2C_P_LOGIN_FAIL: return size == sizeof(sc_packet_login_fail);
	case S2C_P_AVATAR_LOGIN_OK: return size == sizeof(sc_packet_avatar_login_ok);
	case S2C_P_ATTACK_PtoN: return size == sizeof(sc_packet_attack_success);
	case S2C_P_MONSTER_DIE: return size == sizeof(sc_packet_monster_die);
	case S2C_P_MAPINFO: return size == sizeof(sc_packet_mapInfo);
	case S2C_P_MONSTER_NEAR: {
		const std::size_t base_size = offsetof(sc_packet_monster_near, monsters);
		if (size < base_size) {
			return false;
		}
		const auto* near_packet = reinterpret_cast<const sc_packet_monster_near*>(packet);
		if (near_packet->monCount < 0 ||
			near_packet->monCount > static_cast<std::int32_t>(MAX_NEAR_MONSTERS)) {
			return false;
		}
		return size == base_size + sizeof(nearMonsterInfo) * near_packet->monCount;
	}
	default: return false;
	}
}
