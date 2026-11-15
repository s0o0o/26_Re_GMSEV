#pragma once
#include <SFML/Graphics.hpp>
#include "../../shared/travel_client_state.hpp"
#include "ui_alignment.hpp"

class TravelPanel {
public:
    sf::FloatRect bounds{650,12,230,44};
    bool pressed=false;
    bool rDown=false;
    bool activate(const sf::Event& event,sf::Vector2f mouse,bool chat,bool enabled) {
        if(event.type==sf::Event::LostFocus) { pressed=false; rDown=false; }
        if(event.type==sf::Event::KeyReleased && event.key.code==sf::Keyboard::R) rDown=false;
        if(event.type==sf::Event::KeyPressed && event.key.code==sf::Keyboard::R) {
            const bool fresh=!rDown; rDown=true;
            return fresh && !chat && enabled;
        }
        if(chat || !enabled) { pressed=false; return false; }
        if(event.type==sf::Event::MouseButtonPressed && event.mouseButton.button==sf::Mouse::Left)
            pressed=bounds.contains(mouse);
        if(event.type==sf::Event::MouseButtonReleased && event.mouseButton.button==sf::Mouse::Left) {
            const bool accepted=pressed && bounds.contains(mouse); pressed=false; return accepted;
        }
        return false;
    }
    void draw(sf::RenderTarget& target,sf::Font& font,world_regions::Id region,
              const world_travel::ClientState& state,sf::Vector2f mouse={-1,-1},bool chat=false) const {
        using world_travel::ClientStatus;
        const bool disabled=chat || state.pending() || state.status==ClientStatus::Disconnected;
        sf::RectangleShape button({bounds.width,bounds.height});
        button.setPosition(bounds.left,bounds.top);
        button.setFillColor(disabled ? sf::Color(224,216,190)
            : pressed ? sf::Color(224,183,92) : bounds.contains(mouse) ? sf::Color(255,231,166) : sf::Color(255,245,216));
        button.setOutlineColor({130,79,28}); button.setOutlineThickness(-2);
        target.draw(button);
        sf::String label=state.pending() ? sf::String(L"\uC774\uB3D9 \uD655\uC778 \uC911...")
            : region==world_regions::Id::Clubhouse ? sf::String(L"[R] \uD544\uB4DC\uB85C \uB098\uAC00\uAE30")
            : sf::String(L"[R] \uAC70\uC810\uC73C\uB85C \uADC0\uD658");
        sf::Text text(label,font,22); text.setFillColor({66,39,24}); centerText(text,bounds); target.draw(text);
        sf::String message;
        switch(state.status) {
        case ClientStatus::Pending: message=L"\uC11C\uBC84 \uC751\uB2F5\uC744 \uAE30\uB2E4\uB824\uC694"; break;
        case ClientStatus::Arrived: message=L"\uC774\uB3D9 \uD655\uC778 \uC644\uB8CC"; break;
        case ClientStatus::Rejected: message=L"\uC774\uB3D9 \uBD88\uAC00\nR\uD0A4\uB85C \uB2E4\uC2DC \uC2DC\uB3C4\uD558\uC138\uC694"; break;
        case ClientStatus::TimedOut: message=L"\uC751\uB2F5 \uC2DC\uAC04 \uCD08\uACFC\nR\uD0A4\uB85C \uB2E4\uC2DC \uC2DC\uB3C4\uD558\uC138\uC694"; break;
        case ClientStatus::Disconnected: message=L"\uC11C\uBC84 \uC5F0\uACB0 \uC885\uB8CC\nESC\uB85C \uC885\uB8CC \uD6C4 \uC7AC\uC811\uC18D"; break;
        default: if(chat) message=L"\uCC44\uD305 \uC885\uB8CC \uD6C4 \uC774\uB3D9\uD558\uC138\uC694"; break;
        }
        if(!message.isEmpty()) {
            sf::RectangleShape strip({230,52}); strip.setPosition(650,62); strip.setFillColor({255,245,216,245}); target.draw(strip);
            sf::Text status(message,font,16); status.setFillColor({66,39,24});
            centerText(status,{654,64,222,48}); target.draw(status);
        }
    }
};
