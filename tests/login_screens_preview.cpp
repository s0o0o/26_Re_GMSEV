// Render the actual client's login/customization screens without connecting to a server.
#define main hamster_game_main
#include "../TP_2023192003_CLIENT/client_sample/client.cpp"
#undef main
#include <cassert>
#include <filesystem>
#include "../shared/field_content.hpp"

int main() {
    std::cout << "Preparing login screen assets" << std::endl;
    const auto root = std::filesystem::current_path();
    std::filesystem::current_path(root / "TP_2023192003_CLIENT/client_sample");
    assert(gameFont().loadFromFile("neodgm.ttf"));
    for (const auto& value : {sf::String("Bijou"), sf::String("Connecting..."), sf::String(L"\uD584\uCC0C")}) {
        sf::Text centered(value,gameFont(),22);
        const sf::FloatRect area(190,610,230,52);
        centerText(centered,area);
        const auto bounds = centered.getGlobalBounds();
        assert(std::abs(bounds.left+bounds.width/2-(area.left+area.width/2)) < 0.01f);
        assert(std::abs(bounds.top+bounds.height/2-(area.top+area.height/2)) < 0.01f);
    }
    initStartScreenInit();
    assert(hamsterCustomizer().load("assets/characters"));
    assert(hamsterCustomizer().setBackdrop(*startMainTx));
    assert(clubhouseRenderer().load("assets/maps/clubhouse-sheet.png"));
    assert(clubhouseRenderer().loadOutdoor("assets/maps/sunny-peak-sheet.png"));
    std::cout << "Backdrop prepared" << std::endl;
    sf::RenderTexture frame;
    assert(frame.create(900,800));
    frame.clear();
    clubhouseRenderer().draw(frame,3,2);
    sf::Sprite resident(hamsterCustomizer().texture(0,0,0,0),{48,0,48,48});
    resident.setOrigin(24,44); resident.setScale(2,2);
    resident.setPosition((clubhouse::spawnX-3)*50.f+25,(clubhouse::spawnY-2)*50.f+45);
    frame.draw(resident);
    frame.display();
    assert(frame.getTexture().copyToImage().saveToFile((root / "build/tests/clubhouse-map.png").string()));
    frame.clear();
    const int outsideX=world_layout::fieldX+10,outsideY=10;
    const int outsideLeft=world_layout::cameraX(outsideX),outsideTop=world_layout::cameraY(outsideY);
    clubhouseRenderer().draw(frame,outsideLeft,outsideTop);
    int pickupTiles[16][18]{};
    for (auto point : field_content::seeds) {
        int px=point.x-outsideLeft,py=point.y-outsideTop;
        if(px>=0 && px<18 && py>=0 && py<16) pickupTiles[py][px]=4;
    }
    clubhouseRenderer().drawPickups(frame,&pickupTiles[0][0],18,16);
    resident.setPosition((outsideX-outsideLeft)*50.f+25,(outsideY-outsideTop)*50.f+45);
    frame.draw(resident);
    clubhouseRenderer().drawLabel(frame,gameFont(),outsideX);
    frame.display();
    assert(frame.getTexture().copyToImage().saveToFile((root / "build/tests/sunny-peak-map.png").string()));
    sf::Texture enemyTexture;
    assert(enemyTexture.loadFromFile("monHam.png"));
    monHamster=&enemyTexture;
    g_myid=1; avatar._seedNum=22;
    sc_packet_enter enemy{};
    enemy.size=sizeof(enemy); enemy.type=S2C_P_ENTER; enemy.id=MAX_USER+1;
    enemy.monType=1; enemy.x=42; enemy.y=8; enemy.hp=3; enemy.max_hp=5;
    std::strcpy(enemy.name,"HAM");
    process_data(reinterpret_cast<char*>(&enemy),sizeof(enemy));
    assert(combatHud().hp(enemy.id) == 3);
    sc_packet_attack_success damage{};
    damage.size=sizeof(damage); damage.type=S2C_P_ATTACK_PtoN; damage.targetId=enemy.id;
    damage.attackerId=99; damage.seedNum=100; damage.targetHp=2;
    process_data(reinterpret_cast<char*>(&damage),sizeof(damage));
    assert(avatar._seedNum == 22 && combatHud().hp(enemy.id) == 2);
    sc_packet_attack_feedback notice{};
    notice.size=sizeof(notice); notice.type=S2C_P_ATTACK_FEEDBACK;
    notice.reason=static_cast<std::uint8_t>(AttackFeedback::NoSeeds);
    process_data(reinterpret_cast<char*>(&notice),sizeof(notice));
    frame.clear();
    clubhouseRenderer().draw(frame,outsideLeft,outsideTop);
    sf::Sprite enemySprite(enemyTexture,{monTextureWidth,0,monTextureWidth,monTextureWidth});
    enemySprite.setPosition(420,350); frame.draw(enemySprite);
    combatHud().drawMonster(frame,gameFont(),enemy.id,420,350,combatClock.getElapsedTime().asSeconds());
    combatHud().drawMessage(frame,gameFont(),combatClock.getElapsedTime().asSeconds());
    frame.display();
    assert(frame.getTexture().copyToImage().saveToFile((root / "build/tests/combat-feedback.png").string()));
    damage.attackerId=1; damage.seedNum=21;
    process_data(reinterpret_cast<char*>(&damage),sizeof(damage));
    assert(avatar._seedNum == 21);
    sc_packet_monster_die dead{};
    dead.size=sizeof(dead); dead.type=S2C_P_MONSTER_DIE; dead.id=enemy.id;
    nearMonCnt=1; nearMonsters[0]={enemy.id,2,1};
    process_data(reinterpret_cast<char*>(&dead),sizeof(dead));
    assert(players.count(enemy.id) == 0 && combatHud().hp(enemy.id) == -1 && nearMonCnt == 0);
    sc_packet_monster_near lateList{};
    lateList.size=static_cast<std::uint8_t>(offsetof(sc_packet_monster_near,monsters)+sizeof(nearMonsterInfo));
    lateList.type=S2C_P_MONSTER_NEAR; lateList.monCount=1; lateList.monsters[0]={enemy.id,0,1};
    process_data(reinterpret_cast<char*>(&lateList),lateList.size);
    assert(nearMonCnt == 0);
    sc_packet_move stale{};
    stale.size=sizeof(stale); stale.type=S2C_P_MOVE; stale.id=enemy.id; stale.x=42; stale.y=8;
    process_data(reinterpret_cast<char*>(&stale),sizeof(stale));
    assert(players.count(enemy.id) == 0);
    id_input = 321;
    inputNickname = sf::String(L"\uD584\uCC0C");
    inputAccountId = "321";
    choosingNickname = true;
    frame.clear();
    drawStartScreen(frame,inputNickname);
    frame.display();
    assert(frame.getTexture().copyToImage().saveToFile((root / "build/tests/login-credentials.png").string()));
    std::cout << "Credentials rendered" << std::endl;
    assert(loginFlow.finishCredentials());
    frame.clear();
    drawCustomizationScreen(frame);
    frame.display();
    assert(frame.getTexture().copyToImage().saveToFile((root / "build/tests/customization-flow.png").string()));
    std::cout << "Customization rendered" << std::endl;
    sf::Event choose{};
    choose.type = sf::Event::KeyPressed;
    choose.key.code = sf::Keyboard::F5;
    sf::RenderWindow inputWindow;
    hamsterCustomizer().handleEvent(choose,inputWindow);
    assert(hamsterCustomizer().selectedCharacter() == 1);
    frame.clear();
    drawCustomizationScreen(frame);
    frame.display();
    assert(frame.getTexture().copyToImage().saveToFile((root / "build/tests/bijou-login-flow.png").string()));
    assert(loginFlow.connect());
    loadingPhase = LoadingPhase::Waiting;
    frame.clear();
    drawCustomizationScreen(frame);
    frame.display();
    assert(frame.getTexture().copyToImage().saveToFile((root / "build/tests/login-connecting.png").string()));
    for (int i=0;i<4;++i) {
        frame.clear();
        hamsterCustomizer().drawBackdrop(frame);
        drawLoadingScreen(frame,gameFont(),hamsterCustomizer().texture(0,0,0,1),inputNickname,
            static_cast<LoadingPhase>(i),i*.3f);
        frame.display();
        assert(frame.getTexture().copyToImage().saveToFile((root / ("build/tests/loading-phase-"+std::to_string(i)+".png")).string()));
    }
    failLogin("Offline test: retry is available.");
    assert(loginFlow.stage() == GAMESTATE::CustomizeScreen);
    assert(hamsterCustomizer().selectedCharacter() == 1);
    assert(inputNickname == sf::String(L"\uD584\uCC0C"));
    assert(s_socket.getRemotePort() == 0);
    assert(!gameResourcesReady);
    // Exercise the actual client connection/send/framing/state path, without a DB.
    sf::TcpListener listener;
    assert(listener.listen(0,sf::IpAddress::LocalHost) == sf::Socket::Done);
    gameResourcesReady = true; // Rendering resources above suffice for packet handling.
    const auto connectPeer = [&](sf::TcpSocket& peer) {
        loginFlow = LoginFlow{};
        assert(loginFlow.finishCredentials());
        assert(loginFlow.connect());
        beginLoginConnection(listener.getLocalPort());
        assert(loginFlow.stage() == GAMESTATE::ConnectingScreen);
        assert(listener.accept(peer) == sf::Socket::Done);
        peer.setBlocking(false);
        cs_packet_login request{};
        std::size_t total = 0;
        sf::Clock limit;
        while (total < sizeof(request) && limit.getElapsedTime() < sf::seconds(2)) {
            pumpLogin();
            std::size_t count = 0;
            peer.receive(reinterpret_cast<char*>(&request)+total,sizeof(request)-total,count);
            total += count;
            sf::sleep(sf::milliseconds(1));
        }
        assert(total == sizeof(request));
        assert(is_valid_client_packet(request.type,request.size));
        assert(request.inputId == 321 && request.character_kind == 1);
        assert(std::string(request.name) == toUtf8(inputNickname));
        peer.setBlocking(true);
    };
    const auto waitForReply = [&]() {
        sf::Clock limit;
        while (loginFlow.stage() == GAMESTATE::ConnectingScreen && limit.getElapsedTime() < sf::seconds(2)) {
            pumpLogin();
            sf::sleep(sf::milliseconds(1));
        }
        assert(loginFlow.stage() != GAMESTATE::ConnectingScreen);
    };
    for (bool fragment : {false,true}) {
        sf::TcpSocket peer;
        connectPeer(peer);
        sc_packet_avatar_info info{};
        info.size = sizeof(info); info.type = S2C_P_AVATAR_INFO;
        info.id = 1; info.x = 10; info.y = 20; info.character_kind = 1;
        sc_packet_avatar_login_ok ok{};
        ok.size = sizeof(ok); ok.type = S2C_P_AVATAR_LOGIN_OK; ok.id = 1;
        std::vector<char> reply(sizeof(info)+sizeof(ok));
        std::memcpy(reply.data(),&info,sizeof(info));
        std::memcpy(reply.data()+sizeof(info),&ok,sizeof(ok));
        if (fragment) {
            assert(peer.send(reply.data(),1) == sf::Socket::Done);
            sf::Clock limit;
            while (loginBytesReceived == 0 && limit.getElapsedTime() < sf::seconds(2)) { pumpLogin(); sf::sleep(sf::milliseconds(1)); }
            assert(saved_packet_size == 1);
        }
        const std::size_t offset = fragment ? 1 : 0;
        assert(peer.send(reply.data()+offset,reply.size()-offset) == sf::Socket::Done);
        waitForReply();
        assert(loginFlow.stage() == GAMESTATE::GameScreen);
        assert(g_myid == 1 && g_left_x == world_layout::cameraX(10) && g_top_y == world_layout::cameraY(20));
        sc_packet_move move{};
        move.size=sizeof(move); move.type=S2C_P_MOVE; move.id=1;
        move.x=42; move.y=10;
        process_data(reinterpret_cast<char*>(&move),sizeof(move));
        assert(avatar.m_x == 42 && avatar.m_y == 10 && g_left_x == world_layout::cameraX(42));
        move.x=10; move.y=14;
        process_data(reinterpret_cast<char*>(&move),sizeof(move));
        assert(avatar.m_x == 10 && avatar.m_y == 14 && g_left_x == world_layout::cameraX(10));
        sc_packet_mapInfo map{};
        map.size=sizeof(map); map.type=S2C_P_MAPINFO;
        map.left_x=world_layout::cameraX(10); map.top_y=world_layout::cameraY(14);
        map.tiles[0][0]=4;
        process_data(reinterpret_cast<char*>(&map),sizeof(map));
        assert(g_left_x == world_layout::cameraX(10) && g_top_y == world_layout::cameraY(14));
        assert(clientMapData[0][0] == 4);
    }
    {
        sf::TcpSocket peer;
        connectPeer(peer);
        sc_packet_login_fail rejected{};
        rejected.size = sizeof(rejected); rejected.type = S2C_P_LOGIN_FAIL;
        assert(peer.send(&rejected,sizeof(rejected)) == sf::Socket::Done);
        waitForReply();
        assert(loginStatus.find("rejected") != std::string::npos);
    }
    {
        sf::TcpSocket peer;
        connectPeer(peer);
        const char malformed[] = {2,127};
        assert(peer.send(malformed,sizeof(malformed)) == sf::Socket::Done);
        waitForReply();
        assert(loginStatus.find("protocol mismatch") != std::string::npos);
    }
    {
        sf::TcpSocket peer;
        connectPeer(peer);
        pumpLogin(sf::Time::Zero);
        assert(loginFlow.stage() == GAMESTATE::CustomizeScreen);
        assert(loginStatus.find("No login response") != std::string::npos);
    }
    {
        sf::TcpSocket peer;
        connectPeer(peer);
        peer.disconnect();
        waitForReply();
        assert(loginStatus.find("disconnected") != std::string::npos);
    }
    {
        sf::TcpSocket peer;
        connectPeer(peer);
        const char header = sizeof(sc_packet_avatar_info);
        assert(peer.send(&header,1) == sf::Socket::Done);
        sf::Clock limit;
        while (loginBytesReceived == 0 && limit.getElapsedTime() < sf::seconds(2)) { pumpLogin(); sf::sleep(sf::milliseconds(1)); }
        assert(saved_packet_size == 1);
        pumpLogin(sf::Time::Zero);
        assert(loginStatus.find("Incomplete login response") != std::string::npos);
    }
    s_socket.disconnect();
    std::cout << "Loopback login success/failure/framing checks passed" << std::endl;
    std::filesystem::current_path(root);
    std::cout << "Login screen previews passed" << std::endl;
}
