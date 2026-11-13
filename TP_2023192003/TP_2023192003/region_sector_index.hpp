#pragma once
#include "../../shared/world_regions.hpp"
#include <algorithm>
#include <map>
#include <mutex>
#include <stdexcept>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace region_sectors {
struct Key {
    world_regions::Id region=world_regions::Id::Invalid;
    int x=0,y=0;
    bool operator<(const Key& other) const { return std::tie(region,x,y)<std::tie(other.region,other.x,other.y); }
    bool operator==(const Key& other) const { return region==other.region && x==other.x && y==other.y; }
};
struct Entry { int id; Key key; };
// No registry, world lock, Lua, or socket callbacks are invoked while holding mutex_.
class Index {
    const int tileSize_;
    mutable std::mutex mutex_;
    std::map<Key,std::unordered_set<int>> buckets_;
    std::unordered_map<int,Key> membership_;
    void eraseFromBucket(int id,const Key& key) {
        auto bucket=buckets_.find(key);
        if(bucket==buckets_.end()) return;
        bucket->second.erase(id);
        if(bucket->second.empty()) buckets_.erase(bucket);
    }
public:
    explicit Index(int tileSize=20) : tileSize_(tileSize) {
        if(tileSize<=0) throw std::invalid_argument("Sector tile size must be positive");
    }
    bool keyFor(world_regions::Position p,Key& key) const {
        if(!world_regions::valid(p)) return false;
        key={p.region,p.x/tileSize_,p.y/tileSize_}; return true;
    }
    bool put(int id,world_regions::Position p) {
        Key key;
        if(id<0 || !keyFor(p,key)) return false; // Preserve old membership on invalid updates.
        std::lock_guard<std::mutex> guard(mutex_);
        const auto old=membership_.find(id);
        if(old!=membership_.end() && old->second==key) return true;
        buckets_[key].insert(id);
        if(old!=membership_.end()) { const auto previous=old->second; old->second=key; eraseFromBucket(id,previous); }
        else membership_.emplace(id,key);
        return true;
    }
    bool remove(int id) {
        std::lock_guard<std::mutex> guard(mutex_);
        auto entry=membership_.find(id);
        if(entry==membership_.end()) return false;
        eraseFromBucket(id,entry->second); membership_.erase(entry); return true;
    }
    std::unordered_set<int> nearby(world_regions::Position p,int range) const {
        std::unordered_set<int> result;
        if(!world_regions::valid(p) || range<0) return result;
        const auto d=world_regions::describe(p.region);
        const int bounded=(std::min)(range,(std::max)(d.width,d.height));
        const int minX=(std::max)(0,p.x-bounded)/tileSize_,maxX=(std::min)(d.width-1,p.x+bounded)/tileSize_;
        const int minY=(std::max)(0,p.y-bounded)/tileSize_,maxY=(std::min)(d.height-1,p.y+bounded)/tileSize_;
        std::lock_guard<std::mutex> guard(mutex_);
        for(int x=minX;x<=maxX;++x) for(int y=minY;y<=maxY;++y) {
            const auto bucket=buckets_.find({p.region,x,y});
            if(bucket!=buckets_.end()) result.insert(bucket->second.begin(),bucket->second.end());
        }
        return result; // Candidates only; caller checks current identity/state and exact distance.
    }
    int memberships(int id) const {
        std::lock_guard<std::mutex> guard(mutex_); return static_cast<int>(membership_.count(id));
    }
    std::size_t bucketCount() const { std::lock_guard<std::mutex> guard(mutex_); return buckets_.size(); }
    std::vector<Entry> snapshot() const {
        std::lock_guard<std::mutex> guard(mutex_);
        std::vector<Entry> result;
        for(const auto& bucket : buckets_) for(int id : bucket.second) result.push_back({id,bucket.first});
        std::sort(result.begin(),result.end(),[](const Entry& a,const Entry& b) { return a.id<b.id; });
        return result;
    }
};
}
