#include "../TP_2023192003_CLIENT/client_sample/town_npc_renderer.hpp"
#include "../TP_2023192003_CLIENT/client_sample/clubhouse_renderer.hpp"
#include <cassert>
#include <iostream>
int main() {
    const std::string root="TP_2023192003_CLIENT/client_sample/assets/npcs/source";
    sf::Font font; assert(font.loadFromFile("TP_2023192003_CLIENT/client_sample/neodgm.ttf"));
    const char* keys[]={"arnies-brother","blushie-ham","harmony","shopkeeper","shy-ham","snoozer","mistress","penelope","pashmina","panda","oxnard","cappy","bye-q-child","busy-mother","bulls-eye-kid","boss","ham-chat-j-l","ham-chat-a-c","hungry-ham-2","inventor","postie"};
    sf::RenderTexture catalog; assert(catalog.create(1260,1960)); catalog.clear({48,32,37});
    for(int i=0;i<21;++i) {
        sf::Texture texture; assert(texture.loadFromFile(root+"/"+keys[i]+".png"));
        const auto size=texture.getSize();
        const float scale=(std::min)(390.f/size.x,230.f/size.y);
        sf::Sprite sprite(texture); sprite.setScale(scale,scale);
        sprite.setPosition((i%3)*420.f+(420-size.x*scale)/2,(i/3)*280.f+30); catalog.draw(sprite);
        sf::Text title(keys[i],font,19); title.setFillColor({255,248,228});
        centerText(title,{(i%3)*420.f,(i/3)*280.f,420,28}); catalog.draw(title);
        std::cout<<keys[i]<<" "<<size.x<<"x"<<size.y<<"\n";
    }
    catalog.display(); assert(catalog.getTexture().copyToImage().saveToFile("build/tests/npc-source-catalog.png"));
    TownNpcRenderer residents; assert(residents.load(root));
    for(const auto& resident : town_npcs::residents) {
        const auto image=residents.texture(resident.type).copyToImage();
        assert(image.getPixel(image.getSize().x-1,0).a==0);
        const auto rect=TownNpcRenderer::frame(resident.type); int pixels=0;
        for(int y=rect.top;y<rect.top+rect.height;++y) for(int x=rect.left;x<rect.left+rect.width;++x)
            if(image.getPixel(x,y).a) ++pixels;
        assert(pixels>100);
    }
    ClubhouseRenderer room; assert(room.load("TP_2023192003_CLIENT/client_sample/assets/maps/clubhouse-sheet.png"));
    sf::RenderTexture preview; assert(preview.create(1440,1280)); preview.setView(sf::View({0,0,900,800})); preview.clear();
    const int left=world_layout::cameraX(clubhouse::spawnX),top=world_layout::cameraY(clubhouse::spawnY);
    room.draw(preview,left,top);
    sf::Texture player; assert(player.loadFromFile("TP_2023192003_CLIENT/client_sample/assets/characters/hamtaro/character.png"));
    sf::Sprite avatar(player,{48,0,48,48}); avatar.setScale(2,2); avatar.setOrigin(24,44);
    avatar.setPosition((clubhouse::spawnX-left)*50.f+25,(clubhouse::spawnY-top)*50.f+45); preview.draw(avatar);
    for(const auto& resident : town_npcs::residents)
        residents.draw(preview,resident.type,(resident.x-left)*50.f,(resident.y-top)*50.f,font,resident.name);
    room.drawLabel(preview,font,clubhouse::spawnX); preview.display();
    assert(preview.getTexture().copyToImage().saveToFile("build/tests/town-residents-preview.png"));
    std::cout<<"21 sheets loaded; resident source rectangles/background masks and production renderer preview passed\n";
}
