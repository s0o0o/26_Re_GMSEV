#pragma once
#include "ui_alignment.hpp"
#include "../../shared/field_investigation.hpp"
class FieldInvestigationPanel {
    bool eDown_=false;int armed_=0;
public:
    field_investigation::State state;
    inline static const sf::FloatRect prompt{395,584,300,38},closeButton{695,566,160,38};
    bool visible()const{return state.visible();}
    void reset(){state.reset();eDown_=false;armed_=0;}
    static const wchar_t* description(int id){
        const wchar_t* lines[]={L"풀 사이로 작은 꽃들이 고개를 내밀어.\n햇빛이 드는 길을 천천히 걸어 봐.",
            L"나무다리 아래로 맑은 물이 흘러.\n잠시 멈추면 잔잔한 물결이 보여.",L"바위 아래의 길은 단단하고 서늘해.\n절벽 가장자리를 조심하며 둘러봐.",
            L"오래된 그루터기 주위에 풀이 자라.\n나무의 둥근 무늬를 가까이 살펴봐.",L"들판 끝까지 바람이 길을 지나가.\n멀리 온 만큼 주변 풍경을 기억해."};
        return id>=1 && id<=5?lines[id-1]:L"";
    }
    bool handle(const sf::Event& e,sf::Vector2f mouse,world_regions::Position p,bool active,bool blocked,bool connected){
        const bool was=visible();state.reconcile(p,active && connected);
        if(e.type==sf::Event::LostFocus){eDown_=false;armed_=0;}
        if(e.type==sf::Event::KeyReleased && e.key.code==sf::Keyboard::E)eDown_=false;
        bool fresh=false;if(e.type==sf::Event::KeyPressed && e.key.code==sf::Keyboard::E){fresh=!eDown_;eDown_=true;}
        if(was && !visible())return true;
        if(blocked){state.close();armed_=0;return was;}
        if(visible()){
            if(fresh || (e.type==sf::Event::KeyPressed && e.key.code==sf::Keyboard::Escape)){state.close();armed_=0;return true;}
            if(e.type==sf::Event::MouseButtonPressed && e.mouseButton.button==sf::Mouse::Left)armed_=closeButton.contains(mouse)?2:0;
            if(e.type==sf::Event::MouseButtonReleased && e.mouseButton.button==sf::Mouse::Left){if(armed_==2 && closeButton.contains(mouse))state.close();armed_=0;}
            return true;
        }
        if(!active || !connected)return false;
        if(e.type==sf::Event::MouseButtonPressed && e.mouseButton.button==sf::Mouse::Left)armed_=prompt.contains(mouse)?1:0;
        bool click=false;if(e.type==sf::Event::MouseButtonReleased && e.mouseButton.button==sf::Mouse::Left){click=armed_==1 && prompt.contains(mouse);armed_=0;}
        return (fresh || click) && state.open(p,active);
    }
    static void box(sf::RenderTarget& out,sf::FloatRect r,sf::Color color={255,248,228}){sf::RectangleShape s({r.width,r.height});s.setPosition(r.left,r.top);s.setFillColor(color);s.setOutlineColor({130,86,54});s.setOutlineThickness(2);out.draw(s);}
    static void text(sf::RenderTarget& out,const sf::Font& font,sf::String s,sf::FloatRect r,unsigned size=20){sf::Text t(s,font,size);t.setFillColor({72,46,37});centerText(t,r);out.draw(t);}
    void draw(sf::RenderTarget& out,const sf::Font& font,world_regions::Position p,bool active)const {
        if(!active)return;
        if(visible()){
            const auto* l=field_investigation::byId(state.target());if(!l)return;
            box(out,{395,390,490,230});text(out,font,std::to_string(l->id)+"  "+l->name,{415,404,450,32},25);
            text(out,font,description(l->id),{415,450,450,66},20);
            text(out,font,sf::String(L"조사 완료 · 좌표 ")+std::to_string(l->x)+", "+std::to_string(l->y),{415,526,450,24},16);
            text(out,font,L"접속 중 기록 · 재접속 초기화",{410,570,270,25},14);box(out,closeButton,{242,212,155});text(out,font,L"닫기 / E",closeButton,18);
        }else if(const auto id=field_investigation::nearest(p)){
            box(out,prompt);text(out,font,sf::String(L"[E] ")+field_investigation::byId(id)->name+L" 조사",prompt,20);
        }
    }
};
