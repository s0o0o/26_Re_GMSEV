#define main hamster_game_main
#include "../TP_2023192003_CLIENT/client_sample/client.cpp"
#undef main
#include <cassert>
#include <filesystem>

int main() {
    const auto root=std::filesystem::current_path();
    std::filesystem::current_path(root/"TP_2023192003_CLIENT/client_sample");
    assert(gameFont().loadFromFile("neodgm.ttf"));
    assert(hamsterCustomizer().load("assets/characters"));
    assert(clubhouseRenderer().load("assets/maps/clubhouse-sheet.png"));
    assert(clubhouseRenderer().loadOutdoor("assets/maps/sunny-peak-sheet.png"));
    sf::RenderTexture scene,contact; assert(scene.create(900,800) && contact.create(1800,1600));
    sf::Sprite hamster(hamsterCustomizer().worldTexture(0,0,0,0),{48,0,48,48});
    const world_regions::Position positions[]={{world_regions::Id::SunnyPeak,15,9},
        {world_regions::Id::SunnyPeak,81,32},{world_regions::Id::SunnyPeak,112,129},{world_regions::Id::SunnyPeak,250,250}};
    auto render=[&](world_regions::Position p,bool collision) {
        const auto left=world_layout::cameraX(p),top=world_layout::cameraY(p);
        scene.clear(); clubhouseRenderer().draw(scene,p.region,left,top);
        const auto stats=clubhouseRenderer().fieldRenderer().lastDraw();
        assert(stats.chunks<=6 && stats.tiles<=18*16);
        assert(clubhouseRenderer().fieldRenderer().cacheStats().resident<=12);
        if(!field_terrain::starter(p.x,p.y)) assert(stats.tiles==18*16);
        if(collision) for(int y=0;y<16;++y) for(int x=0;x<18;++x) {
            if(!world_layout::walkable({p.region,left+x,top+y})) {
                sf::RectangleShape tile({50,50}); tile.setPosition(x*50.f,y*50.f); tile.setFillColor({190,24,24,105}); scene.draw(tile);
            }
        }
        world_character::place(hamster,(p.x-left)*50.f,(p.y-top)*50.f); scene.draw(hamster);
        clubhouseRenderer().drawLabel(scene,gameFont(),p.region);
        travelPanel.draw(scene,gameFont(),p.region,{});
        scene.display();
    };
    for(int i=0;i<4;++i) {
        render(positions[i],false);
        const auto path=root/("build/tests/field-view-"+std::to_string(i)+".png");
        assert(scene.getTexture().copyToImage().saveToFile(path.string()));
        sf::Sprite sprite(scene.getTexture()); sprite.setPosition((i%2)*900.f,(i/2)*800.f); contact.draw(sprite);
    }
    contact.display(); assert(contact.getTexture().copyToImage().saveToFile((root/"build/tests/field-scenes.png").string()));
    render(positions[1],true);
    assert(scene.getTexture().copyToImage().saveToFile((root/"build/tests/field-collision-preview.png").string()));
    for(const auto size : {sf::Vector2u{1280,720},sf::Vector2u{800,900}}) {
        render(positions[1],false);
        sf::RenderTexture output; assert(output.create(size.x,size.y));
        const auto fit=display_layout::fit(size.x,size.y);
        sf::View view(sf::FloatRect(0,0,900,800));
        view.setViewport({fit.left/size.x,fit.top/size.y,fit.width/size.x,fit.height/size.y});
        output.setView(view); output.clear({32,24,24}); output.draw(sf::Sprite(scene.getTexture())); output.display();
        assert(output.getTexture().copyToImage().saveToFile((root/("build/tests/field-"+std::to_string(size.x)+"x"+std::to_string(size.y)+".png")).string()));
    }
    // Entire-map sweep proves render work is bounded by the view, not world area.
    for(int y=0;y<256;y+=16) for(int x=0;x<256;x+=16) {
        const auto p=world_regions::Position{world_regions::Id::SunnyPeak,x,y};
        clubhouseRenderer().draw(scene,p.region,world_layout::cameraX(p),world_layout::cameraY(p));
        const auto stats=clubhouseRenderer().fieldRenderer().lastDraw();
        assert(stats.chunks<=6 && stats.tiles<=288 && clubhouseRenderer().fieldRenderer().cacheStats().resident<=12);
    }
    assert(!field_chunks::onScreen({world_regions::Id::SunnyPeak,200,200},world_regions::Id::SunnyPeak,0,0,18,16,1));
    assert(field_chunks::onScreen({world_regions::Id::SunnyPeak,19,15},world_regions::Id::SunnyPeak,1,1,18,16,1));
    assert(!field_chunks::onScreen({world_regions::Id::SunnyPeak,20,15},world_regions::Id::SunnyPeak,1,1,18,16,1));
    std::filesystem::current_path(root);
    std::puts("field renderer: start/bridge/cliff/far edge, collision diagnostic, resized views, bounded visible chunks/tiles/cache/culling PASS");
}
