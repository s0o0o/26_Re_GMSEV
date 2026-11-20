#define main production_client_main
#include "../TP_2023192003_CLIENT/client_sample/client.cpp"
#undef main
#include <cassert>
#include <filesystem>
int main(){using Stage=town_walk_quest::Stage;const auto root=std::filesystem::current_path();std::filesystem::current_path(root/"TP_2023192003_CLIENT/client_sample");
    assert(gameFont().loadFromFile("neodgm.ttf") && townNpcRenderer().load("assets/npcs/source") && clubhouseRenderer().load("assets/maps/clubhouse-sheet.png"));
    sf::RenderWindow window(sf::VideoMode(900,800),"Content boundaries fixture",sf::Style::None);window.setVisible(false);g_window=&window;WindowDisplay::resize(window);
    auto info=[&](int region,int x,int y){sc_packet_avatar_info p{};p.size=sizeof(p);p.type=S2C_P_AVATAR_INFO;p.id=7;p.region=static_cast<std::uint8_t>(region);p.x=x;p.y=y;p.hp=p.max_hp=10;ProcessPacket(reinterpret_cast<char*>(&p));};
    auto reset=[&](){gameConnectionLost();travelState={};wardrobeState={};players.clear();info(2,32,32);};
    auto key=[&](sf::Keyboard::Key k,bool release=false){sf::Event e{};e.type=release?sf::Event::KeyReleased:sf::Event::KeyPressed;e.key.code=k;return handleGameUiInput(e,window,{-1,-1});};
    auto press=[&](sf::Keyboard::Key k){key(k);key(k,true);};
    auto click=[&](sf::FloatRect r,bool outside=false){sf::Event e{};e.type=sf::Event::MouseButtonPressed;e.mouseButton.button=sf::Mouse::Left;handleGameUiInput(e,window,{r.left+5,r.top+5});e.type=sf::Event::MouseButtonReleased;handleGameUiInput(e,window,outside?sf::Vector2f{-1,-1}:sf::Vector2f{r.left+5,r.top+5});};
    auto boss=[&](int type=100){const auto* r=town_npcs::byType(type);sc_packet_enter p{};p.size=sizeof(p);p.type=S2C_P_ENTER;p.id=r->id;p.monType=type;p.region=1;p.x=r->x;p.y=r->y;strcpy_s(p.name,r->name);p.hp=p.max_hp=1;ProcessPacket(reinterpret_cast<char*>(&p));};
    reset();isChatMode=true;press(sf::Keyboard::J);assert(!fieldJournalPanel().visible());isChatMode=false;
    assert(fieldInvestigationPanel().state.open(avatar.position,true));press(sf::Keyboard::J);assert(!fieldJournalPanel().visible());fieldInvestigationPanel().reset();
    press(sf::Keyboard::M);assert(fieldMapPanel().visible());press(sf::Keyboard::J);assert(!fieldJournalPanel().visible());press(sf::Keyboard::Escape);
    info(1,12,10);assert(wardrobeState.open(avatar.position,{}));press(sf::Keyboard::J);assert(!fieldJournalPanel().visible());wardrobeState={};
    travelState.status=world_travel::ClientStatus::Pending;press(sf::Keyboard::J);assert(!fieldJournalPanel().visible());travelState={};
    boss();press(sf::Keyboard::E);assert(townDialoguePanel().visible());press(sf::Keyboard::J);assert(!fieldJournalPanel().visible());press(sf::Keyboard::Escape);
    reset();click(FieldJournalPanel::entry,true);assert(!fieldJournalPanel().visible());press(sf::Keyboard::J);assert(fieldJournalPanel().visible());
    press(sf::Keyboard::Up);assert(fieldJournalPanel().selected()==1);for(int i=0;i<10;++i)press(sf::Keyboard::Down);assert(fieldJournalPanel().selected()==5);
    click(FieldJournalPanel::row(1),true);assert(fieldJournalPanel().selected()==5);click(FieldJournalPanel::closeButton,true);assert(fieldJournalPanel().visible());
    for(auto k:{sf::Keyboard::E,sf::Keyboard::F,sf::Keyboard::Q,sf::Keyboard::M,sf::Keyboard::W,sf::Keyboard::R,sf::Keyboard::Tab,sf::Keyboard::Space}){assert(key(k));key(k,true);assert(fieldJournalPanel().visible() && !travelState.pending() && !fieldMapPanel().visible() && !wardrobeState.visible() && !isChatMode && !townDialoguePanel().visible() && !fieldInvestigationPanel().visible());}
    click(FieldJournalPanel::closeButton);assert(!fieldJournalPanel().visible());
    // Holding J through Esc -> map must not leave its latch stuck either.
    key(sf::Keyboard::J);press(sf::Keyboard::Escape);press(sf::Keyboard::M);key(sf::Keyboard::J,true);press(sf::Keyboard::Escape);press(sf::Keyboard::J);assert(fieldJournalPanel().visible());press(sf::Keyboard::Escape);
    // Interrupted experience cannot set completed bit.
    press(sf::Keyboard::E);press(sf::Keyboard::F);assert(fieldInvestigationPanel().state.running());sf::Event lost{};lost.type=sf::Event::LostFocus;handleGameUiInput(lost,window,{-1,-1});assert(!fieldInvestigationPanel().state.running() && !fieldInvestigationPanel().state.completed(1));
    reset();press(sf::Keyboard::E);press(sf::Keyboard::F);assert(fieldInvestigationPanel().state.running());moveAvatar({world_regions::Id::SunnyPeak,48,32});fieldInvestigationPanel().state.tick(GetTickCount64()+900,avatar.position,true);assert(!fieldInvestigationPanel().visible() && !fieldInvestigationPanel().state.completed(1));
    // A Q release while another modal owns input must reach its original latch.
    reset();info(1,12,10);boss();fieldDiscovery.observe({world_regions::Id::SunnyPeak,32,32});fieldDiscovery.observe({world_regions::Id::SunnyPeak,90,96});townWalkQuest.accept(fieldDiscovery,true);
    press(sf::Keyboard::E);key(sf::Keyboard::Q);assert(townWalkQuest.stage()==Stage::Reported);press(sf::Keyboard::Escape);press(sf::Keyboard::J);key(sf::Keyboard::Q,true);press(sf::Keyboard::Escape);press(sf::Keyboard::E);press(sf::Keyboard::Q);assert(residentExperienceQuest.stage()==Stage::Active);
    press(sf::Keyboard::Escape);auto& actions=fieldInvestigationPanel().state;actions.open({world_regions::Id::SunnyPeak,32,32},true);actions.open({world_regions::Id::SunnyPeak,90,96},true);actions.begin({world_regions::Id::SunnyPeak,90,96},true,1000);actions.tick(1900,{world_regions::Id::SunnyPeak,90,96},true);actions.close();
    press(sf::Keyboard::E);assert(residentExperienceQuest.stage()==Stage::Ready);sc_packet_leave left{sizeof(left),S2C_P_LEAVE,town_npcs::baseId};ProcessPacket(reinterpret_cast<char*>(&left));press(sf::Keyboard::Q);assert(!townDialoguePanel().visible() && residentExperienceQuest.stage()==Stage::Ready);
    players.clear();info(1,12,16);boss(101);press(sf::Keyboard::E);assert(townDialoguePanel().visible() && townDialoguePanel().state.type==101);press(sf::Keyboard::Q);assert(residentExperienceQuest.stage()==Stage::Ready);press(sf::Keyboard::Escape);
    players.clear();info(1,12,10);boss();press(sf::Keyboard::E);press(sf::Keyboard::Q);assert(residentExperienceQuest.stage()==Stage::Reported);
    sf::RenderTexture preview;assert(preview.create(900,800));preview.clear({32,24,24});clubhouseRenderer().draw(preview,avatar.position.region,g_left_x,g_top_y);drawTownDialogueUi(preview);preview.display();assert(preview.getTexture().copyToImage().saveToFile((root/"build/tests/content-boundaries-900x800.png").string()));
    gameConnectionLost();assert(!fieldJournalPanel().visible() && !fieldInvestigationPanel().visible() && !fieldDiscovery.active() && townWalkQuest.stage()==Stage::Available && residentExperienceQuest.stage()==Stage::Available);
    reset();residentExperienceQuest.accept(true,true,true,fieldInvestigationPanel().state);press(sf::Keyboard::J);beginLoginConnection(0);assert(!fieldJournalPanel().visible() && !fieldDiscovery.active() && residentExperienceQuest.stage()==Stage::Available);
    std::puts("Content production router: six modal guards/keys/click outside/row bounds/focus+range cancel/Q+J release across modals/Boss leave+wrong NPC/session PASS");
}
