#define main hamster_game_main
#include "../TP_2023192003_CLIENT/client_sample/client.cpp"
#undef main
#include <cassert>
#include <filesystem>

int main() {
    std::puts("wardrobe UI: begin"); std::fflush(stdout);
    const auto root=std::filesystem::current_path();
    std::filesystem::current_path(root/"TP_2023192003_CLIENT/client_sample");
    assert(gameFont().loadFromFile("neodgm.ttf") && hamsterCustomizer().load("assets/characters"));
    assert(clubhouseRenderer().load("assets/maps/clubhouse-sheet.png"));
    assert(wardrobePanel().loadIcon("assets/wardrobe/source/accessory-box-icons.png"));
    sf::RenderWindow window(sf::VideoMode(900,800),"Wardrobe test",sf::Style::None); window.setVisible(false);
    g_window=&window; WindowDisplay::resize(window);
    window.clear(); clubhouseRenderer().draw(window,world_regions::Id::Clubhouse,1,2); window.display();
    g_myid=avatar.id=1; avatar.setAppearance({}); moveAvatar({world_regions::Id::Clubhouse,11,12});
    sf::Event key{}; key.type=sf::Event::KeyPressed;
    auto press=[&](sf::Keyboard::Key code) { key.key.code=code; return handleWardrobeInput(key,window,{-1,-1}); };
    auto releaseW=[&]() { sf::Event e{}; e.type=sf::Event::KeyReleased; e.key.code=sf::Keyboard::W; handleWardrobeInput(e,window,{-1,-1}); };
    isChatMode=true; assert(!press(sf::Keyboard::W) && !wardrobeState.visible()); releaseW(); isChatMode=false;
    assert(press(sf::Keyboard::W) && wardrobeState.visible()); releaseW();
    window.clear(); clubhouseRenderer().draw(window,world_regions::Id::Clubhouse,1,2);
    drawWardrobeUi(window); window.display(); assert(!wardrobeBackdropNeeded);
    assert(press(sf::Keyboard::Right) && wardrobeState.draft.kind==1 && avatar.appearance.kind==0);
    press(sf::Keyboard::Tab); assert(wardrobePanel().category()==1); press(sf::Keyboard::Right);
    assert(wardrobeState.draft.fur==1);
    auto click=[&](sf::FloatRect bounds) {
        const auto pixel=window.mapCoordsToPixel({bounds.left+bounds.width/2,bounds.top+bounds.height/2});
        const auto logical=window.mapPixelToCoords(pixel);
        sf::Event e{}; e.type=sf::Event::MouseButtonPressed; e.mouseButton.button=sf::Mouse::Left;
        e.mouseButton.x=pixel.x; e.mouseButton.y=pixel.y; handleWardrobeInput(e,window,logical);
        e.type=sf::Event::MouseButtonReleased; handleWardrobeInput(e,window,logical);
    };
    click(WardrobePanel::tab(2)); click(wardrobePanel().item(2)); assert(wardrobeState.draft.eyes==2);
    click(WardrobePanel::tab(3)); click(wardrobePanel().item(2)); assert(wardrobeState.draft.scarf==2);
    assert(press(sf::Keyboard::Escape) && !wardrobeState.visible() && avatar.appearance==avatar_appearance::Value{});
    sf::TcpListener listener; assert(listener.listen(0,sf::IpAddress::LocalHost)==sf::Socket::Done);
    assert(s_socket.connect(sf::IpAddress::LocalHost,listener.getLocalPort(),sf::seconds(2))==sf::Socket::Done);
    sf::TcpSocket peer; assert(listener.accept(peer)==sf::Socket::Done); peer.setBlocking(false); s_socket.setBlocking(false);
    auto readRequest=[&]() {
        cs_packet_appearance packet{}; std::size_t total=0; sf::Clock limit;
        while(total<sizeof(packet) && limit.getElapsedTime()<sf::seconds(2)) {
            flushGamePackets(); std::size_t count=0;
            peer.receive(reinterpret_cast<char*>(&packet)+total,sizeof(packet)-total,count); total+=count;
        }
        assert(total==sizeof(packet) && packet.type==C2S_P_APPEARANCE); return packet;
    };
    auto receiveBytes=[&](std::size_t expected) {
        sf::Clock limit; std::size_t total=0;
        while(total<expected && limit.getElapsedTime()<sf::seconds(2)) {
            char buffer[BUF_SIZE]; std::size_t count=0; s_socket.receive(buffer,sizeof(buffer),count);
            if(count) { process_data(buffer,count); total+=count; }
        }
        assert(total==expected);
    };
    click(wardrobePanel().entry); assert(wardrobeState.visible()); press(sf::Keyboard::Right);
    press(sf::Keyboard::Enter); const auto request=readRequest(); assert(wardrobeState.pending());
    const auto before=wardrobeState.draft;
    press(sf::Keyboard::Right); press(sf::Keyboard::Escape); press(sf::Keyboard::Enter);
    assert(wardrobeState.pending() && wardrobeState.draft==before && avatar.appearance==avatar_appearance::Value{});
    std::uint8_t extra=0; std::size_t count=0; assert(peer.receive(&extra,1,count)==sf::Socket::NotReady && count==0);
    sc_packet_appearance_change change{sizeof(change),S2C_P_APPEARANCE_CHANGE,1,1,request.appearance};
    sc_packet_appearance_result result{sizeof(result),S2C_P_APPEARANCE_RESULT,request.requestId,wardrobe::Result::Saved,request.appearance};
    peer.setBlocking(true);
    assert(peer.send(&change,1)==sf::Socket::Done); receiveBytes(1); assert(saved_packet_size==1);
    assert(peer.send(reinterpret_cast<char*>(&change)+1,sizeof(change)-1)==sf::Socket::Done);
    assert(peer.send(&result,sizeof(result))==sf::Socket::Done); receiveBytes(sizeof(change)-1+sizeof(result));
    assert(wardrobeState.stage==wardrobe::Stage::Applied && avatar.appearance==request.appearance);
    sf::RenderTexture frame,contact; assert(frame.create(900,800) && contact.create(1800,2400));
    const wardrobe::Stage states[]={wardrobe::Stage::Editing,wardrobe::Stage::Pending,wardrobe::Stage::Applied,
        wardrobe::Stage::Rejected,wardrobe::Stage::TimedOut,wardrobe::Stage::Disconnected};
    for(int i=0;i<6;++i) {
        auto state=wardrobeState; state.stage=states[i]; state.connected=i!=5;
        frame.clear(); wardrobePanel().draw(frame,gameFont(),hamsterCustomizer(),state); frame.display();
        assert(frame.getTexture().copyToImage().saveToFile((root/("build/tests/wardrobe-state-"+std::to_string(i)+".png")).string()));
        sf::Sprite sprite(frame.getTexture()); sprite.setPosition((i%2)*900.f,(i/2)*800.f); contact.draw(sprite);
    }
    contact.display(); assert(contact.getTexture().copyToImage().saveToFile((root/"build/tests/wardrobe-states.png").string()));
    wardrobeState.stage=wardrobe::Stage::Editing;
    for(int i=0;i<4;++i) {
        click(WardrobePanel::tab(i)); frame.clear(); wardrobePanel().draw(frame,gameFont(),hamsterCustomizer(),wardrobeState); frame.display();
        assert(frame.getTexture().copyToImage().saveToFile((root/("build/tests/wardrobe-category-"+std::to_string(i)+".png")).string()));
    }
    for(auto size : {sf::Vector2u{1280,720},sf::Vector2u{800,900},sf::Vector2u{1920,1080}}) {
        window.setSize(size); WindowDisplay::resize(window);
        click(WardrobePanel::tab(1)); click(wardrobePanel().item(4)); assert(wardrobeState.draft.fur==4);
        const auto fit=display_layout::fit(size.x,size.y);
        sf::RenderTexture output; assert(output.create(size.x,size.y));
        sf::View view(sf::FloatRect(0,0,900,800)); view.setViewport({fit.left/size.x,fit.top/size.y,fit.width/size.x,fit.height/size.y});
        output.setView(view); output.clear({32,24,24}); wardrobePanel().draw(output,gameFont(),hamsterCustomizer(),wardrobeState); output.display();
        assert(output.getTexture().copyToImage().saveToFile((root/("build/tests/wardrobe-"+std::to_string(size.x)+"x"+std::to_string(size.y)+".png")).string()));
    }
    assert(wardrobeState.close());
    moveAvatar({world_regions::Id::SunnyPeak,10,10}); assert(!press(sf::Keyboard::W) && !wardrobeState.visible()); releaseW();
    moveAvatar({world_regions::Id::Clubhouse,11,12}); assert(press(sf::Keyboard::W)); releaseW();
    press(sf::Keyboard::Enter); readRequest(); gameConnectionLost(); assert(wardrobeState.stage==wardrobe::Stage::Disconnected);
    const auto disconnectedCategory=wardrobePanel().category(); const auto disconnectedDraft=wardrobeState.draft;
    press(sf::Keyboard::Tab); click(WardrobePanel::tab((disconnectedCategory+1)%4)); press(sf::Keyboard::Right);
    assert(wardrobePanel().category()==disconnectedCategory && wardrobeState.draft==disconnectedDraft);
    assert(press(sf::Keyboard::Escape) && !wardrobeState.visible()); window.close(); g_window=nullptr;
    std::filesystem::current_path(root);
    std::puts("wardrobe UI: W/chat/region/draft/tabs/items/cancel/disabled/real TCP apply/fragmented reply/resize mapping/disconnect + renderer states PASS");
}
