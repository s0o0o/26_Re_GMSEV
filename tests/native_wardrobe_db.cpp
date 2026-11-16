// Production client/event/render path against the actual server and existing test account.
#define main hamster_game_main
#include "../TP_2023192003_CLIENT/client_sample/client.cpp"
#undef main
#include <cassert>
#include <filesystem>
int main(int argc,char** argv) {
    assert(argc==2); const bool save=std::strcmp(argv[1],"SaveA")==0;
    assert(save || std::strcmp(argv[1],"RestoreA")==0);
    const auto root=std::filesystem::current_path();
    std::filesystem::current_path(root/"TP_2023192003_CLIENT/client_sample");
    assert(gameFont().loadFromFile("neodgm.ttf") && hamsterCustomizer().load("assets/characters"));
    assert(clubhouseRenderer().load("assets/maps/clubhouse-sheet.png") && clubhouseRenderer().loadOutdoor("assets/maps/sunny-peak-sheet.png"));
    assert(townNpcRenderer().load("assets/npcs/source"));
    sf::RenderWindow window(sf::VideoMode(900,800),"Live wardrobe DB test",sf::Style::None); window.setVisible(false);
    g_window=&window; WindowDisplay::resize(window);
    id_input=900001299; inputNickname=L"WardrobeDB";
    assert(loginFlow.finishCredentials() && loginFlow.connect()); beginLoginConnection();
    sf::Clock deadline;
    while(loginFlow.stage()==GAMESTATE::ConnectingScreen && deadline.getElapsedTime()<sf::seconds(10)) { pumpLogin(); sf::sleep(sf::milliseconds(1)); }
    assert(loginFlow.stage()==GAMESTATE::GameScreen);
    const avatar_appearance::Value a{1,4,2,2},b{0,1,1,1};
    assert(avatar.appearance==a || avatar.appearance==b);
    if(!save) assert(avatar.appearance==a); // Must restore the value saved before the server restart.
    auto frame=[&]() { window.clear(); client_main(); window.display(); assert(wardrobeState.connected); };
    if(avatar.position.region!=world_regions::Id::Clubhouse) {
        cs_packet_teleport home{sizeof(home),C2S_P_TELEPORT,world_travel::Action::ReturnHome,1}; send_packet(&home);
        deadline.restart();
        while(avatar.position.region!=world_regions::Id::Clubhouse && deadline.getElapsedTime()<sf::seconds(10)) frame();
    }
    assert(avatar.position.region==world_regions::Id::Clubhouse);
    frame();
    if(save) {
        window.clear(); client_main(); sf::Texture entry;
        assert(entry.create(900,800)); entry.update(window);
        assert(entry.copyToImage().saveToFile((root/"build/tests/wardrobe-entry-live.png").string())); window.display();
    }
    sf::Event key{}; key.type=sf::Event::KeyPressed; key.key.code=sf::Keyboard::W;
    assert(handleWardrobeInput(key,window,{-1,-1}) && wardrobeState.visible()); frame();
    if(save) {
        assert(wardrobeState.edit(a)); key.key.code=sf::Keyboard::Enter; assert(handleWardrobeInput(key,window,{-1,-1}));
        deadline.restart();
        while(wardrobeState.pending() && deadline.getElapsedTime()<sf::seconds(10)) frame();
        assert(wardrobeState.stage==wardrobe::Stage::Applied && avatar.appearance==a);
    } else assert(wardrobeState.baseline==a && wardrobeState.draft==a);
    window.clear(); client_main();
    sf::Texture capture; assert(capture.create(900,800)); capture.update(window);
    assert(capture.copyToImage().saveToFile((root/(save ? "build/tests/wardrobe-live-db.png" : "build/tests/wardrobe-live-restore.png")).string()));
    window.display(); s_socket.disconnect(); window.close(); g_window=nullptr;
    std::filesystem::current_path(root);
    std::cout<<(save ? "LIVE_DB_SAVE_AND_NATIVE_UI_PASS" : "SERVER_RESTART_LOGIN_RESTORE_AND_NATIVE_UI_PASS")<<std::endl;
}
