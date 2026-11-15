#pragma once
#include "game_session.hpp"

inline bool sessionCompletionType(COMP_TYPE type) {
    return type == OP_RECV || type == OP_SEND || type == OP_LOGIN_OK || type == OP_LOGIN_FAIL;
}
enum class SessionCompletion { Ignore, Process, Disconnect };

// object._id is assigned once at allocation; operation retains this exact connection.
// Successful complete sends have no gameplay/fragment/registry mutation to serialize.
inline bool releaseSendCompletionWithoutLock(const OVER_EXP& operation,ULONG_PTR key,bool success,DWORD bytes) {
    const auto& owner=operation.owner;
    if(operation._compType!=OP_SEND || !owner || key!=static_cast<ULONG_PTR>(owner->object._id)) return false;
    // Once disconnect has an owner, SEND completions cannot add cleanup or gameplay work.
    if(owner->disconnectStarted.load()) return true;
    return success && bytes>0 && bytes==operation._wasBuf.len && !owner->transportFailed.load();
}

// Caller holds owner's completionMutex. Never resolve a network completion by ID.
inline SessionCompletion classifySessionCompletion(const OVER_EXP& operation,
    ULONG_PTR key,bool success,DWORD bytes) {
    auto owner=operation.owner;
    if(!sessionCompletionType(operation._compType) || !owner ||
       key != static_cast<ULONG_PTR>(owner->object._id) ||
       owner->disconnectStarted) return SessionCompletion::Ignore;
    // A failed submission wakes RECV via shutdown; that completion must still close the session.
    if(owner->transportFailed) return SessionCompletion::Disconnect;
    if(!success || ((operation._compType == OP_RECV || operation._compType == OP_SEND) && bytes == 0))
        return SessionCompletion::Disconnect;
    if(operation._compType==OP_SEND && bytes!=operation._wasBuf.len) return SessionCompletion::Disconnect;
    return SessionCompletion::Process;
}
