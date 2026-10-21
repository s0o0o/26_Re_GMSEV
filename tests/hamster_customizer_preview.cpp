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
    assert(&customizer.texture(255,255,255) == &customizer.texture(0));
    sf::Image source;
    assert(source.loadFromFile("TP_2023192003_CLIENT/client_sample/character.png"));
    for (unsigned y=0;y<48;++y) for (unsigned x=0;x<192;++x)
        assert(original.getPixel(x,y) == source.getPixel(x,y));
    for (std::uint8_t f=0;f<HAMSTER_FUR_COLORS;++f)
      for (std::uint8_t e=0;e<HAMSTER_EYE_STYLES;++e)
        for (std::uint8_t s=0;s<HAMSTER_SCARF_STYLES;++s) {
            const auto variant = customizer.texture(f,e,s).copyToImage();
            assert(variant.getSize() == source.getSize());
            for (unsigned y=0;y<48;++y) for (unsigned x=0;x<192;++x)
                assert(variant.getPixel(x,y).a == source.getPixel(x,y).a);
        }
    next.key.code = sf::Keyboard::F2;
    customizer.handleEvent(next,inputWindow);
    assert(customizer.selectedEyes() == 1);
    next.key.code = sf::Keyboard::F3;
    customizer.handleEvent(next,inputWindow);
    assert(customizer.selectedEyes() == 2);
    next.key.code = sf::Keyboard::F4;
    customizer.handleEvent(next,inputWindow);
    assert(customizer.selectedScarf() == 1);
    target.clear();
    target.draw(sf::Sprite(background));
    customizer.draw(target,font);
    target.display();
    assert(target.getTexture().copyToImage().saveToFile("build/tests/hamster-eyes-scarf.png"));
    customizer.handleEvent(next,inputWindow);
    customizer.handleEvent(next,inputWindow);
    assert(customizer.selectedScarf() == 0);

    sf::RenderTexture sheet;
    assert(sheet.create(960,720));
    sheet.clear(sf::Color(255,248,228));
    const char* labels[] = {"ROUND / RED SCARF", "SLEEPY / BLUE SCARF", "CLASSIC / NO SCARF"};
    for (int row=0;row<3;++row) {
        sf::Text label(labels[row],font,20);
        label.setFillColor(sf::Color(72,46,37));
        label.setPosition(20.f, row*240.f+8);
        sheet.draw(label);
        for (int direction=0;direction<4;++direction) {
            sf::Sprite sprite(customizer.texture(row == 1 ? 3 : 0, row == 2 ? 0 : row+1, row == 2 ? 0 : row+1), {direction*48,0,48,48});
            sprite.setScale(4,4);
            sprite.setPosition(direction*240.f+24,row*240.f+38);
            sheet.draw(sprite);
        }
    }
    sheet.display();
    assert(sheet.getTexture().copyToImage().saveToFile("build/tests/hamster-layer-contact-sheet.png"));
    std::cout << "Customizer previews and palette checks passed.\n";
}
