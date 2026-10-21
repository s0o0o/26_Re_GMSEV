#include "../shared/clubhouse_map.hpp"
#include "../shared/world_layout.hpp"
#include "../shared/field_content.hpp"
#include "../TP_2023192003/TP_2023192003/world_map.hpp"
#include <thread>
#include <atomic>
#include <vector>
#include <cassert>
#include <queue>
#include <utility>
int main() {
    using namespace clubhouse;
    assert(walkable(spawnX,spawnY));
    for (const auto& point : {std::make_pair(-1,0),std::make_pair(1999,1999),std::make_pair(5,11)}) {
        int x=point.first,y=point.second;
        safePosition(x,y);
        assert(x == spawnX && y == spawnY);
    }
    bool seen[height][width]{};
    std::queue<std::pair<int,int>> pending;
    pending.push({spawnX,spawnY}); seen[spawnY][spawnX]=true;
    while (!pending.empty()) {
        auto p=pending.front(); pending.pop();
        for (auto d : {std::make_pair(1,0),std::make_pair(-1,0),std::make_pair(0,1),std::make_pair(0,-1)}) {
            int x=p.first+d.first,y=p.second+d.second;
            if (walkable(x,y) && !seen[y][x]) { seen[y][x]=true; pending.push({x,y}); }
        }
    }
    for (int y=0;y<height;++y) for (int x=0;x<width;++x) {
        if (walkable(x,y)) assert(seen[y][x]);
        if (x==0 || y==0 || x==width-1 || y==height-1) assert(!walkable(x,y));
    }
    int x=10,y=14;
    assert(world_layout::portal(x,y));
    assert(x == 42 && y == 10 && world_layout::walkable(x,y));
    assert(!world_layout::portal(x,y));
    y=12;
    assert(world_layout::portal(x,y));
    assert(x == 10 && y == 12 && world_layout::walkable(x,y));
    assert(!world_layout::portal(x,y));
    x=42; y=10;
    world_layout::safePosition(x,y);
    assert(x == 42 && y == 10);
    assert(!world_layout::walkable(27,10));
    assert(world_layout::cameraX(34) == 32 && world_layout::cameraX(49) == 34);
    bool fieldSeen[18][20]{};
    pending.push({10,10}); fieldSeen[10][10]=true;
    while (!pending.empty()) {
        auto p=pending.front(); pending.pop();
        for (auto d : {std::make_pair(1,0),std::make_pair(-1,0),std::make_pair(0,1),std::make_pair(0,-1)}) {
            int nx=p.first+d.first,ny=p.second+d.second;
            if (world_layout::fieldWalkable(nx,ny) && !fieldSeen[ny][nx]) {
                fieldSeen[ny][nx]=true; pending.push({nx,ny});
            }
        }
    }
    for (int fy=0;fy<18;++fy) for(int fx=0;fx<20;++fx)
        if (world_layout::fieldWalkable(fx,fy)) assert(fieldSeen[fy][fx]);
    initializeFieldMap();
    for(auto monster : field_content::monsters) assert(field_content::monsterCanStand(monster.x,monster.y));
    assert(!field_content::monsterCanStand(11,12));
    assert(!field_content::monsterCanStand(42,12));
    auto seed=field_content::seeds[0];
    assert(mapTileAt(seed.x,seed.y) == 4);
    std::atomic<int> collected{0};
    std::vector<std::thread> collectors;
    for(int i=0;i<16;++i) collectors.emplace_back([&] { if(collectSeed(seed.x,seed.y)) ++collected; });
    for(auto& thread : collectors) thread.join();
    assert(collected == 1 && mapTileAt(seed.x,seed.y) == 0);
    assert(!collectSeed(-1,0) && !collectSeed(11,12));
    for(auto other : field_content::seeds) assert(world_layout::outdoors(other.x) && world_layout::walkable(other.x,other.y));
}
