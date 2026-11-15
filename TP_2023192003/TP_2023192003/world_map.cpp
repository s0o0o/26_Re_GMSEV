#include "world_map.hpp"
#include <mutex>

namespace {
std::mutex mapMutex;
field_chunks::Cache terrain;
std::array<respawn_policy::Tick,field_content::seeds.size()> seedDeadlines{};
bool walkableLocked(world_regions::Position p) {
    if(!world_regions::valid(p)) return false;
    return p.region==world_regions::Id::SunnyPeak ? field_terrain::walkable(terrain.at(p))
        : clubhouse::walkable(p.x,p.y);
}
int seedIndex(world_regions::Position p) {
    for(std::size_t i=0;i<field_content::seeds.size();++i) {
        const auto seed=field_content::position(field_content::seeds[i]);
        if(p.region==seed.region && p.x==seed.x && p.y==seed.y) return static_cast<int>(i);
    }
    return -1;
}
}
int mapTileAt(world_regions::Position p) {
    if(!world_regions::valid(p)) return 8;
    std::lock_guard<std::mutex> lock(mapMutex);
    if(!walkableLocked(p)) return 3;
    const int seed=seedIndex(p);
    return seed>=0 && seedDeadlines[seed]==0 ? 4 : 0;
}
int mapTileAt(int x,int y) { return mapTileAt(world_regions::decode(x,y)); }
bool playerCanMove(world_regions::Position p) noexcept {
    if(!world_regions::valid(p)) return false;
    std::lock_guard<std::mutex> lock(mapMutex);
    return walkableLocked(p);
}
bool playerCanMove(int x,int y) noexcept { return playerCanMove(world_regions::decode(x,y)); }
bool collectSeed(world_regions::Position p,respawn_policy::Tick now) {
    if(!world_regions::valid(p)) return false;
    std::lock_guard<std::mutex> lock(mapMutex);
    const int seed=seedIndex(p);
    if(seed<0 || seedDeadlines[seed]!=0 || !walkableLocked(p)) return false;
    seedDeadlines[seed]=now+respawn_policy::seedDelay;
    return true;
}
bool collectSeed(int x,int y,respawn_policy::Tick now) { return collectSeed(world_regions::decode(x,y),now); }
std::vector<field_content::Point> restoreSeeds(respawn_policy::Tick now) {
    std::lock_guard<std::mutex> lock(mapMutex);
    std::vector<field_content::Point> restored;
    for(std::size_t i=0;i<seedDeadlines.size();++i) {
        if(!respawn_policy::due(seedDeadlines[i],now)) continue;
        seedDeadlines[i]=0; restored.push_back(field_content::seeds[i]);
    }
    return restored;
}

void initializeFieldMap() {
    std::lock_guard<std::mutex> lock(mapMutex);
    seedDeadlines.fill(0);
    terrain.clear();
}
field_chunks::Stats mapCacheStats() { std::lock_guard<std::mutex> lock(mapMutex); return terrain.stats(); }
