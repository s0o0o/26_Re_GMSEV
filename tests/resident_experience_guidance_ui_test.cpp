#define main production_client_main
#include "../TP_2023192003_CLIENT/client_sample/client.cpp"
#undef main
#include <cassert>
#include <filesystem>
int main(){const auto root=std::filesystem::current_path();std::filesystem::current_path(root/"TP_2023192003_CLIENT/client_sample");
    assert(gameFont().loadFromFile("neodgm.ttf") && hamsterCustomizer().load("assets/characters") && clubhouseRenderer().load("assets/maps/clubhouse-sheet.png") && clubhouseRenderer().loadOutdoor("assets/maps/sunny-peak-sheet.png"));
    sf::Texture profile;assert(profile.loadFromFile("profile.png"));profileSprite.setTexture(profile);profileSprite.setPosition(15,664);nameText.setString("Ham");avatar.set_name("Ham");avatar.get_sprite().setTextureRect({48,0,48,48});
    sf::RenderWindow window(sf::VideoMode(900,800),"Experience guidance fixture",sf::Style::None);window.setVisible(false);g_window=&window;WindowDisplay::resize(window);travelState={};wardrobeState={};
    sc_packet_avatar_info info{};info.size=sizeof(info);info.type=S2C_P_AVATAR_INFO;info.id=7;info.region=2;info.x=32;info.y=32;info.hp=20;info.max_hp=80;info.level=3;info.seedNum=12;ProcessPacket(reinterpret_cast<char*>(&info));
    // Production snapshot unlocks Boss experience only after the walk report.
    field_discovery::Record prerequisite;prerequisite.start();prerequisite.observe({world_regions::Id::SunnyPeak,32,32});prerequisite.observe({world_regions::Id::SunnyPeak,90,96});townWalkQuest.reset();assert(townWalkQuest.accept(prerequisite,true));assert(townWalkQuest.report(prerequisite,true));questTracking.reset();pashminaExperienceQuest.reset();
    auto key=[&](sf::Keyboard::Key k,bool release=false){sf::Event e{};e.type=release?sf::Event::KeyReleased:sf::Event::KeyPressed;e.key.code=k;return handleFieldJournalInput(e,{-1,-1});};
    for(int mode=0;mode<5;++mode){fieldDiscovery.reset();fieldDiscovery.start();fieldInvestigationPanel().reset();fieldJournalPanel().reset();residentExperienceQuest.reset();auto& a=fieldInvestigationPanel().state;
        if(mode>=2){world_regions::Position p{world_regions::Id::SunnyPeak,32,32};fieldDiscovery.observe(p);a.open(p,true);a.close();}
        if(mode>=3){world_regions::Position p{world_regions::Id::SunnyPeak,90,96};fieldDiscovery.observe(p);a.open(p,true);a.begin(p,true,1000);a.tick(1900,p,true);a.close();}
        if(mode)residentExperienceQuest.accept(true,true,true,a);if(mode==4)residentExperienceQuest.report(true,true,a);
        const auto stage=residentExperienceQuest.stage();const auto bits=field_journal::completed(a);assert(key(sf::Keyboard::J));key(sf::Keyboard::J,true);assert(fieldJournalPanel().selected()==(mode==2?2:1));
        for(auto size:{sf::Vector2u{900,800},sf::Vector2u{1280,720},sf::Vector2u{800,900}}){for(int hud=0;hud<2;++hud){if(hud && (mode==0 || mode==4))continue;sf::RenderTexture out;assert(out.create(size.x,size.y));out.clear({32,24,24});auto fit=display_layout::fit(size.x,size.y);sf::View v({0,0,900,800});v.setViewport({fit.left/size.x,fit.top/size.y,fit.width/size.x,fit.height/size.y});out.setView(v);clubhouseRenderer().draw(out,avatar.position.region,g_left_x,g_top_y);avatar.draw(&out);drawProfileHud(out);chatPanel().place(1,true);chatPanel().draw(out,gameFont(),true);
                if(hud)assert(drawResidentExperienceQuest(out,gameFont(),residentExperienceQuest,a,fieldDiscovery,avatar.position));else fieldJournalPanel().draw(out,gameFont(),avatar.position,fieldDiscovery,a,false,&residentExperienceQuest);
                out.display();const auto prefix=hud?"experience-hud-":"experience-journal-";assert(out.getTexture().copyToImage().saveToFile((root/(std::string("build/tests/")+prefix+std::to_string(mode)+"-"+std::to_string(size.x)+"x"+std::to_string(size.y)+".png")).string()));}}
        assert(stage==residentExperienceQuest.stage() && bits==field_journal::completed(a));key(sf::Keyboard::Escape);
    }
    residentExperienceQuest.reset();residentExperienceQuest.accept(true,true,true,fieldInvestigationPanel().state);assert(resident_experience_guidance::next(true,residentExperienceQuest,fieldInvestigationPanel().state).kind==resident_experience_guidance::Kind::Report);
    assert(experienceNextText({resident_experience_guidance::Kind::Report,0},{world_regions::Id::Clubhouse,12,10})!=experienceNextText({resident_experience_guidance::Kind::Report,0},{world_regions::Id::SunnyPeak,32,32}));
    gameConnectionLost();assert(residentExperienceQuest.stage()==resident_experience_quest::Stage::Available);
    std::puts("Experience guidance J/preferred row/read-only/5 states journal + 3 active HUD x 3 sizes/session PASS");
}
