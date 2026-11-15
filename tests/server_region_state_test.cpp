#include "../TP_2023192003/TP_2023192003/game_object.hpp"
#include "../TP_2023192003/TP_2023192003/world_map.hpp"
#include "../TP_2023192003/TP_2023192003/pathfinder.hpp"
#include "../TP_2023192003/TP_2023192003/database_position.hpp"
#include <cassert>
int main() {
    using world_regions::Id; using world_regions::Position;
    GameObjectState field,club; field.position={Id::SunnyPeak,10,10}; club.position={Id::Clubhouse,10,10};
    assert(field.location().x==10 && field.position.x==club.position.x);
    assert(!world_regions::same(field.location(),club.location()));
    assert(world_regions::combatAllowed(field.location()) && !world_regions::combatAllowed(club.location()));
    initializeFieldMap();
    assert(playerCanMove(field.location()) && playerCanMove(club.location()));
    assert(!playerCanMove(Position{Id::SunnyPeak,20,10}) && !playerCanMove(Position{}));
    const Position seed{Id::SunnyPeak,4,8},indoor{Id::Clubhouse,4,8};
    assert(mapTileAt(seed)==4 && mapTileAt(indoor)!=4);
    assert(!collectSeed(indoor,1000) && collectSeed(seed,1000) && !collectSeed(seed,1001));
    auto point=Position{Id::Clubhouse,10,14};
    assert(world_layout::portal(point) && point.region==Id::SunnyPeak && point.x==10 && point.y==10);
    point.y=12; assert(world_layout::portal(point) && point.region==Id::Clubhouse && point.x==10 && point.y==12);
    const auto path=findPathAStar(Position{Id::SunnyPeak,6,7},Position{Id::SunnyPeak,7,7});
    assert(path.size()==2 && path.front().first==6 && path.back().first==7);
    assert(findPathAStar(field.location(),club.location()).empty());
    assert(findPathAStar(Position{Id::Invalid,6,7},Position{Id::Invalid,7,7}).empty());
    world_regions::LegacyPosition persisted;
    assert(database_position::encodeLegacy(field.location(),persisted) && persisted.x==42 && persisted.y==10);
    const auto restored=database_position::restoreLegacy(persisted.x,persisted.y);
    assert(!restored.recovered && restored.position.region==field.position.region && restored.position.x==10);
    assert(field_content::monsterCanStand({Id::SunnyPeak,6,7}) && !field_content::monsterCanStand({Id::Clubhouse,6,7}));
}
