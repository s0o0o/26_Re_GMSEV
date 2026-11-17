#include "../TP_2023192003_CLIENT/client_sample/chat_panel.hpp"
#include "../TP_2023192003_CLIENT/client_sample/window_display.hpp"
#include <Windows.h>
#include <filesystem>
#include <fstream>
int main(){
    wchar_t executable[MAX_PATH]{};GetModuleFileNameW(nullptr,executable,MAX_PATH);
    const auto root=std::filesystem::path(executable).parent_path().parent_path().parent_path();const auto assets=root/"TP_2023192003_CLIENT/client_sample";
    sf::Font font;if(!font.loadFromFile((assets/"neodgm.ttf").string()))return 1;
    sf::Texture profile;if(!profile.loadFromFile((assets/"profile.png").string()))return 1;sf::Sprite sprite(profile);sprite.setPosition(15,664);
    sf::RenderWindow window(sf::VideoMode(900,800),"GMSEV Chat Input Probe - NO DB");window.setFramerateLimit(60);WindowDisplay::resize(window);
    std::ofstream log(root/"build/tests/chat-input-events.log",std::ios::binary);ChatPanel panel;sf::Clock clock,lifetime;
    for(int i=0;i<12;++i)panel.receive("Probe","Earlier message "+std::to_string(i));
    while(window.isOpen() && lifetime.getElapsedTime().asSeconds()<600){
        panel.place(clock.restart().asSeconds(),panel.profileVisible);sf::Event event{};
        while(window.pollEvent(event)){
            if(event.type==sf::Event::Closed){window.close();break;}
            if(event.type==sf::Event::Resized)WindowDisplay::resize(window);
            if(event.type==sf::Event::TextEntered){log<<"TEXT U+"<<std::hex<<event.text.unicode<<std::dec<<"\n";log.flush();}
            sf::Vector2i pixel=sf::Mouse::getPosition(window);
            if(event.type==sf::Event::MouseWheelScrolled)pixel={event.mouseWheelScroll.x,event.mouseWheelScroll.y};
            if(event.type==sf::Event::MouseButtonReleased)pixel={event.mouseButton.x,event.mouseButton.y};
            panel.handle(event,window.mapPixelToCoords(pixel),false,true,GetTickCount64(),[&](const std::string& s){log<<"LOCAL_ECHO "<<s<<"\n";log.flush();panel.receive("Probe",s);return true;});
        }
        if(!window.isOpen())break;window.clear({82,108,72});
        sf::Text help(L"DB 없는 입력 검증\nTab: 입력 / Enter: 로컬 표시\n휠 · PageUp / PageDown · End\nP: 프로필 표시 / 숨김\n실제 게임 서버로 전송하지 않음",font,21);help.setPosition(410,80);help.setFillColor({255,248,228});window.draw(help);
        if(panel.profileVisible)window.draw(sprite,profile_layout::selfStates());panel.draw(window,font,true);window.display();
    }
}
