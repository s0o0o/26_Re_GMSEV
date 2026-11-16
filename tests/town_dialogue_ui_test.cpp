#define main production_client_main
#include "../TP_2023192003_CLIENT/client_sample/client.cpp"
#undef main
#include <cassert>
#include <filesystem>
int main() {
    const auto root=std::filesystem::current_path();std::filesystem::current_path(root/"TP_2023192003_CLIENT/client_sample");
    assert(gameFont().loadFromFile("neodgm.ttf") && townNpcRenderer().load("assets/npcs/source"));
    assert(hamsterCustomizer().load("assets/characters"));
    assert(clubhouseRenderer().load("assets/maps/clubhouse-sheet.png"));
    sf::RenderWindow window(sf::VideoMode(900,800),"Town dialogue regression",sf::Style::None);window.setVisible(false);g_window=&window;
    WindowDisplay::resize(window);travelState={};wardrobeState={};fieldMapPanel().reset();
    auto enter=[&](const town_npcs::Resident& r) {
        sc_packet_enter p{};p.size=sizeof(p);p.type=S2C_P_ENTER;p.id=r.id;p.monType=r.type;p.region=1;p.x=r.x;p.y=r.y;
        strcpy_s(p.name,r.name);p.hp=p.max_hp=1;ProcessPacket(reinterpret_cast<char*>(&p));
    };
    sf::Event e{};
    auto key=[&](sf::Keyboard::Key k){e={};e.type=sf::Event::KeyPressed;e.key.code=k;return handleTownDialogueInput(e,{-1,-1});};
    auto release=[&](sf::Keyboard::Key k){e={};e.type=sf::Event::KeyReleased;e.key.code=k;handleTownDialogueInput(e,{-1,-1});};
    enter(town_npcs::residents[0]);moveAvatar({world_regions::Id::Clubhouse,12,10});
    isChatMode=true;assert(!key(sf::Keyboard::E));release(sf::Keyboard::E);isChatMode=false;
    travelState.status=world_travel::ClientStatus::Pending;assert(!key(sf::Keyboard::E));release(sf::Keyboard::E);travelState={};
    assert(wardrobeState.open(avatar.position,{}));assert(!key(sf::Keyboard::E));release(sf::Keyboard::E);wardrobeState={};
    assert(key(sf::Keyboard::E) && townDialoguePanel().visible());assert(key(sf::Keyboard::E) && townDialoguePanel().state.page==0);
    release(sf::Keyboard::E);assert(key(sf::Keyboard::R) && !travelState.pending());
    assert(key(sf::Keyboard::Enter) && townDialoguePanel().state.page==1);key(sf::Keyboard::Enter);assert(townDialoguePanel().state.page==1);
    release(sf::Keyboard::Enter);assert(key(sf::Keyboard::E) && !townDialoguePanel().visible());release(sf::Keyboard::E);
    auto click=[&](sf::FloatRect r) {const auto pixel=window.mapCoordsToPixel({r.left+r.width/2,r.top+r.height/2});
        e={};e.type=sf::Event::MouseButtonPressed;e.mouseButton.button=sf::Mouse::Left;handleTownDialogueInput(e,window.mapPixelToCoords(pixel));
        e.type=sf::Event::MouseButtonReleased;handleTownDialogueInput(e,window.mapPixelToCoords(pixel));};
    click(TownDialoguePanel::prompt);assert(townDialoguePanel().visible());click(TownDialoguePanel::nextButton);assert(townDialoguePanel().state.page==1);
    click(TownDialoguePanel::closeButton);assert(!townDialoguePanel().visible());
    key(sf::Keyboard::E);release(sf::Keyboard::E);
    sc_packet_move moved{};moved.size=sizeof(moved);moved.type=S2C_P_MOVE;moved.id=town_npcs::baseId;moved.region=1;moved.x=14;moved.y=9;
    ProcessPacket(reinterpret_cast<char*>(&moved));release(sf::Keyboard::E);assert(!townDialoguePanel().visible());
    enter(town_npcs::residents[0]);key(sf::Keyboard::E);release(sf::Keyboard::E);
    sc_packet_leave left{sizeof(left),S2C_P_LEAVE,town_npcs::baseId};ProcessPacket(reinterpret_cast<char*>(&left));
    release(sf::Keyboard::E);assert(!townDialoguePanel().visible());
    for(const auto& resident:town_npcs::residents) {
        players.clear();enter(resident);moveAvatar({world_regions::Id::Clubhouse,resident.x,resident.y+1});
        key(sf::Keyboard::E);release(sf::Keyboard::E);assert(townDialoguePanel().visible());
        for(int page=0;page<2;++page) {
            townDialoguePanel().state.page=page;
            for(const auto size:{sf::Vector2u{900,800},sf::Vector2u{1280,720},sf::Vector2u{800,900}}) {
                window.setSize(size);WindowDisplay::resize(window);sf::RenderTexture out;assert(out.create(size.x,size.y));out.clear({32,24,24});
                const auto fit=display_layout::fit(size.x,size.y);sf::View view(sf::FloatRect(0,0,900,800));
                view.setViewport({fit.left/size.x,fit.top/size.y,fit.width/size.x,fit.height/size.y});out.setView(view);
                clubhouseRenderer().draw(out,avatar.position.region,g_left_x,g_top_y);
                for(auto& p:players) {
                    p.second.draw(); // Also execute the production OBJECT world path on its window.
                    const auto position=p.second.townMotion.sample(combatClock.getElapsedTime().asSeconds());
                    townNpcRenderer().draw(out,p.second._monType,(position.x-g_left_x)*50.f,(position.y-g_top_y)*50.f,
                        gameFont(),p.second.name,page*.4f,p.second.townMotion.facing);
                }
                sf::Sprite actor(hamsterCustomizer().worldTexture(0,0,0,0),{48,0,48,48});
                world_character::place(actor,(avatar.position.x-g_left_x)*50.f,(avatar.position.y-g_top_y)*50.f);out.draw(actor);
                townDialoguePanel().draw(out,gameFont(),townNpcRenderer(),page*.4f);out.display();
                assert(out.getTexture().copyToImage().saveToFile((root/("build/tests/dialogue-"+std::to_string(resident.type)+"-"+std::to_string(page)+"-"+std::to_string(size.x)+"x"+std::to_string(size.y)+".png")).string()));
                click(TownDialoguePanel::closeButton);assert(!townDialoguePanel().visible());click(TownDialoguePanel::prompt);assert(townDialoguePanel().visible());townDialoguePanel().state.page=page;
            }
        }
        key(sf::Keyboard::Escape);assert(!townDialoguePanel().visible());
    }
    key(sf::Keyboard::E);release(sf::Keyboard::E);moveAvatar({world_regions::Id::SunnyPeak,10,10});assert(!townDialoguePanel().visible());
    moveAvatar({world_regions::Id::Clubhouse,14,15});key(sf::Keyboard::E);release(sf::Keyboard::E);gameConnectionLost();assert(!townDialoguePanel().visible());
    assert(!key(sf::Keyboard::E));window.close();g_window=nullptr;std::filesystem::current_path(root);
    std::puts("Town dialogue production E/repeat/pages/click/chat/modals/live MOVE+LEAVE/region/disconnect/3 NPCs x 3 sizes PASS");
}
