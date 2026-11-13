#include "../TP_2023192003_CLIENT/client_sample/town_npc_renderer.hpp"
#include "../TP_2023192003_CLIENT/client_sample/world_character.hpp"
#include "../TP_2023192003_CLIENT/client_sample/clubhouse_renderer.hpp"
#include "../shared/town_behaviour.hpp"
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
        assert(TownNpcRenderer::frame(resident.type,0)!=TownNpcRenderer::frame(resident.type,.4f));
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
    sf::Image original; assert(original.loadFromFile("TP_2023192003_CLIENT/client_sample/assets/characters/hamtaro/character.png"));
    sf::Texture player; assert(player.loadFromImage(world_character::nativeAtlas(original,original,0)));
    sf::Sprite avatar(player,{48,0,48,48});
    world_character::place(avatar,(clubhouse::spawnX-left)*50.f,(clubhouse::spawnY-top)*50.f); preview.draw(avatar);
    for(const auto& resident : town_npcs::residents)
        residents.draw(preview,resident.type,(resident.x-left)*50.f,(resident.y-top)*50.f,font,resident.name);
    room.drawLabel(preview,font,clubhouse::spawnX); preview.display();
    assert(preview.getTexture().copyToImage().saveToFile("build/tests/town-residents-preview.png"));
    sf::RenderTexture frames; assert(frames.create(1200,540)); frames.clear({255,248,228});
    for(int row=0;row<3;++row) for(int direction=0;direction<4;++direction) {
        const auto& resident=town_npcs::residents[row];
        for(int phase=0;phase<2;++phase) {
            const auto rect=TownNpcRenderer::frame(resident.type,phase*.4f,static_cast<town_motion::Facing>(direction));
            const auto image=residents.texture(resident.type).copyToImage(); int opaque=0;
            assert(rect.left>=0 && rect.top>=0 && rect.left+rect.width<=static_cast<int>(image.getSize().x) && rect.top+rect.height<=static_cast<int>(image.getSize().y));
            for(int y=rect.top;y<rect.top+rect.height;++y) for(int x=rect.left;x<rect.left+rect.width;++x) if(image.getPixel(x,y).a) ++opaque;
            assert(opaque>100);
            residents.draw(frames,resident.type,direction*300.f+phase*125.f+45,row*180.f+80,font,resident.name,phase*.4f,static_cast<town_motion::Facing>(direction));
        }
    }
    frames.display(); assert(frames.getTexture().copyToImage().saveToFile("build/tests/town-animation-frames.png"));
    // Time snapshots of the same pure policy used on the server (not a connected game recording).
    std::array<town_behaviour::State,3> states; std::array<town_motion::Point,3> positions;
    for(int i=0;i<3;++i) { states[i].random=town_npcs::residents[i].id; positions[i]={static_cast<float>(town_npcs::residents[i].x),static_cast<float>(town_npcs::residents[i].y)}; }
    for(int time=0;time<=12000;time+=1000) {
        for(int i=0;i<3;++i) {
            const auto& resident=town_npcs::residents[i];
            const auto step=town_behaviour::tick(resident,static_cast<int>(positions[i].x),static_cast<int>(positions[i].y),states[i],time,[&](int x,int y){
                if(std::abs(x-clubhouse::spawnX)<=1 && std::abs(y-clubhouse::spawnY)<=1) return true;
                for(int j=0;j<3;++j) if(i!=j && std::abs(x-positions[j].x)<=1 && std::abs(y-positions[j].y)<=1) return true;
                return false;
            });
            positions[i]={static_cast<float>(step.x),static_cast<float>(step.y)};
        }
        if(time%4000) continue;
        preview.clear(); room.draw(preview,left,top); preview.draw(avatar);
        for(int i=0;i<3;++i) residents.draw(preview,town_npcs::residents[i].type,(positions[i].x-left)*50.f,(positions[i].y-top)*50.f,font,town_npcs::residents[i].name,time/1000.f);
        room.drawLabel(preview,font,clubhouse::spawnX); preview.display();
        assert(preview.getTexture().copyToImage().saveToFile("build/tests/town-walk-"+std::to_string(time)+".png"));
    }
    std::cout<<"21 sheets loaded; resident source rectangles/background masks and production renderer preview passed\n";
}
