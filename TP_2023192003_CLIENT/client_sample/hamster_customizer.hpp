#pragma once
#include <SFML/Graphics.hpp>
#include <array>
#include <string>
#include "../../shared/appearance.hpp"

// Owns textures for the entire session so sprites never point to temporaries.
class HamsterCustomizer {
public:
    bool load(const std::string& path);
    void handleEvent(const sf::Event& event, const sf::RenderWindow& window);
    void draw(sf::RenderTarget& target, const sf::Font& font) const;
    const sf::Texture& texture(std::uint8_t color) const;
    std::uint8_t selected() const { return selected_; }
private:
    std::array<sf::Texture, HAMSTER_FUR_COLORS> textures_;
    std::uint8_t selected_ = 0;
    int direction_ = 1;
    bool loaded_ = false;
};
