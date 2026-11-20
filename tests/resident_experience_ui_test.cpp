#define main production_client_main
#include "../TP_2023192003_CLIENT/client_sample/client.cpp"
#undef main
#include <cassert>
#include <filesystem>
int main(){using Stage=town_walk_quest::Stage;
    const auto root=std::filesystem::current_path();std::filesystem::current_path(root/"TP_2023192003_CLIENT/client_sample");
    assert(gameFont().loadFromFile("neodgm.ttf") && townNpcRenderer().load("assets/npcs/source") && hamsterCustomizer().load("assets/characters"));
    assert(clubhouseRenderer().load("assets/maps/clubhouse-sheet.png") && clubhouseRenderer().loadOutdoor("assets/maps/sunny-peak-sheet.png"));
    sf::RenderWindow window(sf::VideoMode(900,800),"Experience quest fixture",sf::Style::None);window.setVisible(false);g_window=&window;WindowDisplay::resize(window);
    travelState={};wardrobeState={};sc_packet_avatar_info info{};info.size=sizeof(info);info.type=S2C_P_AVATAR_INFO;info.id=7;info.region=1;info.x=12;info.y=10;info.hp=info.max_hp=20;ProcessPacket(reinterpret_cast<char*>(&info));
    auto boss=[&](){const auto& r=town_npcs::residents[0];sc_packet_enter p{};p.size=sizeof(p);p.type=S2C_P_ENTER;p.id=r.id;p.monType=r.type;p.region=1;p.x=r.x;p.y=r.y;strcpy_s(p.name,r.name);p.hp=p.max_hp=1;ProcessPacket(reinterpret_cast<char*>(&p));};boss();
    auto key=[&](sf::Keyboard::Key k,bool release=false){sf::Event e{};e.type=release?sf::Event::KeyReleased:sf::Event::KeyPressed;e.key.code=k;return handleTownDialogueInput(e,{-1,-1});};
    auto press=[&](sf::Keyboard::Key k){key(k);key(k,true);};
    auto own=[&](int region,int x,int y){sc_packet_move p{};p.size=sizeof(p);p.type=S2C_P_MOVE;p.id=7;p.region=static_cast<std::uint8_t>(region);p.x=x;p.y=y;ProcessPacket(reinterpret_cast<char*>(&p));};
auto capture=[&](int stage){for(auto size:{sf::Vector2u{900,800},sf::Vector2u{1280,720},sf::Vector2u{800,900}}){sf::RenderTexture out;assert(out.create(size.x,size.y));out.clear({32,24,24});auto fit=display_layout::fit(size.x,size.y);sf::View v({0,0,900,800});v.setViewport({fit.left/size.x,fit.top/size.y,fit.width/size.x,fit.height/size.y});out.setView(v);clubhouseRenderer().draw(out,avatar.position.region,g_left_x,g_top_y);townNpcRenderer().draw(out,100,(11-g_left_x)*50.f,(9-g_top_y)*50.f,gameFont(),"Boss",stage*.3f,town_motion::Down);drawTownDialogueUi(out);out.display();assert(out.getTexture().copyToImage().saveToFile((root/("build/tests/experience-dialogue-"+std::to_string(stage)+"-"+std::to_string(size.x)+"x"+std::to_string(size.y)+".png")).string()));}};
    // Original visit quest still must be accepted/reported separately.
    press(sf::Keyboard::E);press(sf::Keyboard::Q);assert(townWalkQuest.stage()==Stage::Active && residentExperienceQuest.stage()==Stage::Available);press(sf::Keyboard::Escape);
    own(2,32,32);own(2,90,96);own(1,12,10);boss();press(sf::Keyboard::E);key(sf::Keyboard::Q);assert(townWalkQuest.stage()==Stage::Reported);key(sf::Keyboard::Q);assert(residentExperienceQuest.stage()==Stage::Available);key(sf::Keyboard::Q,true);press(sf::Keyboard::Escape);press(sf::Keyboard::E);capture(0);
    const auto seeds=avatar._seedNum;press(sf::Keyboard::Q);assert(residentExperienceQuest.stage()==Stage::Active);capture(1);press(sf::Keyboard::Escape);
    // Own packet positions + production E/F input; deterministic completion clock injected into State.
    own(2,32,32);sf::Event e{};e.type=sf::Event::KeyPressed;e.key.code=sf::Keyboard::E;assert(handleFieldInvestigationInput(e,{-1,-1}));e.type=sf::Event::KeyReleased;handleFieldInvestigationInput(e,{-1,-1});fieldInvestigationPanel().state.close();
    own(2,90,96);e.type=sf::Event::KeyPressed;e.key.code=sf::Keyboard::E;handleFieldInvestigationInput(e,{-1,-1});e.type=sf::Event::KeyReleased;handleFieldInvestigationInput(e,{-1,-1});e.type=sf::Event::KeyPressed;e.key.code=sf::Keyboard::F;handleFieldInvestigationInput(e,{-1,-1});e.type=sf::Event::KeyReleased;handleFieldInvestigationInput(e,{-1,-1});assert(fieldInvestigationPanel().state.running());fieldInvestigationPanel().state.tick(GetTickCount64()+900,avatar.position,true);
    own(1,12,10);boss();press(sf::Keyboard::E);assert(residentExperienceQuest.stage()==Stage::Ready);capture(2);key(sf::Keyboard::Q);assert(residentExperienceQuest.stage()==Stage::Reported && avatar._seedNum==seeds);key(sf::Keyboard::Q);assert(residentExperienceQuest.stage()==Stage::Reported);key(sf::Keyboard::Q,true);capture(3);
    gameConnectionLost();assert(residentExperienceQuest.stage()==Stage::Available);fieldDiscovery.start();residentExperienceQuest.accept(true,true,true,fieldInvestigationPanel().state);beginLoginConnection(0);assert(residentExperienceQuest.stage()==Stage::Available);
    std::puts("Experience quest production visit prerequisite/E/F/Q/repeat/roundtrip/report/no reward/disconnect/login + 12 renders PASS");
}
