#include "../TP_2023192003/TP_2023192003/world_map.hpp"
#include "../TP_2023192003/TP_2023192003/pathfinder.hpp"
#include "../TP_2023192003/TP_2023192003/database_position.hpp"
#include "../shared/protocol.hpp"
#include <cassert>
#include <climits>
#include <queue>
#include <cstdio>
int main() {
    using world_regions::Id; using world_regions::Position; using field_terrain::Tile;
    initializeFieldMap(); assert(mapCacheStats().resident==0);
    assert(world_regions::fieldWidth==256 && world_regions::fieldHeight==256);
    sc_packet_avatar_info oldInfo{}; oldInfo.size=sizeof(oldInfo); oldInfo.type=S2C_P_AVATAR_INFO; oldInfo.version=3;
    assert(!is_valid_server_packet(reinterpret_cast<const std::uint8_t*>(&oldInfo),sizeof(oldInfo)));
    field_chunks::Cache cache(3);
    for(int y=0;y<256;++y) for(int x=0;x<256;++x) {
        const Position p{Id::SunnyPeak,x,y};
        assert(cache.at(p)==field_terrain::at(x,y));
        assert(playerCanMove(p)==field_terrain::walkable(x,y));
        assert((mapTileAt(p)==3)==!field_terrain::walkable(x,y));
        assert(cache.stats().resident<=3 && mapCacheStats().resident<=16);
    }
    assert(cache.stats().evictions>0 && mapCacheStats().evictions>0);
    assert(field_terrain::at(81,32)==Tile::Bridge && field_terrain::at(81,30)==Tile::Water);
    assert(field_terrain::at(32,129)==Tile::Trail && field_terrain::at(40,129)==Tile::Cliff);
    assert(playerCanMove({Id::SunnyPeak,19,9}) && playerCanMove({Id::SunnyPeak,20,9}));
    assert(!playerCanMove({Id::SunnyPeak,19,7}));
    std::array<bool,256*256> visited{}; std::queue<std::pair<int,int>> queue;
    visited[10*256+10]=true; queue.push({10,10});
    while(!queue.empty()) {
        const auto p=queue.front(); queue.pop();
        for(auto d : {std::pair<int,int>{0,1},{0,-1},{1,0},{-1,0}}) {
            const int x=p.first+d.first,y=p.second+d.second;
            if(x<0 || y<0 || x>=256 || y>=256 || visited[y*256+x] || !field_terrain::walkable(x,y)) continue;
            visited[y*256+x]=true; queue.push({x,y});
        }
    }
    std::size_t reachable=0;
    for(int y=0;y<256;++y) for(int x=0;x<256;++x) {
        if(field_terrain::walkable(x,y)) { assert(visited[y*256+x]); ++reachable; }
    }
    assert(reachable>60000 && visited[254*256+254] && visited[32*256+81] && visited[224*256+81]);
    const Position seed{Id::SunnyPeak,4,8};
    assert(collectSeed(seed,1000));
    for(int y=0;y<256;y+=16) for(int x=0;x<256;x+=16) mapTileAt(Position{Id::SunnyPeak,x,y});
    assert(mapTileAt(seed)==0 && !collectSeed(seed,1001));
    assert(restoreSeeds(1000+respawn_policy::seedDelay-1).empty());
    assert(restoreSeeds(1000+respawn_policy::seedDelay).size()==1 && mapTileAt(seed)==4);
    assert(!playerCanMove({Id::SunnyPeak,256,255}) && mapTileAt(Position{})==8);
    const auto before=cache.stats().resident;
    assert(cache.at({Id::SunnyPeak,INT_MIN,INT_MAX})==Tile::Cliff && cache.stats().resident==before);
    assert(field_chunks::generate(INT_MAX,INT_MIN).cells[0]==Tile::Cliff);
    const auto bounds=field_chunks::visible(15,15); assert(bounds.left==0 && bounds.right==2 && bounds.bottom==1);
    const auto end=field_chunks::visible(250,250,INT_MAX,INT_MAX); assert(end.right==15 && end.bottom==15);
    assert(field_chunks::visible(-1,0).right==-1);
    assert(field_chunks::onScreen({Id::SunnyPeak,254,254},Id::SunnyPeak,238,240));
    assert(!field_chunks::onScreen({Id::Clubhouse,10,10},Id::SunnyPeak,0,0));
    assert(!field_chunks::onScreen({Id::SunnyPeak,10,10},Id::SunnyPeak,INT_MIN,0));
    world_regions::LegacyPosition legacy;
    assert(database_position::encodeLegacy({Id::SunnyPeak,254,254},legacy) && legacy.x==286 && legacy.y==254);
    auto restored=database_position::restoreLegacy(legacy.x,legacy.y); assert(!restored.recovered && restored.position.x==254);
    PathSearchStats stats;
    assert(findPathAStar({Id::SunnyPeak,32,32},{Id::SunnyPeak,240,240},32,&stats).empty());
    assert(stats.exhausted && stats.expanded==32 && stats.allocated<=129);
    assert(!findPathAStar({Id::SunnyPeak,79,32},{Id::SunnyPeak,85,32},1024,&stats).empty());
    assert(!stats.exhausted && stats.expanded<1024);
    std::printf("chunks: full terrain/collision/connectivity (%zu tiles), bounded LRU, pickup eviction, camera/DB edges, path budget PASS\n",reachable);
}
