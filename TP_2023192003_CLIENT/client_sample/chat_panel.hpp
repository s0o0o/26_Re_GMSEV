#pragma once
#include <SFML/Graphics.hpp>
#include <deque>
#include <vector>
#include <cmath>
#include "../../shared/chat_rules.hpp"
#include "ui_alignment.hpp"
#include "profile_layout.hpp"
class ChatPanel {
    bool tabDown_=false,enterDown_=false,pDown_=false,placed_=false;
    float y_=0;
    std::uint64_t nextSend_=0;
    std::string draft_;
    sf::String notice_;
    std::deque<sf::String> lines_;
    std::size_t offset_=0,unread_=0;
public:
    static std::vector<sf::String> wrap(const sf::String& value,unsigned limit=42) {
        std::vector<sf::String> result;sf::String row;unsigned columns=0;
        for(auto cp:value){const unsigned width=cp>=0x1100?2:1;if(columns+width>limit){result.push_back(row);row.clear();columns=0;}row+=cp;columns+=width;}
        if(!row.isEmpty())result.push_back(row);return result;
    }
    std::vector<sf::String> rows()const {
        std::vector<sf::String> result;for(const auto& message:lines_){auto part=wrap(message);result.insert(result.end(),part.begin(),part.end());}return result;
    }
    std::size_t offset()const{return offset_;}
    std::size_t unread()const{return unread_;}
    void latest(){offset_=unread_=0;}
    void scroll(int amount){const auto n=rows().size();const auto maximum=n>5?n-5:0;
        const auto next=static_cast<long long>(offset_)+amount;offset_=static_cast<std::size_t>((std::max)(0ll,(std::min)(next,static_cast<long long>(maximum))));if(!offset_)unread_=0;}
    static void placeLine(sf::Text& text,sf::FloatRect area) {
        centerText(text,area);const auto b=text.getLocalBounds();
        text.setOrigin(b.left,b.top+b.height/2.f);text.setPosition(area.left,std::round(area.top+area.height/2.f));
    }
    bool editing=false,profileVisible=true;
    void reset(){*this=ChatPanel{};}
    const auto& lines()const{return lines_;}
    const auto& draft()const{return draft_;}
    sf::FloatRect bounds()const{return {15,y_,360,174};}
    float targetY(bool profile,float profileTop=profile_layout::selfTop())const{return (profile?profileTop-12.f:785.f)-174.f;}
    void place(float dt,bool profile,float profileTop=profile_layout::selfTop()) {
        const float target=targetY(profile,profileTop);
        if(!placed_){y_=target;placed_=true;}
        else {y_+=(target-y_)*(1.f-std::exp(-std::min(dt,.1f)*28.f));if(std::abs(y_-target)<.1f)y_=target;}
    }
    void receive(const sf::String& name,const std::string& message) {
        if(!chat_rules::valid(message))return;
        const auto value=name+L": "+sf::String::fromUtf8(message.begin(),message.end());
        if(offset_){offset_+=wrap(value).size();unread_=(std::min)(std::size_t{64},unread_+1);}
        lines_.push_back(value);
        while(lines_.size()>64)lines_.pop_front();
        scroll(0);
    }
    template<class Send> bool handle(const sf::Event& e,sf::Vector2f point,bool blocked,bool connected,std::uint64_t now,Send send) {
        if(e.type==sf::Event::KeyReleased) {
            if(e.key.code==sf::Keyboard::Tab)tabDown_=false;
            if(e.key.code==sf::Keyboard::Enter)enterDown_=false;
            if(e.key.code==sf::Keyboard::P)pDown_=false;
        }
        if(e.type==sf::Event::LostFocus){editing=false;draft_.clear();tabDown_=enterDown_=pDown_=false;return false;}
        if(blocked || !connected){editing=false;draft_.clear();return false;}
        if(e.type==sf::Event::MouseWheelScrolled && e.mouseWheelScroll.wheel==sf::Mouse::VerticalWheel && bounds().contains(point)) {
            scroll(e.mouseWheelScroll.delta>0?3:(e.mouseWheelScroll.delta<0?-3:0));return true;
        }
        if(e.type==sf::Event::KeyPressed && (e.key.code==sf::Keyboard::PageUp || e.key.code==sf::Keyboard::PageDown || e.key.code==sf::Keyboard::End)) {
            if(e.key.code==sf::Keyboard::End)latest();else scroll(e.key.code==sf::Keyboard::PageUp?5:-5);return true;
        }
        if(e.type==sf::Event::MouseButtonReleased && e.mouseButton.button==sf::Mouse::Left && offset_ && sf::FloatRect(283,y_+3,82,22).contains(point)){latest();return true;}
        if(e.type==sf::Event::KeyPressed && e.key.code==sf::Keyboard::P && !editing) {
            if(!pDown_)profileVisible=!profileVisible;pDown_=true;return true;
        }
        if(e.type==sf::Event::KeyPressed && e.key.code==sf::Keyboard::Tab) {
            if(!tabDown_){editing=!editing;draft_.clear();notice_.clear();}tabDown_=true;return true;
        }
        if(e.type==sf::Event::MouseButtonReleased && e.mouseButton.button==sf::Mouse::Left && bounds().contains(point)) {
            editing=true;notice_.clear();return true;
        }
        if(!editing)return false;
        if(e.type==sf::Event::KeyPressed && e.key.code==sf::Keyboard::Escape){editing=false;draft_.clear();return true;}
        if(e.type==sf::Event::TextEntered) {
            notice_.clear();
            auto cp=e.text.unicode;
            if(cp==8)chat_rules::eraseLast(draft_);
            else if(cp>=32 && !(cp>=127 && cp<=159) && cp!=0x2028 && cp!=0x2029 && cp<=0x10ffff && !(cp>=0xd800 && cp<=0xdfff)) {
                auto bytes=sf::String(cp).toUtf8();std::string add(bytes.begin(),bytes.end());
                if(draft_.size()+add.size()<=chat_rules::maxBytes)draft_+=add;
                else notice_=L"최대 119바이트 · 지우고 입력해 줘";
            }
        }
        if(e.type==sf::Event::KeyPressed && e.key.code==sf::Keyboard::Enter) {
            if(!enterDown_ && chat_rules::valid(draft_)) {
                if(now<nextSend_)notice_=L"잠깐! 0.5초 뒤에 보내 줘";
                else if(send(draft_)){nextSend_=now+500;draft_.clear();editing=false;notice_.clear();}
                else notice_=L"전송 불가 · 연결을 확인해 줘";
            }
            enterDown_=true;
        }
        return true;
    }
    void draw(sf::RenderTarget& out,const sf::Font& font,bool connected)const {
        auto box=[&](sf::FloatRect r,sf::Color color){sf::RectangleShape shape({r.width,r.height});shape.setPosition(r.left,r.top);shape.setFillColor(color);shape.setOutlineColor({122,81,50});shape.setOutlineThickness(1);out.draw(shape);};
        auto text=[&](sf::String s,sf::FloatRect r,unsigned size){sf::Text t(s,font,size);t.setFillColor({72,46,37});placeLine(t,r);out.draw(t);};
        box(bounds(),{255,248,228,242});
        if(offset_) {
            text(sf::String(L"이전 기록 · 새 메시지 ")+std::to_string(unread_),{25,y_+5,245,20},14);
            box({283,y_+3,82,22},{242,215,159});text(L"최신 / End",{288,y_+5,72,18},12);
        } else text(connected?L"지역 채팅 · 119B · Tab 입력 / P 프로필":L"지역 채팅  ·  연결 끊김",{25,y_+5,340,20},14);
        if(lines_.empty())text(L"같은 지역의 친구들과 이야기해 봐!",{25,y_+48,340,24},14);
        const auto content=rows();const auto end=content.size()-(std::min)(offset_,content.size());const auto start=end>5?end-5:0;
        for(std::size_t i=start;i<end;++i)text(content[i],{25,y_+29+20.f*(i-start),340,18},15);
        box({24,y_+132,342,32},editing?sf::Color(242,215,159):sf::Color(255,248,228));
        sf::String input=editing?sf::String::fromUtf8(draft_.begin(),draft_.end()):sf::String(L"휠: 이전 기록 / Tab·클릭: 입력");
        if(editing){const auto chunks=wrap(input);if(chunks.size()>1)input=sf::String(L"…")+chunks.back();}
        if(editing && draft_.empty())input=L"입력 · Enter 전송 / Esc 취소";
        if(!notice_.isEmpty())input=notice_;
        text(input,{31,y_+136,328,24},14);
    }
};
