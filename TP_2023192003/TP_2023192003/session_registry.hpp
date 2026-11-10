#pragma once

#include <memory>
#include <mutex>
#include <unordered_map>
#include <vector>
#include "game_session.hpp"

// Protect the mapped shared_ptr too, not only the container's buckets.
// No iterator or reference to a mutable map entry escapes this class.
class SessionRegistry {
public:
    using Session=std::shared_ptr<GAMESESSION>;
    using Entry=std::pair<long long,Session>;

    Session get(long long id) const {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it=sessions_.find(id);
        return it == sessions_.end() ? nullptr : it->second;
    }
    void set(long long id,Session session) {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            sessions_[id].swap(session);
        } // Retired sessions are released outside the registry lock.
    }
    std::vector<Entry> snapshot() const {
        std::lock_guard<std::mutex> lock(mutex_);
        std::vector<Entry> result;
        result.reserve(sessions_.size());
        for(const auto& entry : sessions_) result.emplace_back(entry.first,entry.second);
        return result;
    }
    int allocatePlayer(Session session) {
        if(!session) return -1;
        std::unique_lock<std::mutex> lock(mutex_);
        if(session->object._id != -1 || session->_state != ST_FREE) return -1;
        for(int id=0;id<MAX_USER;++id) {
            auto it=sessions_.find(id);
            if(it != sessions_.end() && it->second && it->second->_state != ST_FREE) continue;
            session->object._id=id;
            session->_state=ST_ALLOC;
            sessions_[id].swap(session);
            lock.unlock();
            return id;
        }
        return -1;
    }
    void clear() {
        std::unordered_map<long long,Session> retired;
        { std::lock_guard<std::mutex> lock(mutex_); sessions_.swap(retired); }
    }
private:
    mutable std::mutex mutex_;
    std::unordered_map<long long,Session> sessions_;
};

extern SessionRegistry clients;
