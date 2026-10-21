#include "game_session.hpp"
#include "session_registry.hpp"
#include "world_map.hpp"
#include <algorithm>
#include <cstdio>
#include <cstring>

void GAMESESSION::sendSessionLoginPacket() {
	sc_packet_avatar_info p;
	p.id = object._id;
	p.size = sizeof(sc_packet_avatar_info);
	p.type = S2C_P_AVATAR_INFO;
	p.x = object._x;
	p.y = object._y;
	p.hp = object._hp;
	p.level = object._level;
	p.exp = object._exp;
	p.max_hp = object._maxHp;
	p.seedNum = object._seedNum;
	p.fur_color = object._furColor;
	p.eye_style = object._eyeStyle;
	p.scarf_style = object._scarfStyle;
	registerSend(&p);
}

void GAMESESSION::sendSessioLoginOkPacket() {
	sc_packet_avatar_login_ok p;
	p.id = object._id;
	p.size = sizeof(sc_packet_avatar_login_ok);
	p.type = S2C_P_AVATAR_LOGIN_OK;
	registerSend(&p);
}

void GAMESESSION::sendMovePacket(int sessionId, std::uint32_t moveTime) {
	auto it = clients.find(sessionId);
	if (it == clients.end() || !it->second) return;
	std::shared_ptr<GAMESESSION> user = it->second;

	sc_packet_move p{};
	p.move_time = moveTime;
	p.id = sessionId;
	p.size = sizeof(sc_packet_move);
	p.type = S2C_P_MOVE;
	p.x = user->object._x;
	p.y = user->object._y;
	registerSend(&p);
}

void GAMESESSION::sendAddPlayerPacket(int sessionId) {
	auto it = clients.find(sessionId);
	if (it == clients.end() || !it->second) return;
	std::shared_ptr<GAMESESSION> user = it->second;

	sc_packet_enter p;
	p.id = sessionId;
	sprintf_s(p.name, NAME_SIZE, "%s", user->object._name);

	p.size = sizeof(p);
	p.type = S2C_P_ENTER;
	p.monType = user->monster._monType;
	p.fur_color = user->object._furColor;
	p.eye_style = user->object._eyeStyle;
	p.scarf_style = user->object._scarfStyle;
	p.x = user->object._x;
	p.y = user->object._y;
	_vlL.lock();
	_viewList.insert(sessionId);
	_vlL.unlock();
	registerSend(&p);
}

void GAMESESSION::sendRemoveSessionPacket(int sessionId) {
	_vlL.lock();
	if (_viewList.count(sessionId))
		_viewList.erase(sessionId);
	else {
		_vlL.unlock();
		return;
	}
	_vlL.unlock();

	sc_packet_leave p;
	p.id = sessionId;
	p.size = sizeof(p);
	p.type = S2C_P_LEAVE;
	registerSend(&p);
}

void GAMESESSION::sendChatPacket(int sessionId, const char* message) {
	sc_packet_chat p;
	p.id = sessionId;
	p.size = sizeof(p);
	p.type = S2C_P_CHAT;
	//strcpy_s(p.message, message);

	strncpy_s(p.message, message, MAX_CHAT_LENGTH - 1);
	p.message[MAX_CHAT_LENGTH - 1] = '\0';
	registerSend(&p);
}

void GAMESESSION::sendAttackPacket_PtoN(int targetId, int targetNewHp) {
	sc_packet_attack_success p;
	p.size = sizeof(p);
	p.type = S2C_P_ATTACK_PtoN;

	p.targetId = targetId;
	p.targetHp = targetNewHp;

	p.attackerId = object._id;
	p.seedNum = object._seedNum;
	registerSend(&p);
}

void GAMESESSION::sendAttackPacket_NtoP(int /*sessionId*/) {
	// Reserved: this notification is not implemented yet.
}

void GAMESESSION::sendMonsterDiePacket(int targetId) {
	sc_packet_monster_die p;
	p.size = sizeof(p);
	p.type = S2C_P_MONSTER_DIE;
	p.id = targetId;
	registerSend(&p);
}

void GAMESESSION::sendMonsterNearPacket(nearMonsterInfo* infos, int cnt) {
	cnt = std::clamp(cnt, 0, static_cast<int>(MAX_NEAR_MONSTERS));
	sc_packet_monster_near p{};
	p.type = S2C_P_MONSTER_NEAR;
	p.monCount = cnt;
	p.monType = monster._monType;
	if (cnt > 0) {
		if (infos == nullptr) return;
		memcpy(p.monsters, infos, sizeof(nearMonsterInfo) * cnt);
	}
	p.size = static_cast<std::uint8_t>(offsetof(sc_packet_monster_near, monsters) + sizeof(nearMonsterInfo) * cnt);
	registerSend(&p);
}

void GAMESESSION::sendMapInfo(int player_id, int x, int y) {
	int left_x = x - static_cast<int>(SCREEN_WIDTH) / 2;
	int top_y = y - static_cast<int>(SCREEN_HEIGHT) / 2;

	const int MAP_ROW_CHUNK = 4;
	for (int chunk_y = 0; chunk_y < SCREEN_HEIGHT; chunk_y += MAP_ROW_CHUNK) {
		sc_packet_mapInfo p{};
		p.type = S2C_P_MAPINFO;
		p.left_x = static_cast<std::int16_t>(left_x);
		p.top_y = static_cast<std::int16_t>(top_y + chunk_y);

		for (int j = 0; j < MAP_ROW_CHUNK; ++j) {
			for (int i = 0; i < SCREEN_WIDTH; ++i) {
				int real_x = left_x + i;
				int real_y = top_y + chunk_y + j;

				if (real_x >= 0 && real_x < MAP_WIDTH && real_y >= 0 && real_y < MAP_HEIGHT)
					p.tiles[j][i] = static_cast<std::uint8_t>(map_data[real_y][real_x]);
				else
					p.tiles[j][i] = 8;
			}
		}

		p.size = sizeof(sc_packet_mapInfo);
		// registerSend copies the packet before returning.
		registerSend(&p);
	}
}

void GAMESESSION::sendStatChange(int t) {
	sc_packet_stat_change p;
	p.type = S2C_P_STAT_CHANGE;
	p.size = sizeof(p);
	p.hp = object._hp;
	p.exp = object._exp;
	p.level = object._level;
	p.seedNum = object._seedNum;
	p.mapType = t;
	registerSend(&p);
}
