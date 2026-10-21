#include "world_map.hpp"
#include "../../shared/field_content.hpp"
#include <mutex>

int map_data[MAP_HEIGHT][MAP_WIDTH];
namespace {
std::mutex mapMutex;
std::array<respawn_policy::Tick,field_content::seeds.size()> seedDeadlines{};
}

int mapTileAt(int x,int y) {
    if (x < 0 || y < 0 || x >= MAP_WIDTH || y >= MAP_HEIGHT) return 8;
    std::lock_guard<std::mutex> lock(mapMutex);
    return map_data[y][x];
}

bool collectSeed(int x,int y,respawn_policy::Tick now) {
    if (x < 0 || y < 0 || x >= MAP_WIDTH || y >= MAP_HEIGHT) return false;
    std::lock_guard<std::mutex> lock(mapMutex);
    if (map_data[y][x] != 4) return false;
    map_data[y][x] = 0;
    for(std::size_t i=0;i<field_content::seeds.size();++i) {
        auto p=field_content::seeds[i];
        if(p.x == x && p.y == y) seedDeadlines[i]=now+respawn_policy::seedDelay;
    }
    return true;
}

std::vector<field_content::Point> restoreSeeds(respawn_policy::Tick now) {
    std::lock_guard<std::mutex> lock(mapMutex);
    std::vector<field_content::Point> restored;
    for(std::size_t i=0;i<seedDeadlines.size();++i) {
        if(!respawn_policy::due(seedDeadlines[i],now)) continue;
        auto p=field_content::seeds[i]; map_data[p.y][p.x]=4;
        seedDeadlines[i]=0; restored.push_back(p);
    }
    return restored;
}

void initializeFieldMap() {
    std::lock_guard<std::mutex> lock(mapMutex);
    seedDeadlines.fill(0);
    for (int y=0;y<MAP_HEIGHT;++y) for(int x=0;x<MAP_WIDTH;++x)
        map_data[y][x]=world_layout::walkable(x,y) ? 0 : 3;
    for(auto seed : field_content::seeds) map_data[seed.y][seed.x]=4;
}

bool playerCanMove(int x, int y) noexcept
{
	if (x < 0 || x >= MAP_WIDTH || y < 0 || y >= MAP_HEIGHT) {
		return false;
	}

	return mapTileAt(x,y) != 3;
}
