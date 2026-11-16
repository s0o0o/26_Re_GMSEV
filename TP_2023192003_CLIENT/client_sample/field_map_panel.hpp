#pragma once
#include "ui_alignment.hpp"
#include "../../shared/field_landmarks.hpp"
#include "../../shared/field_discovery.hpp"
#include <string>
class FieldMapPanel {
    bool open_=false,mDown_=false,ready_=false;
    int armed_=0;
    sf::Texture terrain_;
    static void box(sf::RenderTarget& target,sf::FloatRect r,sf::Color color) {
        sf::RectangleShape s({r.width,r.height});s.setPosition(r.left,r.top);s.setFillColor(color);
        s.setOutlineColor({130,86,54});s.setOutlineThickness(2);target.draw(s);
    }
    static void text(sf::RenderTarget& target,const sf::Font& font,const sf::String& value,sf::FloatRect r,unsigned size=20) {
        sf::Text t(value,font,size);t.setFillColor({72,46,37});centerText(t,r);target.draw(t);
    }
    bool prepare() {
        if(ready_) return true;
        sf::Image image;image.create(256,256);
        for(int y=0;y<256;++y) for(int x=0;x<256;++x) {
            sf::Color color;
            switch(field_terrain::at(x,y)) {
            case field_terrain::Tile::Grass: color={126,179,92};break;
            case field_terrain::Tile::Trail: color={228,195,121};break;
            case field_terrain::Tile::Water: color={79,151,220};break;
            case field_terrain::Tile::Bridge: color={182,135,76};break;
            default: color={116,101,76};break;
            }
            image.setPixel(x,y,color);
        }
        ready_=terrain_.loadFromImage(image);terrain_.setSmooth(false);return ready_;
    }
    static void marker(sf::RenderTarget& target,sf::Vector2f p,sf::Color color,float radius=5) {
        sf::RectangleShape s({radius*2,radius*2});s.setOrigin(radius,radius);s.setPosition(p);s.setFillColor(color);
        s.setOutlineColor({72,46,37});s.setOutlineThickness(2);target.draw(s);
    }
public:
    inline static const sf::FloatRect entry{650,124,230,42},closeButton{600,630,190,42};
    inline static const sf::FloatRect mapArea{105,160,432,432};
    bool visible() const { return open_; }
    void reset() { open_=mDown_=false;armed_=0; }
    void regionChanged(world_regions::Position p) { if(p.region!=world_regions::Id::SunnyPeak) reset(); }
    static sf::Vector2f mapPoint(world_regions::Position p) {
        if(p.region!=world_regions::Id::SunnyPeak || !world_regions::valid(p)) return {-1,-1};
        return {mapArea.left+(p.x+.5f)*mapArea.width/256.f,mapArea.top+(p.y+.5f)*mapArea.height/256.f};
    }
    bool handle(const sf::Event& event,sf::Vector2f mouse,world_regions::Position p,bool blocked,bool connected) {
        const bool wasOpen=open_;
        if(event.type==sf::Event::LostFocus) { armed_=0;mDown_=false; }
        if(event.type==sf::Event::KeyReleased && event.key.code==sf::Keyboard::M) mDown_=false;
        bool freshM=false;
        if(event.type==sf::Event::KeyPressed && event.key.code==sf::Keyboard::M) {freshM=!mDown_;mDown_=true;}
        if(!connected || p.region!=world_regions::Id::SunnyPeak || !world_regions::valid(p)) {open_=false;armed_=0;return wasOpen;}
        if(open_) {
            if(freshM || (event.type==sf::Event::KeyPressed && event.key.code==sf::Keyboard::Escape)) open_=false;
            if(event.type==sf::Event::MouseButtonPressed && event.mouseButton.button==sf::Mouse::Left) armed_=closeButton.contains(mouse)?2:0;
            if(event.type==sf::Event::MouseButtonReleased && event.mouseButton.button==sf::Mouse::Left) {if(armed_==2 && closeButton.contains(mouse))open_=false;armed_=0;}
            return true;
        }
        if(blocked) {armed_=0;return false;}
        if(event.type==sf::Event::MouseButtonPressed && event.mouseButton.button==sf::Mouse::Left) armed_=entry.contains(mouse)?1:0;
        bool clicked=false;
        if(event.type==sf::Event::MouseButtonReleased && event.mouseButton.button==sf::Mouse::Left) {clicked=armed_==1 && entry.contains(mouse);armed_=0;}
        if(freshM || clicked) {open_=true;return true;}
        return false;
    }
    void drawEntry(sf::RenderTarget& target,const sf::Font& font,bool disabled,const field_discovery::Record& record={}) const {
        box(target,entry,disabled?sf::Color(221,213,191):sf::Color(255,248,228));
        text(target,font,sf::String(L"[M] \uC9C0\uB3C4 ")+std::to_string(record.count())+"/5",entry,22);
    }
    void draw(sf::RenderTarget& target,const sf::Font& font,world_regions::Position p,const field_discovery::Record& record={}) {
        if(!open_) return;
        sf::RectangleShape dim({900,800});dim.setFillColor({25,24,19,155});target.draw(dim);
        box(target,{70,90,760,600},{255,248,228});
        text(target,font,L"Sunny Peak \uD0D0\uD5D8 \uC9C0\uB3C4",{90,105,720,36},27);
        if(prepare()) {
            sf::Sprite map(terrain_);map.setPosition(mapArea.left,mapArea.top);map.setScale(mapArea.width/256,mapArea.height/256);target.draw(map);
        }
        for(const auto& landmark:field_landmarks::all) {
            const auto pt=mapPoint({world_regions::Id::SunnyPeak,landmark.x,landmark.y});
            marker(target,pt,record.visited(landmark.id)?sf::Color(146,190,113):sf::Color(242,212,155),7);
            text(target,font,std::to_string(landmark.id),{pt.x-10,pt.y-10,20,20},14);
            text(target,font,std::to_string(landmark.id)+"  "+landmark.name,{555,260+(landmark.id-1)*52.f,250,28},19);
            text(target,font,record.visited(landmark.id)?L"\uBC29\uBB38 \uC644\uB8CC":L"\uC544\uC9C1 \uBBF8\uBC29\uBB38",{555,288+(landmark.id-1)*52.f,250,20},15);
        }
        marker(target,mapPoint({world_regions::Id::SunnyPeak,10,12}),{191,140,77},6);
        if(world_regions::valid(p) && p.region==world_regions::Id::SunnyPeak) {
            const auto pt=mapPoint(p);
            sf::RectangleShape player({22,22});player.setOrigin(11,11);player.setPosition(pt);
            player.setFillColor(sf::Color::Transparent);player.setOutlineColor({45,83,158});player.setOutlineThickness(3);target.draw(player);
            text(target,font,"("+std::to_string(p.x)+", "+std::to_string(p.y)+")",{105,598,432,26},17);
        }
        text(target,font,L"\uD30C\uB780 \uD14C\uB450\uB9AC: \uB0B4 \uC704\uCE58",{555,165,250,30},19);
        text(target,font,L"\uAC08\uC0C9 \uC810: \uAC70\uC810 \uCD9C\uAD6C",{555,205,250,30},19);
        text(target,font,sf::String(L"\uD0D0\uD5D8 ")+std::to_string(record.count())+" / 5",{555,540,250,30},22);
        text(target,font,L"\uC811\uC18D \uC911 \uAE30\uB85D",{555,580,250,20},15);
        text(target,font,L"\uC7AC\uC811\uC18D \uC2DC \uCD08\uAE30\uD654",{555,603,250,20},15);
        text(target,font,L"\uB2EB\uC740 \uB4A4 R: \uAC70\uC810 \uADC0\uD658",{105,635,440,28},18);
        box(target,closeButton,{242,212,155});text(target,font,L"\uB2EB\uAE30 (M / Esc)",closeButton,21);
    }
};
