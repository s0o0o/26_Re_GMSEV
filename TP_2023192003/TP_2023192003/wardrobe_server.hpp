#pragma once
#include "appearance_storage.hpp"

// Included after server declarations. Completion lock remains owned while
// synchronous persistence runs outside WorldStateLock. No DDL/files are created.
inline void processAppearanceRequest(GAMESESSION& session,const cs_packet_appearance& request,WorldStateLock& world) {
    const auto result=session.wardrobe.request(request.requestId,request.appearance,session.object.position,
        [&](avatar_appearance::Value value) {
            const int account=session._userInputId;
            auto* store=appearance_storage::active.load(std::memory_order_acquire);
            appearance_storage::Status saved=appearance_storage::Status::Error;
            world.unlock();
            try { if(store) saved=store->save(account,value); } catch(...) { saved=appearance_storage::Status::Error; }
            world.lock();
            if(saved==appearance_storage::Status::Unavailable) return wardrobe::Result::StorageUnavailable;
            if(saved!=appearance_storage::Status::Saved) return wardrobe::Result::StorageFailure;
            if(!currentWorldPlayer(session.object._id,session.connectionId)
                || session.object.position.region!=world_regions::Id::Clubhouse) return wardrobe::Result::StorageFailure;
            session.object.setAppearance(value);
            sc_packet_appearance_change change{};
            change.size=sizeof(change); change.type=S2C_P_APPEARANCE_CHANGE;
            change.id=session.object._id; change.region=static_cast<std::uint8_t>(session.object.position.region);
            change.appearance=value;
            session.registerSend(&change);
            // Scan live observers: visibility can be asymmetric.
            for(const auto& entry : clients.snapshot()) {
                auto other=entry.second;
                if(entry.first<0 || entry.first>=MAX_USER || !other || other.get()==&session
                    || !currentWorldPlayer(entry.first,other->connectionId)
                    || !world_regions::same(other->object.position,session.object.position)) continue;
                bool sees=false;
                { std::lock_guard<std::mutex> view(other->_vlL); sees=other->_viewList.count(session.object._id)!=0; }
                if(sees) other->registerSend(&change);
            }
            return wardrobe::Result::Saved;
        });
    sc_packet_appearance_result response{};
    response.size=sizeof(response); response.type=S2C_P_APPEARANCE_RESULT;
    response.requestId=request.requestId; response.result=result; response.appearance=session.object.appearance();
    session.registerSend(&response);
}
