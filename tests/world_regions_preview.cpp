#include "../TP_2023192003_CLIENT/client_sample/clubhouse_renderer.hpp"
#include "../TP_2023192003/TP_2023192003/region_sector_index.hpp"
#include "../TP_2023192003/TP_2023192003/server_config.hpp"
#include "../TP_2023192003/TP_2023192003/database_position.hpp"
#include <cassert>
#include <iostream>
int main() {
    ClubhouseRenderer renderer;
    assert(renderer.load("TP_2023192003_CLIENT/client_sample/assets/maps/clubhouse-sheet.png"));
    assert(renderer.loadOutdoor("TP_2023192003_CLIENT/client_sample/assets/maps/sunny-peak-sheet.png"));
    sf::Font font; assert(font.loadFromFile("TP_2023192003_CLIENT/client_sample/neodgm.ttf"));
    sf::Texture hamster; assert(hamster.loadFromFile("TP_2023192003_CLIENT/client_sample/assets/characters/hamtaro/character.png"));
    sf::RenderTexture output; assert(output.create(1800,990)); output.clear({48,32,37});
    const world_regions::Position positions[]={{world_regions::Id::Clubhouse,10,10},{world_regions::Id::SunnyPeak,10,10}};
    region_sectors::Index index(SECTOR_SIZE);
    assert(index.put(1,positions[0]) && index.put(2,positions[1]) && index.bucketCount()==2);
    for(int i=0;i<2;++i) {
        world_regions::LegacyPosition encoded; assert(world_regions::encode(positions[i],encoded));
        const int left=world_layout::cameraX(encoded.x),top=world_layout::cameraY(encoded.y);
        sf::RenderTexture scene; assert(scene.create(900,800)); scene.clear();
        renderer.draw(scene,left,top);
        sf::Sprite avatar(hamster,{48,0,48,48});
        avatar.setPosition((encoded.x-left)*50.f,(encoded.y-top)*50.f); scene.draw(avatar);
        renderer.drawLabel(scene,font,encoded.x); scene.display();
        sf::Sprite frame(scene.getTexture()); frame.setPosition(i*900.f,65); output.draw(frame);
        sf::Text title(std::string("Region ")+std::to_string(static_cast<int>(positions[i].region))+" : "+world_regions::describe(positions[i].region).name,font,26);
        title.setFillColor({255,248,228}); centerText(title,{i*900.f,5,900,50}); output.draw(title);
        sf::Text position("Local ("+std::to_string(positions[i].x)+", "+std::to_string(positions[i].y)+")  /  Legacy ("+std::to_string(encoded.x)+", "+std::to_string(encoded.y)+")",font,20);
        position.setFillColor({255,248,228}); centerText(position,{i*900.f,870,900,30}); output.draw(position);
        const auto candidates=index.nearby(positions[i],0);
        assert(candidates.size()==1 && candidates.count(i+1) && !candidates.count(2-i));
        region_sectors::Key key; assert(index.keyFor(positions[i],key));
        sf::Text diagnostic("Local sector ("+std::to_string(key.x)+", "+std::to_string(key.y)+") / matches: "+std::to_string(candidates.size())+" / other region excluded",font,18);
        diagnostic.setFillColor({242,212,155}); centerText(diagnostic,{i*900.f,907,900,28}); output.draw(diagnostic);
        world_regions::LegacyPosition stored; assert(database_position::encodeLegacy(positions[i],stored));
        const auto restored=database_position::restoreLegacy(stored.x,stored.y);
        assert(!restored.recovered && restored.position.region==positions[i].region && restored.position.x==positions[i].x && restored.position.y==positions[i].y);
        sf::Text roundtrip("DB codec: ("+std::to_string(stored.x)+", "+std::to_string(stored.y)+") -> Region "+std::to_string(static_cast<int>(restored.position.region))+" / Local (10, 10): OK",font,18);
        roundtrip.setFillColor({255,248,228}); centerText(roundtrip,{i*900.f,941,900,26}); output.draw(roundtrip);
    }
    output.display();
    assert(output.getTexture().copyToImage().saveToFile("build/tests/world-regions-preview.png"));
    assert(output.getTexture().copyToImage().saveToFile("build/tests/region-sectors-preview.png"));
    assert(output.getTexture().copyToImage().saveToFile("build/tests/database-position-preview.png"));
    std::cout<<"Existing production map renderer preview saved (not a DB-connected game screenshot)\n";
}
