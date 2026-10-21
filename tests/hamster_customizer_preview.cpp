#include "../TP_2023192003_CLIENT/client_sample/hamster_customizer.hpp"
#include <cassert>
#include <iostream>

int main() {
    HamsterCustomizer customizer;
    assert(customizer.load("TP_2023192003_CLIENT/client_sample/character.png"));
    sf::Font font;
    assert(font.loadFromFile("TP_2023192003_CLIENT/client_sample/neodgm.ttf"));
    sf::RenderTexture target;
    assert(target.create(900, 800));
    sf::Texture background;
    assert(background.loadFromFile("TP_2023192003_CLIENT/client_sample/main.png"));
    sf::RenderWindow inputWindow;
    sf::Event next{};
    next.type = sf::Event::KeyPressed;
    next.key.code = sf::Keyboard::Right;
    for (int i = 0; i < HAMSTER_FUR_COLORS; ++i) {
        assert(customizer.selected() == i);
        target.clear(sf::Color(226, 239, 213));
        target.draw(sf::Sprite(background));
        customizer.draw(target, font);
        target.display();
        assert(target.getTexture().copyToImage().saveToFile("build/tests/hamster-preview-" + std::to_string(i) + ".png"));
        customizer.handleEvent(next, inputWindow);
    }
    assert(customizer.selected() == 0);
    const auto original = customizer.texture(0).copyToImage();
    const auto silver = customizer.texture(3).copyToImage();
    int changed = 0;
    for (unsigned y = 0; y < 48; ++y) for (unsigned x = 0; x < 192; ++x) {
        const auto p = original.getPixel(x, y);
        const auto q = silver.getPixel(x, y);
        assert(p.a == q.a);
        if (p.r == p.g && p.g == p.b) assert(p == q);
        if (p != q) ++changed;
    }
    assert(changed > 0);
    assert(&customizer.texture(255) == &customizer.texture(0));
    std::cout << "Customizer previews and palette checks passed.\n";
}
