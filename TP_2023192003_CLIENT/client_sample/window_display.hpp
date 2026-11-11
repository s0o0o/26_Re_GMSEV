#pragma once
#include <SFML/Graphics.hpp>
#include "display_layout.hpp"
#include <string>

// Presentation only: the server's visible tile range is independent of resolution.
class WindowDisplay {
    bool fullscreen_=false;
    bool f11Down_=false;
    sf::Vector2u savedSize_;
    sf::Vector2i savedPosition_;
    std::string title_;
public:
    explicit WindowDisplay(sf::RenderWindow& window,std::string title) : title_(std::move(title)) {
        const auto desktop=sf::VideoMode::getDesktopMode();
        window.setSize({(std::max)(1u,desktop.width*9/10),(std::max)(1u,desktop.height*8/10)});
        window.setPosition({static_cast<int>((desktop.width-window.getSize().x)/2),static_cast<int>((desktop.height-window.getSize().y)/2)});
        configure(window);
    }
    static void configure(sf::RenderWindow& window) {
        window.setFramerateLimit(60);
        resize(window);
    }
    static void resize(sf::RenderWindow& window) {
        const auto size=window.getSize();
        if(!size.x || !size.y) return;
        const auto fit=display_layout::fit(size.x,size.y);
        sf::View view(sf::FloatRect(0,0,display_layout::width,display_layout::height));
        view.setViewport({fit.left/size.x,fit.top/size.y,fit.width/size.x,fit.height/size.y});
        window.setView(view);
    }
    static bool acceptsClick(const sf::RenderWindow& window,int x,int y) {
        const auto size=window.getSize();
        return display_layout::fit(size.x,size.y).contains(static_cast<float>(x),static_cast<float>(y));
    }
    bool fullscreen() const { return fullscreen_; }
    void toggle(sf::RenderWindow& window) {
        if(!fullscreen_) {
            savedSize_=window.getSize(); savedPosition_=window.getPosition();
            window.create(sf::VideoMode::getDesktopMode(),title_,sf::Style::None);
            window.setPosition({0,0}); fullscreen_=true;
        } else {
            window.create(sf::VideoMode(savedSize_.x,savedSize_.y),title_,sf::Style::Default);
            window.setPosition(savedPosition_); fullscreen_=false;
        }
        configure(window);
    }
    // Consume presentation events before login/chat/game input.
    bool handle(const sf::Event& event,sf::RenderWindow& window) {
        if(event.type==sf::Event::KeyReleased && event.key.code==sf::Keyboard::F11) { f11Down_=false; return true; }
        if(event.type==sf::Event::Closed) { window.close(); return true; }
        if(event.type==sf::Event::Resized) { resize(window); return true; }
        if(event.type==sf::Event::KeyPressed &&
           (event.key.code==sf::Keyboard::F11 || (fullscreen_ && event.key.code==sf::Keyboard::Escape))) {
            if(event.key.code==sf::Keyboard::F11) {
                if(f11Down_) return true;
                f11Down_=true;
            }
            toggle(window); return true;
        }
        if(event.type==sf::Event::MouseButtonPressed)
            return !acceptsClick(window,event.mouseButton.x,event.mouseButton.y);
        return false;
    }
};
