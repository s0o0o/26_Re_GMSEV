#include "../TP_2023192003/TP_2023192003/game_database.hpp"
#include <cassert>
#include <climits>
#include <initializer_list>
static int writes=0,savedX=-1,savedY=-1;
bool update_user_info(int,int x,int y,int,int,int,int,int,int) { ++writes; savedX=x; savedY=y; return true; }
int main() {
    using world_regions::Id;
    for(auto id : {Id::Clubhouse,Id::SunnyPeak}) {
        const auto d=world_regions::describe(id);
        for(int y=0;y<d.height;++y) for(int x=0;x<d.width;++x) {
            world_regions::LegacyPosition encoded;
            const bool valid=database_position::encodeLegacy({id,x,y},encoded);
            if(!valid) continue;
            const auto restored=database_position::restoreLegacy(encoded.x,encoded.y);
            assert(!restored.recovered && restored.position.region==id && restored.position.x==x && restored.position.y==y);
        }
    }
    for(auto legacy : {world_regions::LegacyPosition{INT_MAX,INT_MIN},{-1,10},{31,10},{5,10}}) {
        const auto result=database_position::restoreLegacy(legacy.x,legacy.y);
        assert(result.recovered && result.position.region==Id::Clubhouse && result.legacy.x==clubhouse::spawnX && result.legacy.y==clubhouse::spawnY);
    }
    for(auto position : {world_regions::Position{Id::Invalid,0,0},{Id::Clubhouse,5,10},{Id::SunnyPeak,20,10}}) {
        world_regions::LegacyPosition untouched{777,888};
        assert(!database_position::encodeLegacy(position,untouched) && untouched.x==777 && untouched.y==888);
        const int before=writes; assert(!update_user_position(123,position,5,10,1,0,20,1) && writes==before);
    }
    assert(update_user_position(123,{Id::SunnyPeak,10,10},5,10,1,0,20,1) && savedX==42 && savedY==10);
    assert(update_user_position(123,{Id::Clubhouse,10,10},5,10,1,0,20,1) && savedX==10 && savedY==10);
}
