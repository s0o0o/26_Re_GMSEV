#pragma once
#include "town_npc_renderer.hpp"
#include "../../shared/town_dialogue.hpp"
#include "../../shared/exploration_guidance.hpp"
#include "../../shared/town_walk_quest.hpp"
class TownDialoguePanel {
    bool eDown_=false,enterDown_=false;
    int armed_=0;
    bool guideRequested_=false;
    sf::String guideReply_;
    bool qDown_=false,questRequested_=false,questEnabled_=false,questReply_=false;
    town_walk_quest::Stage questStage_=town_walk_quest::Stage::Available;
    unsigned questProgress_=0;
    static void box(sf::RenderTarget& target,sf::FloatRect r,sf::Color color) {
        sf::RectangleShape s({r.width,r.height});s.setPosition(r.left,r.top);s.setFillColor(color);
        s.setOutlineColor({130,86,54});s.setOutlineThickness(2);target.draw(s);
    }
    static void text(sf::RenderTarget& target,const sf::Font& font,const sf::String& value,sf::FloatRect r,unsigned size=20) {
        sf::Text t(value,font,size);t.setFillColor({72,46,37});centerText(t,r);target.draw(t);
    }
public:
    town_dialogue::State state;
    inline static const sf::FloatRect prompt{315,610,270,38},nextButton{460,600,145,40},closeButton{625,600,145,40},guideButton{245,600,195,40};
    inline static const sf::FloatRect questButton{245,556,525,36};
    bool visible() const {return state.visible();}
    void reset() {state.close();eDown_=enterDown_=guideRequested_=qDown_=questRequested_=questReply_=false;armed_=0;guideReply_.clear();}
    void setQuestContext(const town_walk_quest::State& quest,const field_discovery::Record& record) {
        questStage_=quest.stage();questProgress_=quest.progress(record);questEnabled_=record.active() && questStage_!=town_walk_quest::Stage::Reported;
    }
    bool takeQuestRequest(){const bool value=questRequested_;questRequested_=false;return value;}
    void showQuest(const town_walk_quest::State& quest,const field_discovery::Record& record) {
        questReply_=true;
        if(quest.stage()==town_walk_quest::Stage::Reported) guideReply_=L"\uc774\uc57c\uae30\ub97c \ub4e4\ub824\uc918\uc11c \uace0\ub9c8\uc6cc!\n\uc774\ubc88 \uc811\uc18d\uc758 \uc0b0\ucc45 \uc758\ub8b0\ub97c \ub9c8\ucce4\uc5b4.";
        else if(quest.stage()==town_walk_quest::Stage::Ready) guideReply_=L"\ub450 \uacf3\uc744 \ubaa8\ub450 \ub458\ub7ec\ubd24\uad6c\ub098!\n[Q]\ub85c \uc0b0\ucc45 \uc774\uc57c\uae30\ub97c \ubcf4\uace0\ud574 \uc918.";
        else guideReply_=sf::String(L"1 Flower Walk / 2 Riverside Rest\n\ub2e4\ub140\uc628 \uacf3 ")+std::to_string(quest.progress(record))+L"/2, \ub3cc\uc544\uc640 \uc54c\ub824 \uc918. (\uc811\uc18d \uc911)";
    }
    bool takeGuideRequest() {const bool value=guideRequested_;guideRequested_=false;return value;}
    void showGuide(const exploration_guidance::State& guide) {
        questReply_=false;
        if(const auto* goal=guide.landmark()) guideReply_=sf::String(std::to_string(goal->id)+"  "+goal->name)+L"\uc73c\ub85c \uac00 \ubcf4\uc790!\nM \uc9c0\ub3c4\uc5d0\uc11c \ubaa9\ud45c\ub97c \ud655\uc778\ud574.";
        else guideReply_=guide.active()?L"\ubaa8\ub4e0 \ud0d0\ud5d8 \uc9c0\uc810\uc744 \ub458\ub7ec\ubd24\uc5b4!\n\ucc9c\ucc9c\ud788 \uc26c\uc5c8\ub2e4\uac00 \ub610 \ub9cc\ub098.":L"\uc811\uc18d \uc0c1\ud0dc\ub97c \ud655\uc778\ud55c \ub4a4\n\ub2e4\uc2dc \uc774\uc57c\uae30\ud574 \uc918.";
    }
    static const wchar_t* line(int type,int page) {
        const wchar_t* lines[3][2]={{L"\uc5b4\uc11c \uc640! \ud544\ub4dc\uc5d0\ub294 \ub2e4\uc12f \uac1c\uc758\n\ud0d0\ud5d8 \ud45c\uc2dd\uc774 \uc788\uc5b4.",L"R\ub85c \ub3cc\uc544\uc624\uace0 M\uc73c\ub85c \uc9c0\ub3c4\ub97c \ubd10.\n\uc0b0\ucc45 \ub4a4\uc5d0 \ub610 \uc774\uc57c\uae30\ud558\uc790!"},
            {L"\uc624\ub298\uc740 \uaf43\uae38\uc744 \uac77\uace0 \uc2f6\uc5b4.\n\ub108\ub3c4 \ucc9c\ucc9c\ud788 \ub458\ub7ec\ubcf4\uc9c0 \uc54a\uc744\ub798?",L"W\ub85c \uc637\uc7a5\uc744 \uc5f4 \uc218 \uc788\uc5b4.\n\ub9c8\uc74c\uc5d0 \ub4dc\ub294 \ubaa8\uc2b5\uc73c\ub85c \uc0b0\ucc45\ud574 \ubd10."},{L"\ucfe8\ucfe8... \uc7a0\uae50 \uc26c\uc5c8\ub2e4 \uac00...\n\ub09c \uc5ec\uae30\uc11c \uae30\ub2e4\ub9b4\uac8c...",L"\uba40\ub9ac \ub2e4\ub140\uc654\uc73c\uba74 \uc26c\uc5b4\uc57c\uc9c0...\n\ubb34\ub9ac\ud558\uc9c0 \ub9d0\uace0 \ub3cc\uc544\uc640..."}};
        return town_npcs::isResident(type) ? lines[type-100][page==1?1:0] : L"";
    }
    bool handle(const sf::Event& e,sf::Vector2f mouse,world_regions::Position p,const town_dialogue::Candidates& candidates,bool blocked,bool connected) {
        const bool wasOpen=visible();state.reconcile(p,candidates,connected);
        if(e.type==sf::Event::LostFocus) {eDown_=enterDown_=qDown_=false;armed_=0;}
        if(e.type==sf::Event::KeyReleased) {
            if(e.key.code==sf::Keyboard::E)eDown_=false;
            if(e.key.code==sf::Keyboard::Enter)enterDown_=false;
            if(e.key.code==sf::Keyboard::Q)qDown_=false;
        }
        bool freshE=false,freshEnter=false;
        if(e.type==sf::Event::KeyPressed && e.key.code==sf::Keyboard::E){freshE=!eDown_;eDown_=true;}
        if(e.type==sf::Event::KeyPressed && e.key.code==sf::Keyboard::Enter){freshEnter=!enterDown_;enterDown_=true;}
        bool freshQ=false;if(e.type==sf::Event::KeyPressed && e.key.code==sf::Keyboard::Q){freshQ=!qDown_;qDown_=true;}
        if(wasOpen && !visible()){armed_=0;return true;}
        if(blocked){if(wasOpen)reset();armed_=0;return wasOpen;}
        if(visible()) {
            if(freshQ && state.type==100 && questEnabled_){questRequested_=true;return true;}
            if(e.type==sf::Event::KeyPressed && e.key.code==sf::Keyboard::G){guideRequested_=true;return true;}
            if(e.type==sf::Event::KeyPressed && e.key.code==sf::Keyboard::Escape){state.close();armed_=0;return true;}
            if(freshE || freshEnter){state.next();guideReply_.clear();armed_=0;return true;}
            if(e.type==sf::Event::MouseButtonPressed && e.mouseButton.button==sf::Mouse::Left)
                armed_=nextButton.contains(mouse)?2:closeButton.contains(mouse)?3:guideButton.contains(mouse)?4:
                    (state.type==100 && questEnabled_ && questButton.contains(mouse)?5:0);
            if(e.type==sf::Event::MouseButtonReleased && e.mouseButton.button==sf::Mouse::Left) {
                if(armed_==2 && nextButton.contains(mouse)){state.next();guideReply_.clear();}
                if(armed_==3 && closeButton.contains(mouse))state.close();
                if(armed_==4 && guideButton.contains(mouse))guideRequested_=true;
                if(armed_==5 && state.type==100 && questEnabled_ && questButton.contains(mouse))questRequested_=true;
                armed_=0;
            }
            return true;
        }
        if(blocked || !connected){armed_=0;return false;}
        if(e.type==sf::Event::MouseButtonPressed && e.mouseButton.button==sf::Mouse::Left)armed_=prompt.contains(mouse)?1:0;
        bool clicked=false;
        if(e.type==sf::Event::MouseButtonReleased && e.mouseButton.button==sf::Mouse::Left){clicked=armed_==1 && prompt.contains(mouse);armed_=0;}
        if((freshE || clicked) && state.open(p,candidates)){guideReply_.clear();questReply_=false;return true;}return false;
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
        if(state.type==100)text(target,font,L"\uc811\uc18d \uc911\n\uc7ac\uc811\uc18d \ucd08\uae30\ud654",{120,570,110,58},13);
        text(target,font,town_npcs::byType(state.type)->name,{245,455,400,30},23);
        text(target,font,guideReply_.isEmpty()?sf::String(std::to_string(state.page+1)+" / 2"):sf::String(questReply_?L"\uc0b0\ucc45":L"\uc548\ub0b4"),{690,460,80,24},16);
        sf::String body=guideReply_.isEmpty()?sf::String(line(state.type,state.page)):guideReply_;
        if(state.type==100 && guideReply_.isEmpty()) {
            if(questStage_==town_walk_quest::Stage::Available && state.page==0)body=L"\uaf43\uae38(1)\uacfc \uac15\uac00(2)\ub97c \ub458\ub7ec\ubcf4\uace0\n\ub3cc\uc544\uc640 \uc0b0\ucc45 \uc774\uc57c\uae30\ub97c \ub4e4\ub824\uc904\ub798?";
            else if(questStage_==town_walk_quest::Stage::Ready)body=L"\ub450 \uacf3\uc744 \ubaa8\ub450 \ub458\ub7ec\ubd24\uad6c\ub098!\n[Q]\ub85c \uc0b0\ucc45 \uc774\uc57c\uae30\ub97c \ubcf4\uace0\ud574 \uc918.";
            else if(questStage_==town_walk_quest::Stage::Reported)body=L"\uc0b0\ucc45 \uc774\uc57c\uae30\ub97c \ub4e4\ub824\uc918\uc11c \uace0\ub9c8\uc6cc!\n\uc774\ubc88 \uc811\uc18d\uc758 \uc758\ub8b0\ub97c \ub9c8\ucce4\uc5b4.";
        }
        text(target,font,body,{245,495,525,58},21);
        if(state.type==100) {
            const auto label=questStage_==town_walk_quest::Stage::Available?sf::String(L"[Q] \uc0b0\ucc45 \uc758\ub8b0 \ubc1b\uae30 (\ubcf4\uc0c1 \uc5c6\uc74c)"):
                questStage_==town_walk_quest::Stage::Ready?sf::String(L"[Q] Boss\uc5d0\uac8c \ubcf4\uace0\ud558\uae30"):
                questStage_==town_walk_quest::Stage::Reported?sf::String(L"\uc0b0\ucc45 \ubcf4\uace0 \uc644\ub8cc (\uc811\uc18d \uc911)"):
                sf::String(L"[Q] \uc9c4\ud589 \ud655\uc778 ")+std::to_string(questProgress_)+"/2"+L" (\uc811\uc18d \uc911)";
            box(target,questButton,questEnabled_?sf::Color(242,212,155):sf::Color(221,213,191));text(target,font,label,questButton,18);
        } else text(target,font,state.page?L"E / Enter: \ub9c8\uce58\uae30   Esc: \ub2eb\uae30":L"E / Enter: \ub2e4\uc74c   Esc: \ub2eb\uae30",{245,566,525,25},15);
        box(target,guideButton,{242,212,155});text(target,font,L"[G] \ud0d0\ud5d8 \uc548\ub0b4",guideButton,19);
        box(target,nextButton,{242,212,155});text(target,font,state.page?L"\ub9c8\uce58\uae30":L"\ub2e4\uc74c",nextButton,21);
        box(target,closeButton,{255,248,228});text(target,font,L"\ub2eb\uae30",closeButton,21);
    }
};
