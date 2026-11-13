// Actual TCP/DB scene using production client parser and game drawing. Not a mock renderer.
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#define main production_client_main
#include "../TP_2023192003_CLIENT/client_sample/client.cpp"
#undef main
#define GMSEV_BOUNDED_LOAD_TEST
#include "../stress_test/STRESS_TEST/bounded_load.cpp"
#include <cassert>
#include <filesystem>
#include <set>
int main() {
    const auto root=std::filesystem::current_path();
    std::filesystem::current_path(root/"TP_2023192003_CLIENT/client_sample");
    assert(gameFont().loadFromFile("neodgm.ttf"));
    initStartScreenInit(); chatInit(); lodMapTexture();
    assert(townNpcRenderer().load("assets/npcs/source"));
    assert(hamsterCustomizer().load("assets/characters"));
    assert(hamsterCustomizer().setBackdrop(*startMainTx));
    assert(clubhouseRenderer().load("assets/maps/clubhouse-sheet.png"));
    assert(clubhouseRenderer().loadOutdoor("assets/maps/sunny-peak-sheet.png"));
    sf::RenderWindow window(sf::VideoMode(900,800),"Live DB multiplayer capture - production client code");
    window.setFramerateLimit(60); g_window=&window;
    id_input=900001299; inputAccountId="900001299"; inputNickname="Observer";
    assert(loginFlow.finishCredentials()); assert(loginFlow.connect()); beginLoginConnection();
    const auto loginDeadline=bounded_load::Clock::now()+std::chrono::seconds(10);
    while(loginFlow.stage()==GAMESTATE::ConnectingScreen && bounded_load::Clock::now()<loginDeadline) {
        sf::Event event; while(window.pollEvent(event)) if(event.type==sf::Event::Closed) return 2;
        pumpLogin(); window.clear(); drawCustomizationScreen(window); window.display();
    }
    assert(loginFlow.stage()==GAMESTATE::GameScreen);
    bounded_load::Config bots{20,900000000,20,45,GAME_PORT}; bots.scene=true;
    int botResult=-1; std::thread peers([&]{botResult=bounded_load::run(bots);});
    const auto began=bounded_load::Clock::now(); bool captured=false;
    const auto output=root/"build/tests/live-multiplayer-scene.png";
    while(window.isOpen() && bounded_load::Clock::now()-began<std::chrono::seconds(24)) {
        sf::Event event; while(window.pollEvent(event)) if(event.type==sf::Event::Closed) window.close();
        if(!window.isOpen()) break;
        window.clear(); client_main(); window.display();
        int count=0; std::set<std::pair<int,int>> positions;
        for(const auto& entry:players) if(entry.first>=0 && entry.first<MAX_USER) { ++count; positions.emplace(entry.second.m_x,entry.second.m_y); }
        if(!captured && count>=15 && bounded_load::Clock::now()-began>=std::chrono::seconds(8)) {
            sf::Texture pixels; assert(pixels.create(window.getSize().x,window.getSize().y)); pixels.update(window);
            assert(pixels.copyToImage().saveToFile(output.string())); captured=true;
            std::cout<<"LIVE_CAPTURE peer_objects="<<count<<" distinct_peer_tiles="<<positions.size()<<" observer_id="<<g_myid<<" login_stage=GameScreen"<<std::endl;
        }
    }
    s_socket.disconnect(); if(window.isOpen()) window.close(); peers.join();
    client_finish(); assert(captured && botResult==0);
}
