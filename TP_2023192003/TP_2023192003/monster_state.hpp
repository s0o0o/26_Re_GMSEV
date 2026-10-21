#pragma once

#include <atomic>
#include <mutex>
#include "../../shared/respawn_policy.hpp"

struct lua_State;

// Lua lifetime is still managed by the existing server initialization code.
struct MonsterState {
    lua_State* _L = nullptr;
    std::mutex _ll;
    std::recursive_mutex combatMutex;
    std::atomic<std::uint64_t> generation{0};
    respawn_policy::Tick respawnAt=0;
    std::atomic<bool> _isActive{false};
    std::atomic<bool> _isMonsterChase{false};
    std::atomic<int> chaseTarget{-1};
    std::atomic<bool> _isDie{false};
    int _monType = 0;
    short _nextMonX = 0, _nextMonY = 0;
};
