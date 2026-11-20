#pragma once
#include "field_investigation_panel.hpp"
#include "../../shared/resident_quest_log.hpp"
struct ResidentQuestLogView {
    inline static const sf::FloatRect navigation{415,617,260,26};
    static sf::FloatRect row(int index){return {415,265.f+index*70.f,440,62};}
    static const wchar_t* title(int id){return id==1?L"Boss · 두 곳 산책":id==2?L"Boss · 꽃길과 강가":L"Pashmina · 절벽과 그루터기";}
    static const wchar_t* label(resident_quest_log::Status s){using S=resident_quest_log::Status;switch(s){case S::Locked:return L"잠김";case S::Available:return L"수락 가능";case S::Active:return L"진행 중";case S::Ready:return L"보고 준비";default:return L"보고 완료";}}
    static sf::String detail(const resident_quest_log::Row& row){using S=resident_quest_log::Status;
        if(row.status==S::Locked)return L"Boss의 두 곳 산책을 보고하면 열려.\n목록에서는 수락하거나 보고하지 않아.";
        if(row.status==S::Reported)return L"이번 접속의 이야기를 보고했어.\n재접속하면 의뢰 기록도 초기화돼.";
        const sf::String npc=row.id==3?L"Pashmina":L"Boss";
        if(row.status==S::Available)return npc+L"에게 가까이 가서 E 대화 후\nQ로 의뢰를 받아 봐. (보상 없음)";
        if(row.status==S::Ready)return L"거점의 "+npc+L"에게 돌아가\nE 대화 후 Q로 이야기를 보고해 줘.";
        return row.id==1?L"1 꽃길과 2 강가를 방문해 줘.\n두 곳을 다녀온 뒤 Boss에게 보고해.":row.id==2?L"1 꽃길 E 조사 / 2 강가 E 후 F 체험\n끝나면 Boss에게 돌아가 보고해.":L"3 절벽 E 조사 / 4 그루터기 E 후 F 체험\n끝나면 Pashmina에게 돌아가 보고해.";
    }
    static void draw(sf::RenderTarget& out,const sf::Font& font,const resident_quest_log::Snapshot& log,int selected,sf::FloatRect close){using UI=FieldInvestigationPanel;
        UI::box(out,{395,170,490,490});UI::text(out,font,L"접속 중 주민 의뢰",{415,182,440,32},25);
        unsigned active=0,done=0;for(auto r:log.rows){active+=resident_quest_log::running(r.status);done+=r.status==resident_quest_log::Status::Reported;}
        UI::text(out,font,sf::String(L"진행 중 ")+std::to_string(active)+L" · 보고 완료 "+std::to_string(done)+L"/3",{415,225,440,24},18);
        for(int i=0;i<3;++i){auto r=row(i);if(i==selected)UI::box(out,r,{242,212,155});const auto& q=log.rows[i];
            UI::text(out,font,title(q.id),{r.left+8,r.top,280,30},18);UI::text(out,font,label(q.status),{r.left+294,r.top,140,30},17);
            UI::text(out,font,sf::String(q.id==1?L"방문 ":L"조사·체험 ")+std::to_string(q.progress)+L"/2",{r.left+8,r.top+29,425,25},15);}
        UI::text(out,font,detail(log.rows[selected]),{415,490,440,58},17);
        UI::text(out,font,L"방향키 / 행 클릭: 설명 선택",{415,556,440,24},16);
        UI::text(out,font,L"읽기 전용 · 재접속 초기화 · 보상 없음",{415,589,440,22},14);
        UI::box(out,navigation,{242,212,155});UI::text(out,font,L"[Tab] 탐험 기록",navigation,15);
        UI::box(out,close,{242,212,155});UI::text(out,font,L"닫기",close,18);
    }
};
