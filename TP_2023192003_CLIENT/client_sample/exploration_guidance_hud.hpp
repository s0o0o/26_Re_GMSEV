#pragma once
#include "ui_alignment.hpp"
#include "../../shared/exploration_guidance.hpp"
inline void drawExplorationGuidance(sf::RenderTarget& target,const sf::Font& font,const exploration_guidance::State& guide,world_regions::Position p) {
    if(!guide.active() || !world_regions::valid(p))return;
    sf::String value;
    if(const auto* goal=guide.landmark()) {
        value=sf::String(L"\ubaa9\ud45c ")+std::to_string(goal->id)+" "+goal->name+" | ";
        if(p.region==world_regions::Id::Clubhouse)value+=L"R: \ud544\ub4dc\ub85c \ub098\uac00\uc694";
        else if(guide.distance(p)>=0)value+=sf::String(guide.direction(p))+L" | \uc88c\ud45c \uac70\ub9ac "+std::to_string(guide.distance(p));
        else return;
    } else value=L"\ud0d0\ud5d8 \uc9c0\uc810 5\uacf3\uc744 \ubaa8\ub450 \ub458\ub7ec\ubd24\uc5b4\uc694!";
    sf::RectangleShape panel({490,26});panel.setPosition(395,632);panel.setFillColor({255,248,228});
    panel.setOutlineColor({130,86,54});panel.setOutlineThickness(2);target.draw(panel);
    sf::Text text(value,font,17);text.setFillColor({72,46,37});centerText(text,panel.getGlobalBounds());target.draw(text);
}
