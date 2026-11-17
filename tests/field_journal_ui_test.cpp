#define main production_client_main
#include "../TP_2023192003_CLIENT/client_sample/client.cpp"
#undef main
#include <cassert>
#include <filesystem>
int main(){const auto root=std::filesystem::current_path();std::filesystem::current_path(root/"TP_2023192003_CLIENT/client_sample");
    assert(gameFont().loadFromFile("neodgm.ttf") && hamsterCustomizer().load("assets/characters") && clubhouseRenderer().load("assets/maps/clubhouse-sheet.png") && clubhouseRenderer().loadOutdoor("assets/maps/sunny-peak-sheet.png"));
    sf::Texture profile;assert(profile.loadFromFile("profile.png"));profileSprite.setTexture(profile);profileSprite.setPosition(15,664);nameText.setString("Ham");avatar.set_name("Ham");avatar.get_sprite().setTextureRect({48,0,48,48});
    sf::RenderWindow window(sf::VideoMode(900,800),"Journal fixture",sf::Style::None);window.setVisible(false);g_window=&window;WindowDisplay::resize(window);travelState={};
    sc_packet_avatar_info info{};info.size=sizeof(info);info.type=S2C_P_AVATAR_INFO;info.id=7;info.region=2;info.x=32;info.y=32;info.hp=20;info.max_hp=80;info.level=3;info.seedNum=12;ProcessPacket(reinterpret_cast<char*>(&info));
    auto key=[&](sf::Keyboard::Key k,bool released=false){sf::Event e{};e.type=released?sf::Event::KeyReleased:sf::Event::KeyPressed;e.key.code=k;return handleFieldJournalInput(e,{-1,-1});};
    auto click=[&](sf::FloatRect r){sf::Event e{};e.type=sf::Event::MouseButtonPressed;e.mouseButton.button=sf::Mouse::Left;handleFieldJournalInput(e,{r.left+5,r.top+5});e.type=sf::Event::MouseButtonReleased;handleFieldJournalInput(e,{r.left+5,r.top+5});};
    isChatMode=true;assert(!key(sf::Keyboard::J));key(sf::Keyboard::J,true);isChatMode=false;
    assert(key(sf::Keyboard::J) && fieldJournalPanel().visible());key(sf::Keyboard::J);assert(fieldJournalPanel().visible());key(sf::Keyboard::J,true);key(sf::Keyboard::Down);assert(fieldJournalPanel().selected()==2);click(FieldJournalPanel::row(5));assert(fieldJournalPanel().selected()==5);click(FieldJournalPanel::closeButton);assert(!fieldJournalPanel().visible());click(FieldJournalPanel::entry);assert(fieldJournalPanel().visible());key(sf::Keyboard::Escape);
    for(int mode=0;mode<4;++mode){fieldDiscovery.reset();fieldDiscovery.start();fieldInvestigationPanel().reset();fieldJournalPanel().reset();
        for(const auto& l:field_landmarks::all){world_regions::Position p{world_regions::Id::SunnyPeak,l.x,l.y};if(mode)fieldDiscovery.observe(p);if(mode==3 || (mode==2 && l.id<=2)){fieldInvestigationPanel().state.open(p,true);if(mode==3 || l.id==1){fieldInvestigationPanel().state.begin(p,true,l.id*2000);fieldInvestigationPanel().state.tick(l.id*2000+900,p,true);}fieldInvestigationPanel().state.close();}}
        const auto inspectBefore=field_journal::inspected(fieldInvestigationPanel().state),doneBefore=field_journal::completed(fieldInvestigationPanel().state);
        assert(key(sf::Keyboard::J));key(sf::Keyboard::J,true);
        for(auto size:{sf::Vector2u{900,800},sf::Vector2u{1280,720},sf::Vector2u{800,900}}){sf::RenderTexture out;assert(out.create(size.x,size.y));out.clear({32,24,24});auto fit=display_layout::fit(size.x,size.y);sf::View v({0,0,900,800});v.setViewport({fit.left/size.x,fit.top/size.y,fit.width/size.x,fit.height/size.y});out.setView(v);clubhouseRenderer().draw(out,avatar.position.region,g_left_x,g_top_y);avatar.draw(&out);drawProfileHud(out);chatPanel().place(1,true);chatPanel().draw(out,gameFont(),true);fieldJournalPanel().draw(out,gameFont(),avatar.position,fieldDiscovery,fieldInvestigationPanel().state);out.display();assert(out.getTexture().copyToImage().saveToFile((root/("build/tests/journal-"+std::to_string(mode)+"-"+std::to_string(size.x)+"x"+std::to_string(size.y)+".png")).string()));}
        assert(field_journal::inspected(fieldInvestigationPanel().state)==inspectBefore && field_journal::completed(fieldInvestigationPanel().state)==doneBefore);key(sf::Keyboard::Escape);
    }
    sc_packet_move moved{};moved.size=sizeof(moved);moved.type=S2C_P_MOVE;moved.id=7;moved.region=1;moved.x=12;moved.y=10;ProcessPacket(reinterpret_cast<char*>(&moved));key(sf::Keyboard::J);key(sf::Keyboard::J,true);assert(fieldJournalPanel().visible() && field_journal::completed(fieldInvestigationPanel().state)==5);
    gameConnectionLost();assert(!fieldJournalPanel().visible() && !field_journal::completed(fieldInvestigationPanel().state));std::puts("Journal production J/repeat/click/rows/modal/session/read-only/4 states x 3 sizes PASS");}
