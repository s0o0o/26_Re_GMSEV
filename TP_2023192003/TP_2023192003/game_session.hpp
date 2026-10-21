#pragma once

#include "iocp_operation.hpp"
#include "game_object.hpp"
#include "monster_state.hpp"
#include <mutex>
#include <atomic>
#include <unordered_set>

// Transitional aggregate: existing worker code keeps the same session lifetime.
// Connection, gameplay and AI data now have explicit boundaries.
class GAMESESSION {
public:
    inline static std::atomic<std::uint64_t> nextConnectionId{1};
    const std::uint64_t connectionId=nextConnectionId.fetch_add(1);
    OVER_EXP _recvOver;
    SOCKET _socket = 0;
    STATE _state = ST_FREE;
    int _prevRemainData = 0;
    int _userInputId = 0;
    std::atomic<bool> transportFailed{false};
    std::atomic<bool> disconnectStarted{false};
    std::unordered_set<int> _viewList;
    std::mutex _vlL;

    GameObjectState object;
    MonsterState monster;

    GAMESESSION() = default;

	bool registerRecv();
	void registerSend(void*);

	void sendSessionLoginPacket();
	void sendSessioLoginOkPacket();
	void sendMovePacket(int, std::uint32_t moveTime = 0);
	void sendAddPlayerPacket(int);
	void sendRemoveSessionPacket(int);
	void sendChatPacket(int, const char*);
	void sendAttackPacket_PtoN(int, int);		// PlayertoMon
	void sendAttackPacket_NtoP(int);		// MontoPlayer
	void sendMonsterDiePacket(int);
    void sendCombatUpdate(int attacker,int seeds,int target,int hp);
    void sendAttackFeedback(AttackFeedback reason);
	void sendMonsterNearPacket(nearMonsterInfo*, int);
	void sendMapInfo(int, int, int);
	void sendStatChange(int);

	// sector
	void updateSector(short, short);
	std::unordered_set<int> getNearBySessions();
};
