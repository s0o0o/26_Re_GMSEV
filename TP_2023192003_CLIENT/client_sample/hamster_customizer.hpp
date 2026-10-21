#pragma once
#include <SFML/Graphics.hpp>
#include <array>
#include <string>
#include "../../shared/appearance.hpp"

// Owns textures for the entire session so sprites never point to temporaries.
class HamsterCustomizer {
public:
    bool load(const std::string& directory);
    bool setBackdrop(const sf::Texture& texture);
    void handleEvent(const sf::Event& event, const sf::RenderWindow& window);
    void draw(sf::RenderTarget& target, const sf::Font& font) const;
    const sf::Texture& texture(std::uint8_t color, std::uint8_t eyes = 0, std::uint8_t scarf = 0, std::uint8_t kind = 0) const;
    std::uint8_t selected() const { return selected_; }
    std::uint8_t selectedEyes() const { return eyes_; }
    std::uint8_t selectedScarf() const { return scarf_; }
    std::uint8_t selectedCharacter() const { return character_; }
private:
    std::array<sf::Texture, HAMSTER_CHARACTER_APPEARANCES> textures_;
    std::uint8_t selected_ = 0;
    std::uint8_t eyes_ = 0, scarf_ = 0;
    std::uint8_t character_ = 0;
    int direction_ = 1;
    bool loaded_ = false;
    sf::Texture blurredBackdrop_;
    bool hasBackdrop_ = false;
};
