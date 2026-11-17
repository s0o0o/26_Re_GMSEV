#define main production_client_main
#include "../TP_2023192003_CLIENT/client_sample/client.cpp"
#undef main
#include <cassert>
#include <filesystem>
int main(){
    const auto root=std::filesystem::current_path();std::filesystem::current_path(root/"TP_2023192003_CLIENT/client_sample");
    assert(gameFont().loadFromFile("neodgm.ttf") && hamsterCustomizer().load("assets/characters") && clubhouseRenderer().load("assets/maps/clubhouse-sheet.png") && clubhouseRenderer().loadOutdoor("assets/maps/sunny-peak-sheet.png"));
    sf::Texture profile;assert(profile.loadFromFile("profile.png"));profileSprite.setTexture(profile);profileSprite.setPosition(15,664);
    sf::RenderWindow window(sf::VideoMode(900,800),"Field content fixture",sf::Style::None);window.setVisible(false);g_window=&window;WindowDisplay::resize(window);travelState={};
    nameText.setString("Ham");avatar.set_name("Ham");avatar.get_sprite().setTextureRect({48,0,48,48});
    sc_packet_avatar_info info{};info.size=sizeof(info);info.type=S2C_P_AVATAR_INFO;info.id=7;info.region=2;info.x=32;info.y=32;info.hp=20;info.max_hp=80;info.level=3;info.seedNum=12;ProcessPacket(reinterpret_cast<char*>(&info));
    auto move=[&](world_regions::Id region,int x,int y){sc_packet_move p{};p.size=sizeof(p);p.type=S2C_P_MOVE;p.id=7;p.region=static_cast<std::uint8_t>(region);p.x=x;p.y=y;ProcessPacket(reinterpret_cast<char*>(&p));};
    auto key=[&](sf::Keyboard::Key k,bool released=false){sf::Event e{};e.type=released?sf::Event::KeyReleased:sf::Event::KeyPressed;e.key.code=k;return handleFieldInvestigationInput(e,{-1,-1});};
    auto capture=[&](const std::string& state,int id,sf::Vector2u size,FieldInvestigationPanel* panel=nullptr,std::uint64_t now=0){sf::RenderTexture out;assert(out.create(size.x,size.y));out.clear({32,24,24});auto fit=display_layout::fit(size.x,size.y);sf::View v({0,0,900,800});v.setViewport({fit.left/size.x,fit.top/size.y,fit.width/size.x,fit.height/size.y});out.setView(v);clubhouseRenderer().draw(out,avatar.position.region,g_left_x,g_top_y);avatar.draw(&out);drawProfileHud(out);chatPanel().place(1,true);chatPanel().draw(out,gameFont(),true);(panel?*panel:fieldInvestigationPanel()).draw(out,gameFont(),avatar.position,fieldDiscovery.active(),now);out.display();assert(out.getTexture().copyToImage().saveToFile((root/("build/tests/field-"+state+"-"+std::to_string(id)+"-"+std::to_string(size.x)+"x"+std::to_string(size.y)+".png")).string()));};
    isChatMode=true;assert(!key(sf::Keyboard::E));key(sf::Keyboard::E,true);isChatMode=false;
    fieldDiscovery.reset();assert(!key(sf::Keyboard::E));key(sf::Keyboard::E,true);fieldDiscovery.start();
    for(const auto& l:field_landmarks::all){move(world_regions::Id::SunnyPeak,l.x,l.y);assert(key(sf::Keyboard::E) && fieldInvestigationPanel().visible());key(sf::Keyboard::E);assert(fieldInvestigationPanel().visible() && fieldInvestigationPanel().state.inspected(l.id));
        for(auto size:{sf::Vector2u{900,800},sf::Vector2u{1280,720},sf::Vector2u{800,900}})capture("investigate",l.id,size);
        auto actionPanel=fieldInvestigationPanel();assert(actionPanel.state.begin(avatar.position,true,0));
        auto canceled=actionPanel;sf::Event lost{};lost.type=sf::Event::LostFocus;canceled.handle(lost,{-1,-1},avatar.position,true,false,true,450);assert(!canceled.state.running() && !canceled.state.tick(900,avatar.position,true));
        for(auto size:{sf::Vector2u{900,800},sf::Vector2u{1280,720},sf::Vector2u{800,900}})capture("action-mid",l.id,size,&actionPanel,450);
        assert(actionPanel.state.tick(900,avatar.position,true) && actionPanel.state.completed(l.id));
        auto buttonPanel=fieldInvestigationPanel();sf::Event press{};press.type=sf::Event::MouseButtonPressed;press.mouseButton.button=sf::Mouse::Left;
        buttonPanel.handle(press,{500,535},avatar.position,true,false,true,0);press.type=sf::Event::MouseButtonReleased;buttonPanel.handle(press,{-1,-1},avatar.position,true,false,true,0);assert(!buttonPanel.state.running());
        press.type=sf::Event::MouseButtonPressed;buttonPanel.handle(press,{500,535},avatar.position,true,false,true,0);press.type=sf::Event::MouseButtonReleased;buttonPanel.handle(press,{500,535},avatar.position,true,false,true,0);assert(buttonPanel.state.running());
        capture("action-done",l.id,{900,800},&actionPanel,1600);
        sf::Event map{};map.type=sf::Event::KeyPressed;map.key.code=sf::Keyboard::M;assert(!handleFieldMapInput(map,{-1,-1}) && !fieldMapPanel().visible());
        key(sf::Keyboard::E,true);key(sf::Keyboard::Escape);assert(!fieldInvestigationPanel().visible());
    }
    move(world_regions::Id::SunnyPeak,32,32);capture("prompt",1,{900,800});
    auto click=[&](sf::FloatRect r){sf::Event e{};e.type=sf::Event::MouseButtonPressed;e.mouseButton.button=sf::Mouse::Left;const auto point=sf::Vector2f(r.left+r.width/2,r.top+r.height/2);handleFieldInvestigationInput(e,point);e.type=sf::Event::MouseButtonReleased;handleFieldInvestigationInput(e,point);};
    click(FieldInvestigationPanel::prompt);assert(fieldInvestigationPanel().visible());assert(key(sf::Keyboard::F) && fieldInvestigationPanel().state.running());key(sf::Keyboard::F);assert(fieldInvestigationPanel().state.running());key(sf::Keyboard::F,true);const auto seedBefore=avatar._seedNum;
    assert(fieldInvestigationPanel().state.tick(GetTickCount64()+1000,avatar.position,true) && fieldInvestigationPanel().state.completed(1));assert(avatar._seedNum==seedBefore);
    click(FieldInvestigationPanel::closeButton);assert(!fieldInvestigationPanel().visible());
    key(sf::Keyboard::E);key(sf::Keyboard::E,true);move(world_regions::Id::SunnyPeak,37,32);assert(!fieldInvestigationPanel().visible());
    move(world_regions::Id::Clubhouse,12,10);assert(fieldInvestigationPanel().state.inspected(1));gameConnectionLost();assert(!fieldInvestigationPanel().state.inspected(1));
    const auto transformed=profile_layout::selfStates().transform.transformRect({15,664,520,121});assert(transformed.left==15 && transformed.top>=676 && transformed.top+transformed.height<=785.1f);
    std::puts("Field content production E/F/repeat/modals/actions/cancel/5 landmarks/resize/leave/session/profile90% PASS");
}
