#pragma once
#include "ui_alignment.hpp"
#include "../../shared/combat_rules.hpp"
#include <map>

class CombatHud {
    struct Monster { int hp=5; int maxHp=5; int damage=0; float hitUntil=0; };
    std::map<int,Monster> monsters_;
    sf::String message_;
    float messageUntil_=0;
public:
    void enter(int id,int hp=5,int maxHp=5) {
        Monster state; state.maxHp=maxHp > 0 ? maxHp : 1;
        state.hp=hp < 0 ? 0 : (hp > state.maxHp ? state.maxHp : hp);
        monsters_[id]=state;
    }
    void leave(int id) { monsters_.erase(id); }
    void clear() { monsters_.clear(); messageUntil_=0; }
    int hp(int id) const { auto it=monsters_.find(id); return it == monsters_.end() ? -1 : it->second.hp; }
    void hit(int id,int hp,float now) {
        auto it=monsters_.find(id);
        if(it == monsters_.end()) return;
        auto& m=it->second;
        const int value=hp < 0 ? 0 : (hp > m.maxHp ? m.maxHp : hp);
        m.damage=m.hp-value; m.hp=value; m.hitUntil=now+.45f;
    }
    void feedback(AttackFeedback reason,float now) {
        switch(reason) {
        case AttackFeedback::NoSeeds: message_=L"\uC528\uC557\uC774 \uBD80\uC871\uD574\uC694. \uBA3C\uC800 \uCC44\uC9D1\uD574 \uC8FC\uC138\uC694!"; break;
        case AttackFeedback::NoTarget: message_=L"\uACF5\uACA9\uD560 \uBAAC\uC2A4\uD130\uAC00 \uAC00\uAE4C\uC774 \uC5C6\uC5B4\uC694."; break;
        default: message_=L"\uD074\uB7FD\uD558\uC6B0\uC2A4\uB294 \uC548\uC804 \uAD6C\uC5ED\uC774\uC5D0\uC694."; break;
        }
        messageUntil_=now+2.f;
    }
    void drawMonster(sf::RenderTarget& target,const sf::Font& font,int id,float x,float y,float now) const {
        auto it=monsters_.find(id);
        if(it == monsters_.end() || it->second.hp <= 0) return;
        const auto& m=it->second;
        sf::RectangleShape back({46,7}); back.setPosition(x+1,y-10);
        back.setFillColor({72,46,37}); target.draw(back);
        sf::RectangleShape fill({44.f*m.hp/m.maxHp,5}); fill.setPosition(x+2,y-9);
        fill.setFillColor({128,177,80}); target.draw(fill);
        if(now < m.hitUntil && m.damage > 0) {
            sf::RectangleShape flash({48,48}); flash.setPosition(x,y); flash.setFillColor({255,224,150,65});
            flash.setOutlineColor({240,154,67}); flash.setOutlineThickness(2); target.draw(flash);
            sf::Text amount("-"+std::to_string(m.damage),font,22); amount.setFillColor({153,50,40});
            centerText(amount,{x-2,y-40,52,28}); target.draw(amount);
        }
    }
    void drawMessage(sf::RenderTarget& target,const sf::Font& font,float now) const {
        if(now >= messageUntil_) return;
        sf::RectangleShape panel({620,42}); panel.setPosition(140,585);
        panel.setFillColor({255,248,228,240}); panel.setOutlineColor({130,86,54}); panel.setOutlineThickness(2);
        target.draw(panel);
        sf::Text text(message_,font,22); text.setFillColor({72,46,37}); centerText(text,panel.getGlobalBounds()); target.draw(text);
    }
};
