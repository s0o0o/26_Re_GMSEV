#pragma once
#include <SFML/Graphics.hpp>
#include <array>
#include <stdexcept>
namespace world_character {
constexpr float scale=2.f,footX=24.f,footY=46.f;
// UI atlas was generated with nearest-neighbour floor(native * 5/4).
// Select one representative of EACH original pixel; never blur/re-interpolate the result.
inline sf::Image nativeAtlas(const sf::Image& composed,const sf::Image& original,unsigned kind) {
    if(kind>1 || composed.getSize()!=sf::Vector2u(192,48) || original.getSize()!=composed.getSize())
        throw std::invalid_argument("Invalid world character atlas");
    const std::array<unsigned,4> widths=kind ? std::array<unsigned,4>{{32,37,32,32}} : std::array<unsigned,4>{{31,31,31,31}};
    sf::Image result; result.create(192,48,sf::Color::Transparent);
    for(unsigned frame=0;frame<4;++frame) {
        unsigned top=46;
        for(unsigned y=0;y<46;++y) for(unsigned x=0;x<48;++x)
            if(original.getPixel(frame*48+x,y).a) top=(std::min)(top,y);
        if(top==46) throw std::invalid_argument("Empty world character frame");
        const unsigned width=widths[frame],expandedWidth=width*5/4,expandedHeight=46-top;
        const unsigned height=(expandedHeight*4+4)/5;
        const unsigned sourceLeft=(48-expandedWidth)/2,targetLeft=(48-width)/2;
        for(unsigned y=0;y<height;++y) for(unsigned x=0;x<width;++x) {
            const unsigned sx=frame==3 ? expandedWidth-1-((width-1-x)*expandedWidth+width-1)/width : (x*expandedWidth+width-1)/width;
            const unsigned sy=(y*expandedHeight+height-1)/height;
            result.setPixel(frame*48+targetLeft+x,46-height+y,composed.getPixel(frame*48+sourceLeft+sx,top+sy));
        }
    }
    return result;
}
inline void place(sf::Sprite& sprite,float tileX,float tileY) {
    sprite.setScale(scale,scale); sprite.setOrigin(footX,footY);
    sprite.setPosition(tileX+25.f,tileY+46.f);
}
}
