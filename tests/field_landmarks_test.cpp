#include "../shared/field_landmarks.hpp"
#include <cassert>
#include <queue>
int main() {
    bool seen[256][256]{};std::queue<std::pair<int,int>> q;q.push({10,10});seen[10][10]=true;
    int reached=0;
    while(!q.empty()) { auto p=q.front();q.pop();++reached;
        for(auto d: {std::pair<int,int>{1,0},{-1,0},{0,1},{0,-1}}) {
            int x=p.first+d.first,y=p.second+d.second;
            if(x<0 || y<0 || x>=256 || y>=256 || seen[y][x] || !field_terrain::walkable(x,y)) continue;
            seen[y][x]=true;q.push({x,y});
        }
    }
    assert(reached==62640);
    for(const auto& p:field_landmarks::all) {
        assert(p.id>=1 && p.id<=5 && seen[p.y][p.x]);
        for(const auto& other:field_landmarks::all) if(&p!=&other) assert(p.id!=other.id);
    }
    for(int y=0;y<256;++y) for(int x=0;x<256;++x) {
        const auto d=field_landmarks::detail(x,y);assert(d<=3);
        if(d) assert(!field_terrain::starter(x,y) && field_terrain::at(x,y)==field_terrain::Tile::Grass);
        assert(field_landmarks::zone(x,y)>=0 && field_landmarks::zone(x,y)<4);
    }
}
