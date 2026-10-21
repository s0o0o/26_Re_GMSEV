#pragma once
#include <SFML/Graphics/Image.hpp>

// Applied once to a static backdrop, never per render frame.
sf::Image blurBackground(const sf::Image& source);
