#define main production_client_main
#include "../TP_2023192003_CLIENT/client_sample/client.cpp"
#undef main
#include <cassert>
#include <filesystem>
int main() {
    const auto root=std::filesystem::current_path();std::filesystem::current_path(root/"TP_2023192003_CLIENT/client_sample");
    assert(gameFont().loadFromFile("neodgm.ttf"));assert(hamsterCustomizer().load("assets/characters"));
    assert(clubhouseRenderer().loadOutdoor("assets/maps/sunny-peak-sheet.png"));
    sf::RenderWindow window(sf::VideoMode(900,800),"Field map regression",sf::Style::None);window.setVisible(false);g_window=&window;
    WindowDisplay::resize(window);travelState={};wardrobeState={};moveAvatar({world_regions::Id::SunnyPeak,90,96});
    sf::Event e{};
    auto key=[&](sf::Keyboard::Key k) {e={};e.type=sf::Event::KeyPressed;e.key.code=k;return handleFieldMapInput(e,{-1,-1});};
    auto release=[&]() {e={};e.type=sf::Event::KeyReleased;e.key.code=sf::Keyboard::M;handleFieldMapInput(e,{-1,-1});};
    isChatMode=true;assert(!key(sf::Keyboard::M) && !fieldMapPanel().visible());release();isChatMode=false;
    travelState.status=world_travel::ClientStatus::Pending;assert(!key(sf::Keyboard::M));release();travelState={};
    wardrobeState.open({world_regions::Id::Clubhouse,4,4},{});assert(!key(sf::Keyboard::M));release();wardrobeState={};
    assert(key(sf::Keyboard::M) && fieldMapPanel().visible());assert(key(sf::Keyboard::M) && fieldMapPanel().visible());release();
    assert(key(sf::Keyboard::R) && !travelState.pending());assert(key(sf::Keyboard::Left));
    assert(key(sf::Keyboard::Escape) && !fieldMapPanel().visible());
    auto click=[&](sf::FloatRect bounds,bool releaseInside=true) {
        const auto pixel=window.mapCoordsToPixel({bounds.left+bounds.width/2,bounds.top+bounds.height/2});
        e={};e.type=sf::Event::MouseButtonPressed;e.mouseButton.button=sf::Mouse::Left;handleFieldMapInput(e,window.mapPixelToCoords(pixel));
        e.type=sf::Event::MouseButtonReleased;handleFieldMapInput(e,releaseInside?window.mapPixelToCoords(pixel):sf::Vector2f{-1,-1});
    };
    click(FieldMapPanel::entry,false);assert(!fieldMapPanel().visible());click(FieldMapPanel::entry);assert(fieldMapPanel().visible());
    moveAvatar({world_regions::Id::Clubhouse,4,4});assert(!fieldMapPanel().visible());assert(!key(sf::Keyboard::M));release();
    moveAvatar({world_regions::Id::SunnyPeak,90,96});assert(key(sf::Keyboard::M));release();gameConnectionLost();assert(!fieldMapPanel().visible());
    assert(!key(sf::Keyboard::M));release();travelState={};
    for(const auto size:{sf::Vector2u{900,800},sf::Vector2u{1280,720},sf::Vector2u{800,900}}) {
        window.setSize(size);WindowDisplay::resize(window);click(FieldMapPanel::entry);assert(fieldMapPanel().visible());
        sf::RenderTexture out;assert(out.create(size.x,size.y));out.clear({32,24,24});
        const auto fit=display_layout::fit(size.x,size.y);sf::View view(sf::FloatRect(0,0,900,800));
        view.setViewport({fit.left/size.x,fit.top/size.y,fit.width/size.x,fit.height/size.y});out.setView(view);
        clubhouseRenderer().draw(out,avatar.position.region,g_left_x,g_top_y);fieldMapPanel().draw(out,gameFont(),avatar.position);out.display();
        assert(out.getTexture().copyToImage().saveToFile((root/("build/tests/field-map-"+std::to_string(size.x)+"x"+std::to_string(size.y)+".png")).string()));
        click(FieldMapPanel::closeButton);assert(!fieldMapPanel().visible());
    }
    for(const auto p:{world_regions::Position{world_regions::Id::SunnyPeak,0,0},{world_regions::Id::SunnyPeak,255,255}}) {
        const auto point=FieldMapPanel::mapPoint(p);assert(FieldMapPanel::mapArea.contains(point));
    }
    assert(FieldMapPanel::mapPoint({world_regions::Id::Clubhouse,10,10})==sf::Vector2f(-1,-1));
    assert(FieldMapPanel::mapPoint({world_regions::Id::SunnyPeak,256,0})==sf::Vector2f(-1,-1));
    sf::RenderTexture entry;assert(entry.create(900,800));entry.clear();
    clubhouseRenderer().draw(entry,avatar.position.region,g_left_x,g_top_y);
    travelPanel.draw(entry,gameFont(),avatar.position.region,travelState);
    fieldMapPanel().drawEntry(entry,gameFont(),false);entry.display();
    assert(entry.getTexture().copyToImage().saveToFile((root/"build/tests/field-map-entry.png").string()));
    fieldMapPanel().reset();fieldDiscovery.reset();
    moveAvatar({world_regions::Id::SunnyPeak,32,32});assert(!fieldDiscovery.count()); // Local placement is not a server observation.
    sc_packet_move before{};before.size=sizeof(before);before.type=S2C_P_MOVE;before.id=g_myid;
    before.region=2;before.x=32;before.y=32;ProcessPacket(reinterpret_cast<char*>(&before));assert(!fieldDiscovery.count());
    sc_packet_avatar_info info{};info.size=sizeof(info);info.type=S2C_P_AVATAR_INFO;info.id=1;info.region=2;
    info.x=32;info.y=32;info.hp=info.max_hp=10;ProcessPacket(reinterpret_cast<char*>(&info));
    assert(fieldDiscovery.active() && fieldDiscovery.visited(1) && fieldDiscovery.count()==1);
    ProcessPacket(reinterpret_cast<char*>(&info));assert(fieldDiscovery.count()==1);
    sc_packet_move move{};move.size=sizeof(move);move.type=S2C_P_MOVE;move.id=2;move.region=2;move.x=90;move.y=96;
    ProcessPacket(reinterpret_cast<char*>(&move));assert(fieldDiscovery.count()==1); // Someone else's movement.
    move.id=1;ProcessPacket(reinterpret_cast<char*>(&move));assert(fieldDiscovery.visited(2) && fieldDiscovery.count()==2);
    move.region=1;move.x=4;move.y=4;ProcessPacket(reinterpret_cast<char*>(&move));assert(fieldDiscovery.count()==2);
    move.region=2;move.x=90;move.y=96;ProcessPacket(reinterpret_cast<char*>(&move));assert(fieldDiscovery.count()==2);
    for(int stage=0;stage<3;++stage) {
        field_discovery::Record sample;sample.start();
        if(stage>0) sample=fieldDiscovery;
        if(stage==2) for(const auto& p:field_landmarks::all) sample.observe({world_regions::Id::SunnyPeak,p.x,p.y});
        assert(sample.count()==(stage==0 ? 0u : stage==1 ? 2u : 5u));
        key(sf::Keyboard::M);release();
        sf::RenderTexture states;assert(states.create(900,800));states.clear();
        clubhouseRenderer().draw(states,avatar.position.region,g_left_x,g_top_y);
        fieldMapPanel().draw(states,gameFont(),avatar.position,sample);states.display();
        assert(states.getTexture().copyToImage().saveToFile((root/("build/tests/discovery-state-"+std::to_string(stage)+".png")).string()));
        for(const auto size:{sf::Vector2u{1280,720},sf::Vector2u{800,900}}) {
            sf::RenderTexture resized;assert(resized.create(size.x,size.y));resized.clear({32,24,24});
            const auto fit=display_layout::fit(size.x,size.y);sf::View v(sf::FloatRect(0,0,900,800));
            v.setViewport({fit.left/size.x,fit.top/size.y,fit.width/size.x,fit.height/size.y});resized.setView(v);
            clubhouseRenderer().draw(resized,avatar.position.region,g_left_x,g_top_y);
            fieldMapPanel().draw(resized,gameFont(),avatar.position,sample);resized.display();
            assert(resized.getTexture().copyToImage().saveToFile((root/("build/tests/discovery-state-"+std::to_string(stage)+"-"+std::to_string(size.x)+"x"+std::to_string(size.y)+".png")).string()));
        }
        key(sf::Keyboard::Escape);
    }
    gameConnectionLost();assert(!fieldDiscovery.active() && !fieldDiscovery.count());
    // A new avatar on a new connection starts from zero, then records only its arrival.
    info.x=240;info.y=224;ProcessPacket(reinterpret_cast<char*>(&info));assert(fieldDiscovery.count()==1 && fieldDiscovery.visited(5));
    sc_packet_enter arrival{};arrival.size=sizeof(arrival);arrival.type=S2C_P_ENTER;arrival.id=1;arrival.region=2;arrival.x=192;arrival.y=192;
    ProcessPacket(reinterpret_cast<char*>(&arrival));assert(fieldDiscovery.count()==2 && fieldDiscovery.visited(4));
    beginLoginConnection(0);assert(!fieldDiscovery.active() && !fieldDiscovery.count()); // Local bounded failed login, no DB/server launched.
    gameConnectionLost();fieldMapPanel().reset();window.close();g_window=nullptr;std::filesystem::current_path(root);
    std::puts("Field map production input: M/repeat/chat/pending/wardrobe/modal/region/disconnect/click/resize/markers + terrain renderer PASS");
}
