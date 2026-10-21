#pragma once
#include <SFML/Graphics.hpp>
#include <algorithm>
#include <cmath>

// SFML glyph bounds have nonzero left/top offsets; position their visible center.
inline void centerText(sf::Text& text, const sf::FloatRect& area) {
    auto bounds = text.getLocalBounds();
    const float scale = std::min(1.f, std::min(
        (area.width - 12.f) / std::max(1.f, bounds.width),
        (area.height - 4.f) / std::max(1.f, bounds.height)));
    text.setScale(scale, scale);
    text.setOrigin(bounds.left + bounds.width / 2.f, bounds.top + bounds.height / 2.f);
    text.setPosition(std::round(area.left + area.width / 2.f),
        std::round(area.top + area.height / 2.f));
}
