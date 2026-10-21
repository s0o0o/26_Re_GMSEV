#pragma once
#include <SFML/Graphics.hpp>
#include "../../shared/clubhouse_map.hpp"

class ClubhouseRenderer {
    sf::Texture texture_;
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
    void draw(sf::RenderTarget& target,int left,int top) const {
        sf::RectangleShape backdrop(target.getView().getSize());
        backdrop.setFillColor({48,32,37}); target.draw(backdrop);
        sf::Sprite room(texture_);
        room.setScale(50.f/16,50.f/16);
        room.setPosition(-left*50.f,-top*50.f);
        target.draw(room);
    }
};
