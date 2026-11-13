#pragma once

#include "iocp_operation.hpp"
#include "game_object.hpp"
#include "monster_state.hpp"
#include "world_state.hpp"
#include <mutex>
#include <atomic>
#include <unordered_set>
#include <memory>

// Pending I/O owns this connection through its operation, independently of registry ID reuse.
// Gameplay and AI data remain in this transitional aggregate.
class GAMESESSION : public std::enable_shared_from_this<GAMESESSION> {
public:
    inline static std::atomic<std::uint64_t> nextConnectionId{1};
    const std::uint64_t connectionId=nextConnectionId.fetch_add(1);
    OVER_EXP _recvOver; // Saved fragment only; pending OS receives use owned heap operations.
    SOCKET _socket = INVALID_SOCKET;
    std::atomic<STATE> _state{ST_FREE};
    std::mutex transportMutex;
    std::recursive_mutex completionMutex;
    std::atomic<bool> recvPending{false};
    int _prevRemainData = 0;
    int _userInputId = 0;
    std::atomic<bool> transportFailed{false};
    std::atomic<bool> disconnectStarted{false};
    std::unordered_set<int> _viewList;
    std::mutex _vlL;

    GameObjectState object;
    MonsterState monster;

    GAMESESSION() = default;
    GameObjectState snapshotObject() const { WorldStateLock lock; return object; }

	bool registerRecv();
	void registerSend(void*);
    void closeTransport();

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
    // Legacy old-coordinate arguments remain for callers; the index owns canonical membership.
	void updateSector(short, short);
	std::unordered_set<int> getNearBySessions();
};
