#pragma once
#include "ui_alignment.hpp"
#include "../../shared/town_walk_quest.hpp"
#include "../../shared/resident_experience_quest.hpp"
#include "../../shared/exploration_guidance.hpp"
inline bool drawTownWalkQuest(sf::RenderTarget& target,const sf::Font& font,const town_walk_quest::State& quest,
    const field_discovery::Record& record,const exploration_guidance::State& guide,world_regions::Position p) {
    if(!record.active() || !world_regions::valid(p) || quest.stage()==town_walk_quest::Stage::Available || quest.stage()==town_walk_quest::Stage::Reported)return false;
    sf::String value=sf::String(L"\uc0b0\ucc45 ")+std::to_string(quest.progress(record))+"/2 | ";
    if(quest.stage()==town_walk_quest::Stage::Ready)value+=p.region==world_regions::Id::Clubhouse?L"Boss\uc5d0\uac8c E \ub300\ud654 \ud6c4 Q \ubcf4\uace0 (\uc811\uc18d \uc911)":
        L"R\ub85c \ub3cc\uc544\uac00 Boss\uc5d0\uac8c \ubcf4\uace0\ud574\uc694 (\uc811\uc18d \uc911)";
    else if(const auto* goal=guide.landmark()) value+=sf::String(L"\ubaa9\ud45c ")+std::to_string(goal->id)+" "+goal->name+" | M: "+L"\uc9c0\ub3c4";
    else value+=L"M \uc9c0\ub3c4\uc758 1\ubc88\uacfc 2\ubc88\uc744 \ub458\ub7ec\ubd10\uc694 (\uc811\uc18d \uc911)";
    sf::RectangleShape panel({490,26});panel.setPosition(395,632);panel.setFillColor({255,248,228});
    panel.setOutlineColor({130,86,54});panel.setOutlineThickness(2);target.draw(panel);
    sf::Text text(value,font,17);text.setFillColor({72,46,37});centerText(text,panel.getGlobalBounds());target.draw(text);return true;
}
