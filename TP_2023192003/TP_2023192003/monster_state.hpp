#pragma once

#include <atomic>
#include <mutex>
#include <memory>
#include <string>
#include "../../shared/respawn_policy.hpp"
#include "../../shared/town_behaviour.hpp"

struct lua_State;

// Lua's custom deleter is installed where the VM is created, not in this header.
struct MonsterState {
    town_behaviour::State residentBehaviour;
    std::shared_ptr<lua_State> script;
    bool scriptDisabled=false; // World-protected; failures disable only this NPC's script.
    std::string scriptError;
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
