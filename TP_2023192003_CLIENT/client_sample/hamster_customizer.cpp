#include "hamster_customizer.hpp"

namespace {
const std::array<sf::Color, HAMSTER_FUR_COLORS> palette = {{
    {255, 184, 0}, {235, 212, 160}, {153, 101, 64}, {160, 170, 185}, {231, 155, 169}
}};
const char* names[] = {"Golden", "Cream", "Cocoa", "Silver", "Rose"};
sf::FloatRect swatch(int i) { return {215.f + i * 100.f, 413.f, 70.f, 42.f}; }
void label(sf::RenderTarget& target, const sf::Font& font, const char* text,
    float x, float y, unsigned size, sf::Color color = sf::Color(72, 46, 37)) {
    sf::Text t(text, font, size);
    t.setPosition(x, y);
    t.setFillColor(color);
    target.draw(t);
}
}

bool HamsterCustomizer::load(const std::string& path) {
    loaded_ = false;
    sf::Image original;
    if (!original.loadFromFile(path) || original.getSize() != sf::Vector2u(192, 48)) return false;
    for (std::uint8_t i = 0; i < HAMSTER_FUR_COLORS; ++i) {
        sf::Image variant = original;
        if (i != 0) {
            for (unsigned y = 0; y < 48; ++y) for (unsigned x = 0; x < 192; ++x) {
                const auto p = original.getPixel(x, y);
                // Warm yellow fur only: preserve white belly, eyes, pink ears and outline.
                if (p.a && p.r > 160 && p.g > 100 && p.b < 100 && p.r > p.g) {
                    const float shade = p.r / 255.f;
                    variant.setPixel(x, y, sf::Color(
                        static_cast<sf::Uint8>(palette[i].r * shade),
                        static_cast<sf::Uint8>(palette[i].g * shade),
                        static_cast<sf::Uint8>(palette[i].b * shade), p.a));
                }
            }
        }
        if (!textures_[i].loadFromImage(variant)) return false;
        textures_[i].setSmooth(false);
    }
    loaded_ = true;
    return true;
}

const sf::Texture& HamsterCustomizer::texture(std::uint8_t color) const {
    return textures_[validFurColor(color) ? color : 0];
}

void HamsterCustomizer::handleEvent(const sf::Event& event, const sf::RenderWindow& window) {
    if (event.type == sf::Event::KeyPressed) {
        if (event.key.code == sf::Keyboard::Right) selected_ = (selected_ + 1) % HAMSTER_FUR_COLORS;
        if (event.key.code == sf::Keyboard::Left) selected_ = (selected_ + HAMSTER_FUR_COLORS - 1) % HAMSTER_FUR_COLORS;
        if (event.key.code == sf::Keyboard::Down) direction_ = (direction_ + 1) % 4;
        if (event.key.code == sf::Keyboard::Up) direction_ = (direction_ + 3) % 4;
    }
    if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
        const auto point = window.mapPixelToCoords({event.mouseButton.x, event.mouseButton.y});
        for (std::uint8_t i = 0; i < HAMSTER_FUR_COLORS; ++i) if (swatch(i).contains(point)) selected_ = i;
    }
}

void HamsterCustomizer::draw(sf::RenderTarget& target, const sf::Font& font) const {
    sf::RectangleShape panel({580.f, 440.f});
    panel.setPosition(160.f, 90.f);
    panel.setFillColor({255, 248, 228});
    panel.setOutlineColor({130, 86, 54});
    panel.setOutlineThickness(6.f);
    target.draw(panel);
    label(target, font, "MAKE YOUR HAMSTER", 260, 108, 28);
    if (!loaded_) { label(target, font, "Character preview unavailable", 235, 230, 24); return; }
    sf::Sprite preview(texture(selected_), {direction_ * 48, 0, 48, 48});
    preview.setScale(4.f, 4.f);
    preview.setPosition(354.f, 160.f);
    target.draw(preview);
    label(target, font, names[selected_], 400, 365, 26);
    for (int i = 0; i < HAMSTER_FUR_COLORS; ++i) {
        const auto bounds = swatch(i);
        sf::RectangleShape button({bounds.width, bounds.height});
        button.setPosition(bounds.left, bounds.top);
        button.setFillColor(palette[i]);
        button.setOutlineColor(i == selected_ ? sf::Color(72, 46, 37) : sf::Color(200, 182, 156));
        button.setOutlineThickness(i == selected_ ? 5.f : 2.f);
        target.draw(button);
    }
    label(target, font, "Click color / Left-Right: fur / Up-Down: turn", 195, 485, 19);
}
