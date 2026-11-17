#define main production_client_main
#include "../TP_2023192003_CLIENT/client_sample/client.cpp"
#undef main
#include <cassert>
#include <filesystem>
int main(){
    const auto root=std::filesystem::current_path();std::filesystem::current_path(root/"TP_2023192003_CLIENT/client_sample");
    assert(gameFont().loadFromFile("neodgm.ttf"));assert(clubhouseRenderer().load("assets/maps/clubhouse-sheet.png"));assert(hamsterCustomizer().load("assets/characters"));assert(townNpcRenderer().load("assets/npcs/source"));
    sf::Texture profile,monster;assert(profile.loadFromFile("profile.png") && monster.loadFromFile("monProfile.png"));profileSprite.setTexture(profile);profileSprite.setPosition(15,664);monProfile=&monster;hunsterProfile=&monster;
    sf::RenderWindow window(sf::VideoMode(900,800),"Chat regression",sf::Style::None);window.setVisible(false);g_window=&window;
    WindowDisplay::resize(window);travelState={};avatar.id=g_myid=7;avatar.set_name("Ham");
    nameText.setString("Ham");sc_packet_avatar_info info{};info.size=sizeof(info);info.type=S2C_P_AVATAR_INFO;info.id=7;info.region=1;info.x=12;info.y=10;info.hp=20;info.max_hp=80;info.level=3;info.seedNum=12;ProcessPacket(reinterpret_cast<char*>(&info));
    const sf::FloatRect lineSlot{25,29,340,18};
    for(const auto& row:ChatPanel::wrap(sf::String(std::string(NAME_SIZE-1,'W'))+L": "+sf::String(std::string(119,'W')))) {sf::Text longest(row,gameFont(),15);ChatPanel::placeLine(longest,lineSlot);auto ink=longest.getGlobalBounds();assert(ink.left>=25 && ink.left+ink.width<=365 && ink.top>=29 && ink.top+ink.height<=47);}
    auto key=[&](sf::Keyboard::Key k,bool release=false){sf::Event e{};e.type=release?sf::Event::KeyReleased:sf::Event::KeyPressed;e.key.code=k;return handleChatInput(e,{-1,-1});};
    auto text=[&](sf::Uint32 cp){sf::Event e{};e.type=sf::Event::TextEntered;e.text.unicode=cp;return handleChatInput(e,{-1,-1});};
    chatPanel().reset();chatPanel().place(1,true);assert(key(sf::Keyboard::Tab) && isChatMode);key(sf::Keyboard::Tab);assert(isChatMode);key(sf::Keyboard::Tab,true);
    text(0xac00);text(0xb098);text(8);assert(chatPanel().draft()==u8"가");
    for(int i=0;i<50;++i)text(0xac00);assert(chatPanel().draft().size()==117);text(8);assert(chatPanel().draft().size()==114);
    key(sf::Keyboard::Escape);assert(!isChatMode && chatPanel().draft().empty());
    sf::TcpListener listener;assert(listener.listen(0,sf::IpAddress::LocalHost)==sf::Socket::Done);assert(s_socket.connect(sf::IpAddress::LocalHost,listener.getLocalPort())==sf::Socket::Done);
    sf::TcpSocket peer;assert(listener.accept(peer)==sf::Socket::Done);s_socket.setBlocking(false);
    key(sf::Keyboard::Tab);key(sf::Keyboard::Tab,true);for(auto cp:sf::String(L"안녕"))text(cp);key(sf::Keyboard::Enter);assert(!isChatMode);key(sf::Keyboard::Enter,true);
    cs_packet_chat sent{};std::size_t n=0;assert(peer.receive(&sent,sizeof(sent),n)==sf::Socket::Done && n==sizeof(sent));assert(sent.id==7 && std::string(sent.message)==u8"안녕");
    const auto count=players.size();sc_packet_chat received{};received.size=sizeof(received);received.type=S2C_P_CHAT;received.id=89;strcpy_s(received.message,u8"같이 산책할래?");ProcessPacket(reinterpret_cast<char*>(&received));assert(players.size()==count && chatPanel().lines().size()==1);
    std::string wireLong(119,'x');strcpy_s(received.message,wireLong.c_str());ProcessPacket(reinterpret_cast<char*>(&received));assert(chatPanel().lines().size()==2 && chatPanel().lines().back().getSize()>=119);
    memset(received.message,'x',sizeof(received.message));ProcessPacket(reinterpret_cast<char*>(&received));assert(chatPanel().lines().size()==2);
    for(int i=0;i<70;++i)chatPanel().receive(L"친구",u8"꽃길에서 만나!");assert(chatPanel().lines().size()==64);
    key(sf::Keyboard::P);assert(!chatPanel().profileVisible);key(sf::Keyboard::P);assert(!chatPanel().profileVisible);key(sf::Keyboard::P,true);
    chatPanel().place(1,false);const auto before=chatPanel().bounds().top;key(sf::Keyboard::P);key(sf::Keyboard::P,true);chatPanel().place(.02f,true);assert(chatPanel().bounds().top<before);
    for(int i=0;i<40;++i)chatPanel().place(.016f,true);assert(chatPanel().bounds().top+chatPanel().bounds().height<=profile_layout::selfTop()-11.9f);
    for(int mode=0;mode<4;++mode)for(auto size:{sf::Vector2u{900,800},sf::Vector2u{1280,720},sf::Vector2u{800,900}}){
        const bool shown=mode!=0;sc_packet_monster_near nearPacket{};nearPacket.size=sizeof(nearPacket);nearPacket.type=S2C_P_MONSTER_NEAR;nearPacket.monCount=mode<2?0:(mode==2?1:5);
        nearPacket.size=static_cast<std::uint8_t>(offsetof(sc_packet_monster_near,monsters)+sizeof(nearMonsterInfo)*nearPacket.monCount);
        for(int i=0;i<nearPacket.monCount;++i){nearPacket.monsters[i].monType=1;nearPacket.monsters[i].monId=10000+i;nearPacket.monsters[i].monHp=32767;}ProcessPacket(reinterpret_cast<char*>(&nearPacket));assert(nearMonCnt==nearPacket.monCount);
        chatPanel().profileVisible=shown;for(int i=0;i<40;++i)chatPanel().place(.016f,true,profile_layout::stackTop(shown,nearMonCnt));chatPanel().editing=true;
        assert(chatPanel().bounds().top>0 && chatPanel().bounds().top+174<=profile_layout::stackTop(shown,nearMonCnt)-11.9f);
        sf::RenderTexture out;assert(out.create(size.x,size.y));out.clear({32,24,24});auto fit=display_layout::fit(size.x,size.y);sf::View view({0,0,900,800});view.setViewport({fit.left/size.x,fit.top/size.y,fit.width/size.x,fit.height/size.y});out.setView(view);
        clubhouseRenderer().draw(out,world_regions::Id::Clubhouse,0,0);drawProfileHud(out);chatPanel().draw(out,gameFont(),true);out.display();
        const std::string state=mode==0?"free":(mode==1?"profile":(mode==2?"monster":"crowded"));
        assert(out.getTexture().copyToImage().saveToFile((root/("build/tests/chat-"+state+"-"+std::to_string(size.x)+"x"+std::to_string(size.y)+".png")).string()));
    }
    auto capture=[&](const std::string& name,ChatPanel& panel,bool connected,bool integrated=false,sf::Vector2u size={900,800}) {
        sf::RenderTexture out;assert(out.create(size.x,size.y));out.clear({32,24,24});auto fit=display_layout::fit(size.x,size.y);sf::View view({0,0,900,800});view.setViewport({fit.left/size.x,fit.top/size.y,fit.width/size.x,fit.height/size.y});out.setView(view);clubhouseRenderer().draw(out,world_regions::Id::Clubhouse,g_left_x,g_top_y);drawProfileHud(out);panel.draw(out,gameFont(),connected);
        if(integrated){explorationGuide.select(fieldDiscovery);drawExplorationGuidance(out,gameFont(),explorationGuide,avatar.position);townDialoguePanel().drawPrompt(out,gameFont(),100);}
        out.display();assert(out.getTexture().copyToImage().saveToFile((root/("build/tests/chat-state-"+name+".png")).string()));
    };
    nearMonCnt=0;ChatPanel history;history.place(0,true);std::string longKorean;for(int i=0;i<2;++i)longKorean+=u8"꽃길에서 친구들과 함께 산책해요 ";assert(chat_rules::valid(longKorean));
    auto captureHistory=[&](const char* state){for(auto size:{sf::Vector2u{900,800},sf::Vector2u{1280,720},sf::Vector2u{800,900}})capture(std::string(state)+"-"+std::to_string(size.x)+"x"+std::to_string(size.y),history,true,false,size);};
    for(int i=0;i<12;++i)history.receive("Ham"+std::to_string(i),longKorean);captureHistory("long-latest");
    history.scroll(8);const auto stable=history.rows()[history.rows().size()-history.offset()-5];history.receive(L"친구",longKorean);assert(history.rows()[history.rows().size()-history.offset()-5]==stable && history.unread()==1);captureHistory("long-scrolled");
    sf::Event recent{};recent.type=sf::Event::MouseButtonReleased;recent.mouseButton.button=sf::Mouse::Left;history.handle(recent,{300,history.bounds().top+10},false,true,0,[](const auto&){return true;});assert(!history.offset() && !history.unread());captureHistory("long-returned");
    ChatPanel states;states.place(1,true);nearMonCnt=0;
    auto feed=[&](sf::Keyboard::Key k,std::uint64_t now,bool sent=true){sf::Event e{};e.type=sf::Event::KeyPressed;e.key.code=k;states.handle(e,{-1,-1},false,true,now,[&](const auto&){return sent;});e.type=sf::Event::KeyReleased;states.handle(e,{-1,-1},false,true,now,[](const auto&){return true;});};
    auto type=[&](sf::Uint32 cp){sf::Event e{};e.type=sf::Event::TextEntered;e.text.unicode=cp;states.handle(e,{-1,-1},false,true,1000,[](const auto&){return true;});};
    capture("empty",states,true);capture("offline",states,false);
    feed(sf::Keyboard::Tab,1000);for(int i=0;i<119;++i)type('a');capture("max-input",states,true);type('a');capture("limit",states,true);assert(states.draft().size()==119);
    feed(sf::Keyboard::Enter,1000,false);assert(states.editing);capture("send-failure",states,true);
    feed(sf::Keyboard::Enter,1000);assert(!states.editing);feed(sf::Keyboard::Tab,1001);type('b');feed(sf::Keyboard::Enter,1001);assert(states.editing);capture("rate",states,true);
    sf::Event lost{};lost.type=sf::Event::LostFocus;states.handle(lost,{-1,-1},false,true,1002,[](const auto&){return true;});assert(!states.editing && states.draft().empty());
    feed(sf::Keyboard::Tab,1600);sf::Event blocked{};blocked.type=sf::Event::KeyPressed;blocked.key.code=sf::Keyboard::Enter;states.handle(blocked,{-1,-1},true,true,1600,[](const auto&){assert(false);return true;});assert(!states.editing);
    chatPanel().profileVisible=true;for(int i=0;i<40;++i)chatPanel().place(.016f,true);capture("integrated",chatPanel(),true,true);
    states.reset();states.place(0,false);nearMonCnt=5;for(int i=0;i<5;++i){nearMonsters[i].monType=1;monIdText[i].setString(std::to_string(10000+i));monHpText[i].setString("32767");}states.place(.02f,true,profile_layout::stackTop(true,5));capture("moving",states,true);
    nearMonCnt=0;avatar.get_sprite().setTextureRect({48,0,48,48});received.id=7;strcpy_s(received.message,u8"산책 다녀올게!");ProcessPacket(reinterpret_cast<char*>(&received));assert(chatPanel().lines().back().find(L"Ham")!=sf::String::InvalidPos);
    sf::RenderTexture bubble;assert(bubble.create(900,800));bubble.clear();clubhouseRenderer().draw(bubble,avatar.position.region,g_left_x,g_top_y);avatar.draw(&bubble);drawProfileHud(bubble);chatPanel().draw(bubble,gameFont(),true);bubble.display();auto bubbleImage=bubble.getTexture().copyToImage();assert(bubbleImage.getPixel(100,100)!=sf::Color::Black);assert(bubbleImage.saveToFile((root/"build/tests/chat-state-bubble.png").string()));
    gameConnectionLost();assert(!isChatMode && chatPanel().lines().empty());
    std::puts("Chat UTF8/input/TCP/unknown sender/bounded history/profile motion/3 sizes/disconnect PASS");
}
