#pragma once
#include <SFML/Graphics.hpp>
#include "../../shared/town_npcs.hpp"
#include "ui_alignment.hpp"
#include "town_motion.hpp"
#include <array>
#include <string>
class TownNpcRenderer {
    std::array<sf::Texture,3> textures_;
public:
    // Original sheets are preserved; SFML renders source rectangles directly.
    static sf::IntRect frame(int type,float seconds=0,town_motion::Facing facing=town_motion::Down) {
        const int phase=static_cast<int>((std::max)(0.f,seconds)*3)%2;
        switch(type) {
        case 100:
            if(facing==town_motion::Up) return {150,179,37,39};
            if(facing==town_motion::Left || facing==town_motion::Right) return {200,130,38,38};
            return {3+phase*50,79,30,39};
        case 101:
            if(facing==town_motion::Up) return {80,5,35,33};
            if(facing==town_motion::Left || facing==town_motion::Right) return {41,5,35,33};
            return phase ? sf::IntRect(157,5,35,33) : sf::IntRect(3,5,35,33);
        case 102: return {57+phase*44,56,43,37};
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
    void draw(sf::RenderTarget& target,int type,float tileX,float tileY,const sf::Font& font,const std::string& name,float seconds=0,town_motion::Facing facing=town_motion::Down) const {
        if(!town_npcs::isResident(type)) return;
        const auto rect=frame(type,seconds,facing);
        sf::Sprite sprite(texture(type),rect);
        sprite.setScale(facing==town_motion::Right && type!=102 ? -2.f : 2.f,2.f); sprite.setOrigin(rect.width/2.f,static_cast<float>(rect.height));
        sprite.setPosition(tileX+25.f,tileY+46.f); target.draw(sprite);
        sf::Text text(name,font,18); text.setFillColor({72,46,37});
        text.setOutlineColor({255,248,228}); text.setOutlineThickness(2);
        centerText(text,{tileX-55.f,tileY-52.f,160.f,24.f}); target.draw(text);
    }
};
