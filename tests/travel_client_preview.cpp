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
    loginFlow.finishCredentials(); loginFlow.connect(); loginFlow.accept();
    avatar.id=g_myid=1; avatar.get_sprite().setTexture(hamsterCustomizer().worldTexture(0,0,0,0));
    moveAvatar({world_regions::Id::Clubhouse,11,12});
    sf::TcpListener listener; assert(listener.listen(0,sf::IpAddress::LocalHost)==sf::Socket::Done);
    assert(s_socket.connect(sf::IpAddress::LocalHost,listener.getLocalPort(),sf::seconds(2))==sf::Socket::Done);
    sf::TcpSocket peer; assert(listener.accept(peer)==sf::Socket::Done);
    s_socket.setBlocking(false); peer.setBlocking(false);
    auto receiveRequest=[&]() {
        cs_packet_teleport request{}; std::size_t total=0; sf::Clock limit;
        while(total<sizeof(request) && limit.getElapsedTime()<sf::seconds(2)) {
            flushGamePackets(); std::size_t got=0;
            peer.receive(reinterpret_cast<char*>(&request)+total,sizeof(request)-total,got); total+=got;
        }
        assert(total==sizeof(request) && is_valid_client_packet(request.type,request.size)); return request;
    };
    sf::Event key{}; key.type=sf::Event::KeyPressed; key.key.code=sf::Keyboard::R;
    sf::Event released=key; released.type=sf::Event::KeyReleased;
    assert(!handleTravelInput(key,{-1,-1},true) && !travelState.pending());
    handleTravelInput(released,{-1,-1},true);
    assert(handleTravelInput(key,{-1,-1},false));
    const auto first=receiveRequest();
    assert(first.action==world_travel::Action::EnterField && first.requestId==1);
    assert(avatar.position.region==world_regions::Id::Clubhouse); // No optimistic relocation.
    handleTravelInput(key,{-1,-1},false);
    std::uint8_t unexpected; std::size_t bytes=0;
    assert(peer.receive(&unexpected,1,bytes)==sf::Socket::NotReady && bytes==0);
    auto reply=[&](std::uint32_t id,world_travel::Result result) {
        sc_packet_travel_result response{sizeof(response),S2C_P_TRAVEL_RESULT,id,result};
        peer.setBlocking(true);
        assert(peer.send(&response,1)==sf::Socket::Done);
        assert(peer.send(reinterpret_cast<char*>(&response)+1,sizeof(response)-1)==sf::Socket::Done);
        std::size_t total=0; sf::Clock limit;
        while(total<sizeof(response) && limit.getElapsedTime()<sf::seconds(2)) {
            char buffer[BUF_SIZE]; std::size_t count=0;
            const auto status=s_socket.receive(buffer,sizeof(buffer),count);
            assert(status!=sf::Socket::Error && status!=sf::Socket::Disconnected);
            if(count) { process_data(buffer,count); total+=count; }
        }
        assert(total==sizeof(response)); peer.setBlocking(false);
    };
    reply(999,world_travel::Result::Success); assert(travelState.pending());
    sc_packet_move move{}; move.size=sizeof(move); move.type=S2C_P_MOVE; move.id=1;
    move.region=2; move.x=10; move.y=10;
    process_data(reinterpret_cast<char*>(&move),sizeof(move));
    reply(1,world_travel::Result::Success); assert(!travelState.pending());
    handleTravelInput(key,{-1,-1},false); assert(!travelState.pending()); // Held R must not bounce back.
    handleTravelInput(released,{-1,-1},false);
    assert(avatar.position.region==world_regions::Id::SunnyPeak);
    sf::Event mouse{}; mouse.type=sf::Event::MouseButtonPressed; mouse.mouseButton.button=sf::Mouse::Left;
    assert(!handleTravelInput(mouse,{700,30},false));
    mouse.type=sf::Event::MouseButtonReleased;
    assert(handleTravelInput(mouse,{700,30},false));
    auto second=receiveRequest(); assert(second.requestId==2 && second.action==world_travel::Action::ReturnHome);
    reply(2,world_travel::Result::Blocked);
    assert(travelState.status==world_travel::ClientStatus::Rejected);
    assert(avatar.position.region==world_regions::Id::SunnyPeak);
    handleTravelInput(key,{-1,-1},false); receiveRequest();
    travelState.tick(travelState.deadline); assert(travelState.status==world_travel::ClientStatus::TimedOut);
    reply(3,world_travel::Result::Success); assert(travelState.status==world_travel::ClientStatus::TimedOut);

    sf::RenderTexture scene; assert(scene.create(900,800));
    sf::Sprite hamster(hamsterCustomizer().worldTexture(0,0,0,0),{0,0,48,48});
    auto render=[&](world_regions::Id region,const world_travel::ClientState& state,sf::Vector2f hover,bool chat) {
        scene.clear(); clubhouseRenderer().draw(scene,region,1,2);
        world_character::place(hamster,450,400); scene.draw(hamster);
        clubhouseRenderer().drawLabel(scene,gameFont(),region);
        travelPanel.draw(scene,gameFont(),region,state,hover,chat); scene.display();
    };
    sf::RenderTexture contact; assert(contact.create(1800,1600)); contact.clear();
    const world_travel::ClientStatus states[]={world_travel::ClientStatus::Ready,world_travel::ClientStatus::Pending,
        world_travel::ClientStatus::TimedOut,world_travel::ClientStatus::Disconnected};
    for(int i=0;i<4;++i) {
        world_travel::ClientState state; state.status=states[i];
        render(i==0 ? world_regions::Id::Clubhouse : world_regions::Id::SunnyPeak,state,{-1,-1},false);
        sf::Sprite sprite(scene.getTexture()); sprite.setPosition((i%2)*900.f,(i/2)*800.f); contact.draw(sprite);
    }
    contact.display(); assert(contact.getTexture().copyToImage().saveToFile((root/"build/tests/travel-states.png").string()));
    // Actual display-view letterbox policy; target size changes, not the logical UI.
    for(const auto size : {sf::Vector2u{1280,720},sf::Vector2u{1920,1080},sf::Vector2u{800,900}}) {
        world_travel::ClientState state;
        render(world_regions::Id::SunnyPeak,state,{700,30},false);
        sf::RenderTexture output; assert(output.create(size.x,size.y));
        sf::View view(sf::FloatRect(0,0,900,800));
        const auto fit=display_layout::fit(size.x,size.y);
        view.setViewport({fit.left/size.x,fit.top/size.y,fit.width/size.x,fit.height/size.y});
        const sf::Vector2f click{fit.left+700*fit.scale,fit.top+30*fit.scale};
        const sf::Vector2f logical{(click.x-fit.left)/fit.scale,(click.y-fit.top)/fit.scale};
        assert(travelPanel.bounds.contains(logical));
        assert(!travelPanel.bounds.contains({-10,-10}));
        output.setView(view); output.clear({32,24,24}); output.draw(sf::Sprite(scene.getTexture())); output.display();
        const auto path=root/("build/tests/travel-"+std::to_string(size.x)+"x"+std::to_string(size.y)+".png");
        assert(output.getTexture().copyToImage().saveToFile(path.string()));
    }
    // Remaining states are rendered for visual inspection of text fit.
    contact.clear();
    for(int i=0;i<4;++i) {
        world_travel::ClientState state;
        if(i==0) state.status=world_travel::ClientStatus::Arrived;
        if(i==1) state.status=world_travel::ClientStatus::Rejected;
        travelPanel.pressed=i==2;
        render(world_regions::Id::SunnyPeak,state,{700,30},i==3);
        sf::Sprite sprite(scene.getTexture()); sprite.setPosition((i%2)*900.f,(i/2)*800.f); contact.draw(sprite);
    }
    contact.display(); assert(contact.getTexture().copyToImage().saveToFile((root/"build/tests/travel-extra-states.png").string()));
    travelPanel.pressed=false;
    gameConnectionLost(); assert(travelState.status==world_travel::ClientStatus::Disconnected && !gameOutbox.queued());
    assert(handleTravelInput(key,{-1,-1},false));
    key.key.code=sf::Keyboard::Escape; assert(!handleTravelInput(key,{-1,-1},false));
    std::filesystem::current_path(root);
    std::puts("native travel: R/chat/click/TCP request/pending/id/authoritative MOVE/reject/timeout/disconnect/render PASS");
}
