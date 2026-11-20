#pragma once
#include "field_investigation_panel.hpp"
#include "../../shared/field_journal.hpp"
#include "resident_experience_hud.hpp"
#include "resident_quest_log_view.hpp"
class FieldJournalPanel {
    bool open_=false,jDown_=false;int armed_=0,selected_=1;
    bool questTab_=false,tabDown_=false;int selectedQuest_=0;
    bool enterDown_=false,aDown_=false;int trackRequest_=-1;
    bool activeOnly_=false,fDown_=false;
public:
    inline static const sf::FloatRect entry{650,174,230,38},closeButton{710,616,150,30};
    static sf::FloatRect row(int id){return {415,285.f+(id-1)*40.f,440,38};}
    bool visible()const{return open_;}int selected()const{return selected_;}
    bool questTab()const{return questTab_;}int selectedQuest()const{return selectedQuest_;}
    int takeTrackRequest(){const int r=trackRequest_;trackRequest_=-1;return r;}
    bool activeOnly()const{return activeOnly_;}
    void reset(){open_=jDown_=questTab_=tabDown_=enterDown_=aDown_=activeOnly_=fDown_=false;armed_=0;selected_=1;selectedQuest_=0;trackRequest_=-1;}
    bool handle(const sf::Event& e,sf::Vector2f mouse,bool active,bool blocked,const field_discovery::Record& record,const field_investigation::State& state,int preferredTarget=0,const resident_quest_log::Snapshot& log={}){
        if(e.type==sf::Event::LostFocus){jDown_=tabDown_=enterDown_=aDown_=fDown_=false;armed_=0;}
        if(e.type==sf::Event::KeyReleased && e.key.code==sf::Keyboard::J)jDown_=false;
        if(e.type==sf::Event::KeyReleased && e.key.code==sf::Keyboard::Tab)tabDown_=false;
        if(e.type==sf::Event::KeyReleased && e.key.code==sf::Keyboard::Enter)enterDown_=false;
        if(e.type==sf::Event::KeyReleased && e.key.code==sf::Keyboard::A)aDown_=false;
        if(e.type==sf::Event::KeyReleased && e.key.code==sf::Keyboard::F)fDown_=false;
        bool fresh=false;if(e.type==sf::Event::KeyPressed && e.key.code==sf::Keyboard::J){fresh=!jDown_;jDown_=true;}
        const bool was=open_;if(!active || blocked){open_=false;armed_=0;return was;}
        if(open_){
            if(fresh || (e.type==sf::Event::KeyPressed && e.key.code==sf::Keyboard::Escape)){open_=false;return true;}
            if(e.type==sf::Event::KeyPressed && e.key.code==sf::Keyboard::Tab){if(!tabDown_ && log.active){questTab_=!questTab_;armed_=0;}tabDown_=true;return true;}
            if(log.active && e.type==sf::Event::MouseButtonPressed && e.mouseButton.button==sf::Mouse::Left && ResidentQuestLogView::navigation.contains(mouse)){armed_=8;return true;}
            if(e.type==sf::Event::MouseButtonReleased && e.mouseButton.button==sf::Mouse::Left && armed_==8){if(log.active && ResidentQuestLogView::navigation.contains(mouse))questTab_=!questTab_;armed_=0;return true;}
            if(questTab_ && log.active){
auto normalize=[&](){const auto v=resident_quest_log::visible(log,activeOnly_);if(v.position(selectedQuest_)<0){const int next=v.count?v.indices[0]:-1;if(next!=selectedQuest_)armed_=0;selectedQuest_=next;}return v;};
                if(e.type==sf::Event::KeyPressed && e.key.code==sf::Keyboard::F){if(!fDown_){activeOnly_=!activeOnly_;armed_=0;normalize();}fDown_=true;return true;}
                const auto visible=normalize();
                if(e.type==sf::Event::KeyPressed && e.key.code==sf::Keyboard::Enter){if(!enterDown_ && selectedQuest_>=0 && resident_quest_log::running(log.rows[selectedQuest_].status))trackRequest_=selectedQuest_+1;enterDown_=true;return true;}
                if(e.type==sf::Event::KeyPressed && e.key.code==sf::Keyboard::A){if(!aDown_)trackRequest_=0;aDown_=true;return true;}
                if(e.type==sf::Event::KeyPressed && (e.key.code==sf::Keyboard::Up || e.key.code==sf::Keyboard::Down)){if(armed_==20)armed_=0;if(visible.count){const int at=visible.position(selectedQuest_);selectedQuest_=visible.indices[(std::max)(0,(std::min)(visible.count-1,at+(e.key.code==sf::Keyboard::Up?-1:1)))];}}
                if(e.type==sf::Event::MouseButtonPressed && e.mouseButton.button==sf::Mouse::Left){armed_=closeButton.contains(mouse)?7:ResidentQuestLogView::filterButton.contains(mouse)?22:ResidentQuestLogView::autoButton.contains(mouse)?21:ResidentQuestLogView::trackButton.contains(mouse)?20:0;for(int i=0;i<visible.count;++i)if(ResidentQuestLogView::row(i).contains(mouse))armed_=10+visible.indices[i];}
                if(e.type==sf::Event::MouseButtonReleased && e.mouseButton.button==sf::Mouse::Left){if(armed_==7 && closeButton.contains(mouse))open_=false;if(armed_==22 && ResidentQuestLogView::filterButton.contains(mouse)){activeOnly_=!activeOnly_;normalize();}if(armed_==20 && ResidentQuestLogView::trackButton.contains(mouse) && selectedQuest_>=0 && resident_quest_log::running(log.rows[selectedQuest_].status))trackRequest_=selectedQuest_+1;if(armed_==21 && ResidentQuestLogView::autoButton.contains(mouse))trackRequest_=0;for(int i=0;i<visible.count;++i)if(armed_==10+visible.indices[i] && ResidentQuestLogView::row(i).contains(mouse))selectedQuest_=visible.indices[i];armed_=0;}return true;
            }
            if(e.type==sf::Event::KeyPressed && e.key.code==sf::Keyboard::Up)selected_=(std::max)(1,selected_-1);
            if(e.type==sf::Event::KeyPressed && e.key.code==sf::Keyboard::Down)selected_=(std::min)(5,selected_+1);
            if(e.type==sf::Event::MouseButtonPressed && e.mouseButton.button==sf::Mouse::Left){armed_=closeButton.contains(mouse)?7:0;for(int id=1;id<=5;++id)if(row(id).contains(mouse))armed_=id+1;}
            if(e.type==sf::Event::MouseButtonReleased && e.mouseButton.button==sf::Mouse::Left){if(armed_==7 && closeButton.contains(mouse))open_=false;for(int id=1;id<=5;++id)if(armed_==id+1 && row(id).contains(mouse))selected_=id;armed_=0;}
            return true;
        }
        if(e.type==sf::Event::MouseButtonPressed && e.mouseButton.button==sf::Mouse::Left)armed_=entry.contains(mouse)?1:0;
        bool click=false;if(e.type==sf::Event::MouseButtonReleased && e.mouseButton.button==sf::Mouse::Left){click=armed_==1 && entry.contains(mouse);armed_=0;}
        if(fresh || click){open_=true;selected_=preferredTarget>=1 && preferredTarget<=5?preferredTarget:field_journal::next(record,state);if(!selected_)selected_=1;return true;}return false;
    }
    void draw(sf::RenderTarget& out,const sf::Font& font,world_regions::Position p,const field_discovery::Record& record,const field_investigation::State& state,bool disabled=false,const resident_experience_quest::State* quest=nullptr,const resident_quest_log::Snapshot& log={})const {
        if(!record.active())return;using UI=FieldInvestigationPanel;
        if(!open_){UI::box(out,entry,disabled?sf::Color(221,213,191):sf::Color(255,248,228));UI::text(out,font,L"[J] 탐험 수첩",entry,20);return;}
        if(questTab_ && log.active){ResidentQuestLogView::draw(out,font,log,selectedQuest_,closeButton,activeOnly_);return;}
        UI::box(out,{395,170,490,490});UI::text(out,font,L"접속 중 탐험 수첩",{415,182,440,32},25);
        UI::text(out,font,sf::String(L"방문 ")+std::to_string(record.count())+L"/5 · 조사 "+std::to_string(field_journal::inspected(state))+L"/5 · 체험 "+std::to_string(field_journal::completed(state))+L"/5",{415,225,440,24},18);
        UI::text(out,font,L"장소 / 방향키로 선택",{415,260,260,22},15);UI::text(out,font,L"방문   조사   체험",{685,260,170,22},15);
        for(const auto& l:field_landmarks::all){if(l.id==selected_)UI::box(out,row(l.id),{242,212,155});UI::text(out,font,std::to_string(l.id)+" "+l.name,{415,row(l.id).top,260,36},17);
            const bool done[]={record.visited(l.id),state.inspected(l.id),state.completed(l.id)};for(int i=0;i<3;++i)UI::text(out,font,done[i]?L"O":L"-",{685+i*56.f,row(l.id).top,56,36},18);}
        UI::text(out,font,UI::description(selected_),{415,490,440,56},17);
        sf::String next=L"다섯 곳의 체험 완료! 보상은 없어.";
        if(const auto id=field_journal::next(record,state)){const auto* l=field_investigation::byId(id);next=sf::String(L"다음 ")+std::to_string(id)+" "+l->name;
            const auto d=field_journal::distance(p,id);next+=d>=0?sf::String(L" · 좌표 거리 ")+std::to_string(d):sf::String(L" · R로 필드 이동");}
        sf::String limit=L"읽기로 완료되지 않아 · 재접속 초기화";
        if(quest){const auto goal=resident_experience_guidance::next(record.active(),*quest,state);
            if(goal.kind!=resident_experience_guidance::Kind::None){next=L"의뢰: "+experienceNextText(goal,p);
                limit=sf::String(L"체험 의뢰 ")+std::to_string(quest->progress(state))+L"/2 · 재접속 초기화 · 보상 없음";}}
        if(log.goal.questId){next=sf::String(log.pinned?L"추적: ":L"의뢰: ")+questGoalText(log.goal,p);}
        UI::text(out,font,next,{410,556,460,26},16);UI::text(out,font,limit,{410,589,460,22},14);
        if(log.active){UI::box(out,ResidentQuestLogView::navigation,{242,212,155});UI::text(out,font,L"[Tab] 주민 의뢰",ResidentQuestLogView::navigation,15);}else UI::text(out,font,L"J / Esc 닫기",{415,617,260,26},15);UI::box(out,closeButton,{242,212,155});UI::text(out,font,L"닫기",closeButton,18);
    }
};
