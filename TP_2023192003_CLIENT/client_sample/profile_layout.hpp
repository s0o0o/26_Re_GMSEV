#pragma once
#include "ui_alignment.hpp"
namespace profile_layout {
inline constexpr float scale=.9f;
inline constexpr float selfTop(){return 785.f-121.f*scale;}
inline sf::RenderStates selfStates(){sf::RenderStates s;s.transform.translate(15,785).scale(scale,scale).translate(-15,-785);return s;}
inline float stackTop(bool self,int monsters,float profileTop=selfTop()) {
    const int count=(std::max)(0,(std::min)(5,monsters));
    return (self?profileTop:785.f)-66.f*count-(count?8.f:0.f);
}
inline sf::FloatRect monsterSlot(bool self,int index) {return {15,(self?selfTop():785.f)-66.f*(index+1)-8.f,260,60};}
// The legacy profile raster owns NAME and SEED labels. Keep the dynamic name
// inside its own slot; coordinates are in the existing 900x800 logical view.
inline sf::FloatRect nameSlot() { return {242,678,128,31}; }
inline void placeName(sf::Text& text) { centerText(text,nameSlot()); }
}
