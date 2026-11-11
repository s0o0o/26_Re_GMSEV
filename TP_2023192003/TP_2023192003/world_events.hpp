#pragma once
#include "session_registry.hpp"

inline bool currentWorldPlayer(int id,std::uint64_t connectionId) {
    auto player=clients.get(id);
    return id >= 0 && id < MAX_USER && player &&
        player->connectionId == connectionId && player->_state == ST_INGAME &&
        !player->disconnectStarted && !player->transportFailed;
}

// Shared production/test fan-out: only live player connections receive events.
template<class Action>
void forWorldPlayers(const std::unordered_set<int>& audience,Action action) {
    WorldStateLock stateLock;
    for(int id : audience) {
        if(id < 0 || id >= MAX_USER) continue;
        auto player=clients.get(id);
        if(!player) continue;
        if(player->_state != ST_INGAME || player->disconnectStarted || player->transportFailed) continue;
        action(*player);
    }
}
inline void publishCombat(GAMESESSION& target,int attacker,int seeds,bool died,
                          const std::unordered_set<int>& audience) {
    WorldStateLock stateLock;
    forWorldPlayers(audience,[&](GAMESESSION& viewer) {
        if(!world_regions::same(viewer.object.location(),target.object.location())) return;
        viewer.sendCombatUpdate(attacker,seeds,target.object._id,target.object._hp);
        if(died) {
            viewer.sendMonsterDiePacket(target.object._id);
            std::lock_guard<std::mutex> lock(viewer._vlL);
            viewer._viewList.erase(target.object._id);
        }
    });
}

// A player's region change must invalidate queued chase work even if they return quickly.
inline void cancelRegionChases(int playerId) {
    WorldStateLock stateLock;
    for(const auto& entry : clients.snapshot()) {
        if(entry.first<MAX_USER || !entry.second) continue;
        auto& npc=*entry.second;
        std::lock_guard<std::recursive_mutex> combat(npc.monster.combatMutex);
        if(npc.monster.chaseTarget==playerId) {
            ++npc.monster.generation;
            npc.monster.chaseTarget=-1;
            npc.monster._isMonsterChase=false; npc.monster._isActive=false;
        }
        std::lock_guard<std::mutex> view(npc._vlL);
        npc._viewList.erase(playerId);
    }
}

// Socket close and DB persistence stay in disconnect(); this only detaches world state.
template<class RemoveSector>
void detachWorldPresence(GAMESESSION& leaving,RemoveSector removeSector) {
    WorldStateLock stateLock;
    const int id=leaving.object._id;
    removeSector(id);
    { std::lock_guard<std::mutex> lock(leaving._vlL); leaving._viewList.clear(); }
    // Scan observers, not just leaving's list: visibility can be asymmetric.
    for(auto& entry : clients.snapshot()) {
        auto other=entry.second;
        if(!other || other.get() == &leaving) continue;
        if(entry.first >= MAX_USER) {
            std::lock_guard<std::recursive_mutex> lock(other->monster.combatMutex);
            if(other->monster.chaseTarget == id) {
                ++other->monster.generation;
                other->monster.chaseTarget=-1;
                other->monster._isMonsterChase=false;
                other->monster._isActive=false;
            }
            std::lock_guard<std::mutex> viewLock(other->_vlL);
            other->_viewList.erase(id);
        } else if(other->_state == ST_INGAME && !other->disconnectStarted && !other->transportFailed) {
            other->sendRemoveSessionPacket(id);
        } else {
            std::lock_guard<std::mutex> lock(other->_vlL);
            other->_viewList.erase(id);
        }
    }
}
