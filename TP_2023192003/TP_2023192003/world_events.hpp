#pragma once
#include "session_registry.hpp"

inline bool currentWorldPlayer(int id,std::uint64_t connectionId) {
    auto it=clients.find(id);
    return id >= 0 && id < MAX_USER && it != clients.end() && it->second &&
        it->second->connectionId == connectionId && it->second->_state == ST_INGAME &&
        !it->second->disconnectStarted && !it->second->transportFailed;
}

// Shared production/test fan-out: only live player connections receive events.
template<class Action>
void forWorldPlayers(const std::unordered_set<int>& audience,Action action) {
    for(int id : audience) {
        if(id < 0 || id >= MAX_USER) continue;
        auto it=clients.find(id);
        if(it == clients.end() || !it->second) continue;
        auto player=it->second;
        if(player->_state != ST_INGAME || player->disconnectStarted || player->transportFailed) continue;
        action(*player);
    }
}
inline void publishCombat(GAMESESSION& target,int attacker,int seeds,bool died,
                          const std::unordered_set<int>& audience) {
    forWorldPlayers(audience,[&](GAMESESSION& viewer) {
        viewer.sendCombatUpdate(attacker,seeds,target.object._id,target.object._hp);
        if(died) {
            viewer.sendMonsterDiePacket(target.object._id);
            std::lock_guard<std::mutex> lock(viewer._vlL);
            viewer._viewList.erase(target.object._id);
        }
    });
}

// Socket close and DB persistence stay in disconnect(); this only detaches world state.
template<class RemoveSector>
void detachWorldPresence(GAMESESSION& leaving,RemoveSector removeSector) {
    const int id=leaving.object._id;
    removeSector(id);
    { std::lock_guard<std::mutex> lock(leaving._vlL); leaving._viewList.clear(); }
    // Scan observers, not just leaving's list: visibility can be asymmetric.
    for(auto& entry : clients) {
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
