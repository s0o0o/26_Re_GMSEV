#pragma once
#include <SFML/Graphics.hpp>
#include "../../shared/world_layout.hpp"
#include "ui_alignment.hpp"

class ClubhouseRenderer {
    sf::Texture texture_;
    sf::Texture outdoor_;
public:
    bool load(const std::string& path) {
        sf::Image source;
        if (!source.loadFromFile(path) || source.getSize().x < 865 || source.getSize().y < 597) return false;
        source.createMaskFromColor(source.getPixel(0,0));
        sf::Image room;
        room.create(clubhouse::width*16,clubhouse::height*16,sf::Color(48,32,37));
        room.copy(source,0,0,{505,323,360,274},true);
        if (!texture_.loadFromImage(room)) return false;
        texture_.setSmooth(false);
        return true;
    }
    bool loadOutdoor(const std::string& path) {
        sf::Image source,field;
        if (!source.loadFromFile(path) || source.getSize().x < 1241 || source.getSize().y < 289) return false;
        field.create(320,288);
        field.copy(source,0,0,{921,1,320,288});
        if (!outdoor_.loadFromImage(field)) return false;
        outdoor_.setSmooth(false); return true;
    }
    void draw(sf::RenderTarget& target,int left,int top) const {
        const bool field = world_layout::outdoors(left);
        sf::RectangleShape backdrop(target.getView().getSize());
        backdrop.setFillColor({48,32,37}); target.draw(backdrop);
        sf::Sprite room(field ? outdoor_ : texture_);
        room.setScale(50.f/16,50.f/16);
        room.setPosition(((field ? world_layout::fieldX : 0)-left)*50.f,-top*50.f);
        target.draw(room);
        sf::RectangleShape portal({42,12});
        portal.setFillColor({242,212,155}); portal.setOutlineColor({130,86,54}); portal.setOutlineThickness(2);
        portal.setPosition(((field ? world_layout::fieldX : 0)+10-left)*50.f+4,(field ? 12-top : 14-top)*50.f+28);
        target.draw(portal);
    }
    void drawLabel(sf::RenderTarget& target,const sf::Font& font,int playerX) const {
        sf::RectangleShape label({310,38}); label.setPosition(295,12);
        label.setFillColor({255,248,228,235}); label.setOutlineColor({130,86,54}); label.setOutlineThickness(2);
        target.draw(label);
        sf::Text text(world_layout::outdoors(playerX) ? L"Sunny Peak / \uC57C\uC678" : L"Club House / \uD074\uB7FD\uD558\uC6B0\uC2A4",font,20);
        text.setFillColor({72,46,37}); centerText(text,label.getGlobalBounds()); target.draw(text);
        sf::Text hint(L"\uAE08\uC0C9 \uBC1C\uD310\uC744 \uBC1F\uC73C\uBA74 \uC9C0\uC5ED \uC774\uB3D9",font,16);
        hint.setFillColor({72,46,37});
        sf::RectangleShape help({330,28}); help.setPosition(285,58); help.setFillColor({255,248,228,220});
        target.draw(help); centerText(hint,help.getGlobalBounds()); target.draw(hint);
    }
    void drawPickups(sf::RenderTarget& target,const int* tiles,int columns,int rows) const {
        for(int y=0;y<rows;++y) for(int x=0;x<columns;++x) {
            if (tiles[y*columns+x] != 4) continue;
            sf::RectangleShape shadow({24,5}); shadow.setFillColor({70,80,35,90});
            shadow.setPosition(x*50.f+13,y*50.f+38); target.draw(shadow);
            sf::ConvexShape seed(6);
            const sf::Vector2f points[]={{25,10},{36,23},{32,35},{25,40},{18,35},{14,23}};
            for(int i=0;i<6;++i) seed.setPoint(i,points[i]);
            seed.setPosition(x*50.f,y*50.f); seed.setFillColor({147,93,48});
            seed.setOutlineColor({72,46,37}); seed.setOutlineThickness(2); target.draw(seed);
            sf::RectangleShape stripe({5,20}); stripe.setFillColor({255,235,167});
            stripe.setPosition(x*50.f+23,y*50.f+16); target.draw(stripe);
        }
    }
};
