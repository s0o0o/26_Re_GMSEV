#include "../TP_2023192003_CLIENT/client_sample/window_display.hpp"
#include "../TP_2023192003_CLIENT/client_sample/hamster_customizer.hpp"
#include <cassert>
#include <iostream>
int main() {
    HamsterCustomizer customizer;
    assert(customizer.load("TP_2023192003_CLIENT/client_sample/assets/characters"));
    sf::Texture background; assert(background.loadFromFile("TP_2023192003_CLIENT/client_sample/main.png"));
    assert(customizer.setBackdrop(background));
    sf::Font font; assert(font.loadFromFile("TP_2023192003_CLIENT/client_sample/neodgm.ttf"));
    sf::RenderWindow window(sf::VideoMode(900,800),"Display regression");
    WindowDisplay display(window,"Display regression"); window.setVisible(false);
    for(auto size : {sf::Vector2u(900,800),{1280,720},{1920,1080},{600,1000},{3440,1440}}) {
        window.setSize(size); WindowDisplay::resize(window);
        const auto actual=window.getSize();
        const auto fit=display_layout::fit(actual.x,actual.y);
        const sf::Vector2i pixel(static_cast<int>(fit.left+540*fit.scale),static_cast<int>(fit.top+135*fit.scale));
        const auto logical=window.mapPixelToCoords(pixel);
        assert(std::abs(logical.x-540)<3 && std::abs(logical.y-135)<3);
        sf::Event click{}; click.type=sf::Event::MouseButtonPressed; click.mouseButton.button=sf::Mouse::Left;
        click.mouseButton.x=pixel.x; click.mouseButton.y=pixel.y;
        assert(!display.handle(click,window)); customizer.handleEvent(click,window); assert(customizer.selectedCharacter()==1);
        sf::RenderTexture render; assert(render.create(size.x,size.y));
        auto view=window.getView(); // Same production view, but render at the requested output size.
        const auto renderFit=display_layout::fit(size.x,size.y);
        view.setViewport({renderFit.left/size.x,renderFit.top/size.y,renderFit.width/size.x,renderFit.height/size.y});
        render.setView(view); render.clear(sf::Color(25,20,18)); customizer.draw(render,font); render.display();
        assert(render.getTexture().copyToImage().saveToFile("build/tests/display-"+std::to_string(size.x)+"x"+std::to_string(size.y)+".png"));
    }
    window.setSize({960,720}); window.setPosition({40,40}); WindowDisplay::resize(window);
    const auto saved=window.getSize(); const auto position=window.getPosition();
    sf::Event key{}; key.type=sf::Event::KeyPressed; key.key.code=sf::Keyboard::F11;
    assert(display.handle(key,window) && display.fullscreen());
    assert(display.handle(key,window) && display.fullscreen()); // Ignore key auto-repeat.
    key.type=sf::Event::KeyReleased; display.handle(key,window);
    key.type=sf::Event::KeyPressed; assert(display.handle(key,window) && !display.fullscreen());
    assert(window.getSize()==saved && window.getPosition()==position);
    key.type=sf::Event::KeyReleased; display.handle(key,window);
    key.type=sf::Event::KeyPressed; display.handle(key,window);
    key.key.code=sf::Keyboard::Escape; assert(display.handle(key,window) && !display.fullscreen() && window.isOpen());
    window.clear(); customizer.draw(window,font); window.display(); // Assets survive window recreation.
    window.close(); std::cout<<"Display: SFML sizing, click mapping, render output, F11/escape and window restoration passed\n";
}
