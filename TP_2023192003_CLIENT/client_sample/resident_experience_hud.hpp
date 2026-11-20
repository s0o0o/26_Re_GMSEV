#pragma once
#include "field_investigation_panel.hpp"
#include "../../shared/resident_experience_guidance.hpp"
inline sf::String experienceNextText(resident_experience_guidance::Goal goal,world_regions::Position p){
    using Kind=resident_experience_guidance::Kind;
    if(goal.kind==Kind::Report){const sf::String name=goal.residentType==101?"Pashmina":"Boss";return p.region==world_regions::Id::Clubhouse?name+L"에게 E / Q 체험 보고":L"R로 귀환 후 "+name+L" E / Q 보고";}
    if(goal.kind==Kind::None)return {};
    const sf::String place=goal.target==3?L"3 절벽 아래":goal.target==4?L"4 그루터기":goal.target==1?L"1 꽃길":L"2 강가";
    sf::String value=place+(goal.kind==Kind::Inspect?L"에서 E 조사":L"에서 E 연 뒤 F 체험");
    if(p.region!=world_regions::Id::SunnyPeak)value=L"R로 필드 이동 / "+value;
    return value;
}
inline bool drawResidentExperienceQuest(sf::RenderTarget& out,const sf::Font& font,const resident_experience_quest::State& quest,
    const field_investigation::State& actions,const field_discovery::Record& record,world_regions::Position p){
    const auto goal=resident_experience_guidance::next(record.active(),quest,actions);
    if(goal.kind==resident_experience_guidance::Kind::None || !world_regions::valid(p))return false;
    const sf::FloatRect r{395,632,490,26};FieldInvestigationPanel::box(out,r);
    FieldInvestigationPanel::text(out,font,sf::String(L"체험 ")+std::to_string(quest.progress(actions))+L"/2 | "+experienceNextText(goal,p)+L" · J 수첩",r,16);return true;
}
