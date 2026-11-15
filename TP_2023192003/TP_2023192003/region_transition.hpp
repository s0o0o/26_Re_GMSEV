#pragma once

// Included after server map/sector and AI declarations. All entry points take
// WorldStateLock; the owning connection's completion lock is held by IOCP.
inline bool transitionRegion(GAMESESSION& session,world_regions::Position target,
                             std::uint32_t moveTime=0) {
    WorldStateLock lock;
    if(!world_regions::valid(session.object.location()) || !playerCanMove(target)
       || session.object.position.region==target.region) return false;
    const int id=session.object._id;
    std::unordered_set<int> oldView;
    { std::lock_guard<std::mutex> view(session._vlL); oldView=session._viewList; }
    // Preserve explicit LEAVE for tools/clients that do not clear on own MOVE.
    for(int otherId : oldView) session.sendRemoveSessionPacket(otherId);
    detachWorldPresence(session,[](int leavingId) { worldSectors.remove(leavingId); });
    session.object.position=target;
    worldSectors.put(id,target);
    // MOVE before new map/ENTER: native also discards the previous region's cache.
    session.sendMovePacket(id,moveTime);
    session.sendMapInfo(id,target);
    const auto nearby=session.getNearBySessions();
    nearMonsterInfo monsters[MAX_NEAR_MONSTERS]{};
    int count=0;
    for(int otherId : nearby) {
        auto other=clients.get(otherId);
        if(!other || other->_state!=ST_INGAME || other->disconnectStarted || other->transportFailed) continue;
        if(isPlayer(otherId)) other->sendAddPlayerPacket(id);
        else if(isMonster(otherId)) wakeUpMonster(otherId,id);
        session.sendAddPlayerPacket(otherId);
        if(otherId>=MAX_USER && count<MAX_NEAR_MONSTERS
           && town_npcs::hostile(other->monster._monType) && !other->monster._isDie
           && world_regions::combatAllowed(target)
           && world_regions::inRange(target,other->object.location(),2)) {
            monsters[count++]={otherId,other->object._hp,other->monster._monType};
        }
    }
    session.sendMonsterNearPacket(monsters,count);
    return true;
}
