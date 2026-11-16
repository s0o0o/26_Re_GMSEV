#pragma once
#include "town_npc_renderer.hpp"
#include "../../shared/town_dialogue.hpp"
class TownDialoguePanel {
    bool eDown_=false,enterDown_=false;
    int armed_=0;
    static void box(sf::RenderTarget& target,sf::FloatRect r,sf::Color color) {
        sf::RectangleShape s({r.width,r.height});s.setPosition(r.left,r.top);s.setFillColor(color);
        s.setOutlineColor({130,86,54});s.setOutlineThickness(2);target.draw(s);
    }
    static void text(sf::RenderTarget& target,const sf::Font& font,const sf::String& value,sf::FloatRect r,unsigned size=20) {
        sf::Text t(value,font,size);t.setFillColor({72,46,37});centerText(t,r);target.draw(t);
    }
public:
    town_dialogue::State state;
    inline static const sf::FloatRect prompt{315,610,270,38},nextButton{460,600,145,40},closeButton{625,600,145,40};
    bool visible() const {return state.visible();}
    void reset() {state.close();eDown_=enterDown_=false;armed_=0;}
    static const wchar_t* line(int type,int page) {
        const wchar_t* lines[3][2]={{L"\uc5b4\uc11c \uc640! \ud544\ub4dc\uc5d0\ub294 \ub2e4\uc12f \uac1c\uc758\n\ud0d0\ud5d8 \ud45c\uc2dd\uc774 \uc788\uc5b4.",L"R\ub85c \ub3cc\uc544\uc624\uace0 M\uc73c\ub85c \uc9c0\ub3c4\ub97c \ubd10.\n\uc0b0\ucc45 \ub4a4\uc5d0 \ub610 \uc774\uc57c\uae30\ud558\uc790!"},
            {L"\uc624\ub298\uc740 \uaf43\uae38\uc744 \uac77\uace0 \uc2f6\uc5b4.\n\ub108\ub3c4 \ucc9c\ucc9c\ud788 \ub458\ub7ec\ubcf4\uc9c0 \uc54a\uc744\ub798?",L"W\ub85c \uc637\uc7a5\uc744 \uc5f4 \uc218 \uc788\uc5b4.\n\ub9c8\uc74c\uc5d0 \ub4dc\ub294 \ubaa8\uc2b5\uc73c\ub85c \uc0b0\ucc45\ud574 \ubd10."},{L"\ucfe8\ucfe8... \uc7a0\uae50 \uc26c\uc5c8\ub2e4 \uac00...\n\ub09c \uc5ec\uae30\uc11c \uae30\ub2e4\ub9b4\uac8c...",L"\uba40\ub9ac \ub2e4\ub140\uc654\uc73c\uba74 \uc26c\uc5b4\uc57c\uc9c0...\n\ubb34\ub9ac\ud558\uc9c0 \ub9d0\uace0 \ub3cc\uc544\uc640..."}};
        return town_npcs::isResident(type) ? lines[type-100][page==1?1:0] : L"";
    }
    bool handle(const sf::Event& e,sf::Vector2f mouse,world_regions::Position p,const town_dialogue::Candidates& candidates,bool blocked,bool connected) {
        const bool wasOpen=visible();state.reconcile(p,candidates,connected);
        if(e.type==sf::Event::LostFocus) {eDown_=enterDown_=false;armed_=0;}
        if(e.type==sf::Event::KeyReleased) {
            if(e.key.code==sf::Keyboard::E)eDown_=false;
            if(e.key.code==sf::Keyboard::Enter)enterDown_=false;
        }
        bool freshE=false,freshEnter=false;
        if(e.type==sf::Event::KeyPressed && e.key.code==sf::Keyboard::E){freshE=!eDown_;eDown_=true;}
        if(e.type==sf::Event::KeyPressed && e.key.code==sf::Keyboard::Enter){freshEnter=!enterDown_;enterDown_=true;}
        if(wasOpen && !visible()){armed_=0;return true;}
        if(visible()) {
            if(e.type==sf::Event::KeyPressed && e.key.code==sf::Keyboard::Escape){state.close();armed_=0;return true;}
            if(freshE || freshEnter){state.next();armed_=0;return true;}
            if(e.type==sf::Event::MouseButtonPressed && e.mouseButton.button==sf::Mouse::Left)
                armed_=nextButton.contains(mouse)?2:closeButton.contains(mouse)?3:0;
            if(e.type==sf::Event::MouseButtonReleased && e.mouseButton.button==sf::Mouse::Left) {
                if(armed_==2 && nextButton.contains(mouse))state.next();
                if(armed_==3 && closeButton.contains(mouse))state.close();
                armed_=0;
            }
            return true;
        }
        if(blocked || !connected){armed_=0;return false;}
        if(e.type==sf::Event::MouseButtonPressed && e.mouseButton.button==sf::Mouse::Left)armed_=prompt.contains(mouse)?1:0;
        bool clicked=false;
        if(e.type==sf::Event::MouseButtonReleased && e.mouseButton.button==sf::Mouse::Left){clicked=armed_==1 && prompt.contains(mouse);armed_=0;}
        return (freshE || clicked) && state.open(p,candidates);
    }
    void drawPrompt(sf::RenderTarget& target,const sf::Font& font,int type) const {
        const auto* resident=town_npcs::byType(type);if(!resident)return;
        box(target,prompt,{255,248,228});text(target,font,sf::String("[E] ")+resident->name+L" \ub300\ud654",prompt,21);
    }
    void draw(sf::RenderTarget& target,const sf::Font& font,const TownNpcRenderer& renderer,float seconds) const {
        if(!visible())return;
        box(target,{100,440,700,220},{255,248,228});
        const auto rect=TownNpcRenderer::frame(state.type,seconds);
        sf::Sprite portrait(renderer.texture(state.type),rect);portrait.setScale(2,2);
        portrait.setPosition(130+(100-rect.width*2)/2.f,474);target.draw(portrait);
        text(target,font,town_npcs::byType(state.type)->name,{245,455,400,30},23);
        text(target,font,std::to_string(state.page+1)+" / 2",{690,460,80,24},16);
        text(target,font,line(state.type,state.page),{245,500,525,62},21);
        text(target,font,state.page?L"E / Enter: \ub9c8\uce58\uae30   Esc: \ub2eb\uae30":L"E / Enter: \ub2e4\uc74c   Esc: \ub2eb\uae30",{245,566,525,25},15);
        text(target,font,L"\uac00\uae4c\uc6b4 \uc8fc\ubbfc\uacfc \uc774\uc57c\uae30 \uc911",{245,603,195,30},15);
        box(target,nextButton,{242,212,155});text(target,font,state.page?L"\ub9c8\uce58\uae30":L"\ub2e4\uc74c",nextButton,21);
        box(target,closeButton,{255,248,228});text(target,font,L"\ub2eb\uae30",closeButton,21);
    }
};
