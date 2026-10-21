#pragma once
#include <WS2tcpip.h>
#include <cstring>
#include "server_config.hpp"

enum COMP_TYPE {
	OP_ACCEPT, OP_RECV, OP_SEND, OP_PLAYER_MOVE, OP_NPC_RUN, OP_LOGIN_OK, OP_LOGIN_FAIL,
	OP_MONSTER_MOVE, OP_MONSTER_CHASE, OP_MONSTER_ATTACK_TO_USER, OP_USER_ATTACK_TO_MONSTER
};
enum STATE { ST_FREE, ST_ALLOC, ST_INGAME };

class OVER_EXP {
public:
	WSAOVERLAPPED		_over;
	WSABUF				_wasBuf;
	char				_sendBuf[BUF_SIZE];

	COMP_TYPE			_compType;
	int					_playerId;				// npc가 반응할 대상(플레이어) id 저장용

	OVER_EXP()
	{
		_wasBuf.len = BUF_SIZE;
		_wasBuf.buf = _sendBuf;
		_compType = OP_RECV;
		ZeroMemory(&_over, sizeof(_over));
	}

	OVER_EXP(char* packet)
	{
		_wasBuf.len = packet[0];
		_wasBuf.buf = _sendBuf;
		ZeroMemory(&_over, sizeof(_over));
		_compType = OP_SEND;
		memcpy(_sendBuf, packet, _wasBuf.len);
	}
};
