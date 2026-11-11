#pragma once
#include <SFML/Graphics.hpp>
#include "../../shared/town_npcs.hpp"
#include "ui_alignment.hpp"
#include <array>
#include <string>
class TownNpcRenderer {
    std::array<sf::Texture,3> textures_;
public:
    // Original sheets are preserved; SFML renders source rectangles directly.
    static sf::IntRect frame(int type) {
        switch(type) {
        case 100: return {3,79,30,39};
        case 101: return {3,5,35,33};
        case 102: return {76,6,43,37};
        default: return {};
        }
    }
    bool load(const std::string& directory) {
        for(std::size_t i=0;i<textures_.size();++i) {
            sf::Image image;
            if(!image.loadFromFile(directory+"/"+town_npcs::residents[i].asset+".png")) return false;
            const auto key=image.getPixel(image.getSize().x-1,0);
            if(key.a) image.createMaskFromColor(key); // Only in-memory; original PNG is unchanged.
            const auto rect=frame(town_npcs::residents[i].type);
            if(rect.left<0 || rect.top<0 || rect.left+rect.width>static_cast<int>(image.getSize().x) || rect.top+rect.height>static_cast<int>(image.getSize().y)) return false;
            if(!textures_[i].loadFromImage(image)) return false;
            textures_[i].setSmooth(false);
        }
        return true;
    }
    const sf::Texture& texture(int type) const { return textures_.at(type-town_npcs::firstType); }
    void draw(sf::RenderTarget& target,int type,float tileX,float tileY,const sf::Font& font,const std::string& name) const {
        if(!town_npcs::isResident(type)) return;
        const auto rect=frame(type);
        sf::Sprite sprite(texture(type),rect);
        sprite.setScale(2.f,2.f); sprite.setOrigin(rect.width/2.f,static_cast<float>(rect.height));
        sprite.setPosition(tileX+25.f,tileY+46.f); target.draw(sprite);
        sf::Text text(name,font,18); text.setFillColor({72,46,37});
        text.setOutlineColor({255,248,228}); text.setOutlineThickness(2);
        centerText(text,{tileX-55.f,tileY-52.f,160.f,24.f}); target.draw(text);
    }
};
