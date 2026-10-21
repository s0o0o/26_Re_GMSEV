#include "../TP_2023192003_CLIENT/client_sample/hamster_customizer.hpp"
#include "../TP_2023192003_CLIENT/client_sample/background_blur.hpp"
#include <cassert>
#include <iostream>

int main() {
    HamsterCustomizer customizer;
    assert(customizer.load("TP_2023192003_CLIENT/client_sample/assets/characters"));
    sf::Font font;
    assert(font.loadFromFile("TP_2023192003_CLIENT/client_sample/neodgm.ttf"));
    sf::RenderTexture target;
    assert(target.create(900, 800));
    sf::Texture background;
    assert(background.loadFromFile("TP_2023192003_CLIENT/client_sample/main.png"));
    assert(customizer.setBackdrop(background));
    sf::Image solid;
    solid.create(3,2,sf::Color(123,90,45,255));
    const auto uniform = blurBackground(solid);
    assert(uniform.getSize() == solid.getSize());
    assert(uniform.getPixel(0,0) == solid.getPixel(0,0));
    assert(uniform.getPixel(2,1) == solid.getPixel(2,1));
    sf::Image impulse;
    impulse.create(33,33,sf::Color::Black);
    impulse.setPixel(16,16,sf::Color::White);
    const auto blurred = blurBackground(impulse);
    assert(blurred.getPixel(16,16).r < 255);
    assert(blurred.getPixel(17,16).r > 0);
    assert(blurred.getPixel(16,17).r > 0);
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
    assert(source.loadFromFile("TP_2023192003_CLIENT/client_sample/assets/characters/hamtaro/character.png"));
    for (unsigned y=0;y<48;++y) for (unsigned x=0;x<192;++x)
        assert(original.getPixel(x,y) == source.getPixel(x,y));
    for(std::uint8_t kind=0;kind<HAMSTER_CHARACTER_KINDS;++kind) {
      assert(source.loadFromFile(std::string("TP_2023192003_CLIENT/client_sample/assets/characters/") + (kind == 0 ? "hamtaro" : "bijou") + "/character.png"));
      const auto natural = customizer.texture(0,0,0,kind).copyToImage();
      for(unsigned y=0;y<48;++y)for(unsigned x=0;x<192;++x)assert(natural.getPixel(x,y) == source.getPixel(x,y));
      for (std::uint8_t f=0;f<HAMSTER_FUR_COLORS;++f)
      for (std::uint8_t e=0;e<HAMSTER_EYE_STYLES;++e)
        for (std::uint8_t s=0;s<HAMSTER_SCARF_STYLES;++s) {
            const auto variant = customizer.texture(f,e,s,kind).copyToImage();
            assert(variant.getSize() == source.getSize());
            for (unsigned y=0;y<48;++y) for (unsigned x=0;x<192;++x)
                assert(variant.getPixel(x,y).a == source.getPixel(x,y).a);
        }
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
    next.key.code = sf::Keyboard::F5;
    customizer.handleEvent(next,inputWindow);
    assert(customizer.selectedCharacter() == 1);
    next.key.code = sf::Keyboard::F1;
    customizer.handleEvent(next,inputWindow);
    target.clear();
    customizer.draw(target,font);
    target.display();
    assert(target.getTexture().copyToImage().saveToFile("build/tests/bijou-customization.png"));
    sheet.clear(sf::Color(255,248,228));
    for(int row=0;row<3;++row) {
        sf::Text title(row == 0 ? "HAMTARO" : (row == 1 ? "BIJOU" : "BIJOU / ROUND / RED SCARF"),font,20);
        title.setPosition(20,row*240.f+8);
        title.setFillColor(sf::Color(72,46,37));
        sheet.draw(title);
        for(int d=0;d<4;++d) {
            sf::Sprite sprite(customizer.texture(0,row == 2 ? 1 : 0,row == 2 ? 1 : 0,row == 0 ? 0 : 1),{d*48,0,48,48});
            sprite.setScale(4,4);
            sprite.setPosition(d*240.f+24,row*240.f+38);
            sheet.draw(sprite);
        }
    }
    sheet.display();
    assert(sheet.getTexture().copyToImage().saveToFile("build/tests/two-character-contact-sheet.png"));
    next.key.code = sf::Keyboard::F5;
    customizer.handleEvent(next,inputWindow);
    assert(customizer.selectedCharacter() == 0);
    assert(&customizer.texture(0,0,0,255) == &customizer.texture(0));
    std::cout << "Customizer previews and palette checks passed.\n";
}
