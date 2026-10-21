#pragma once
#include "ui_alignment.hpp"
#include <cmath>

enum class LoadingPhase { Connecting, Sending, Waiting, Receiving };
inline const wchar_t* loadingMessage(LoadingPhase phase) {
    switch (phase) {
    case LoadingPhase::Connecting: return L"\uC11C\uBC84\uC5D0 \uC5F0\uACB0\uD558\uB294 \uC911";
    case LoadingPhase::Sending: return L"\uB85C\uADF8\uC778 \uC815\uBCF4\uB97C \uBCF4\uB0B4\uB294 \uC911";
    case LoadingPhase::Waiting: return L"\uD584\uC2A4\uD130 \uB9C8\uC744\uC758 \uBB38\uC744 \uC5EC\uB294 \uC911";
    default: return L"\uC11C\uBC84 \uC751\uB2F5\uC744 \uD655\uC778\uD558\uB294 \uC911";
    }
}

// Uses the game's own atlas; no generated image or extra asset is needed.
inline void drawLoadingScreen(sf::RenderTarget& target, const sf::Font& font,
    const sf::Texture& hamster, const sf::String& nickname, LoadingPhase phase, float seconds) {
    const sf::Color brown(130,86,54), ink(72,46,37);
    const auto box = [&](sf::FloatRect rect, sf::Color fill, float outline) {
        sf::RectangleShape shape({rect.width,rect.height});
        shape.setPosition(rect.left,rect.top);
        shape.setFillColor(fill); shape.setOutlineColor(brown); shape.setOutlineThickness(outline);
        target.draw(shape);
    };
    const auto text = [&](const sf::String& value, sf::FloatRect area, unsigned size, sf::Color color) {
        sf::Text label(value,font,size); label.setFillColor(color);
        centerText(label,area); target.draw(label);
    };
    box({206,196,500,406},{0,0,0,65},0);
    box({200,190,500,406},{255,248,228},6);
    box({212,202,476,382},{255,248,228},1);
    text(L"\uD584\uC2A4\uD130 \uB9C8\uC744\uB85C \uCD9C\uBC1C!",{225,222,450,42},28,ink);
    const float bounce = std::abs(std::sin(seconds*4.f))*7.f;
    sf::RectangleShape shadow({66,6});
    shadow.setPosition(417,400); shadow.setFillColor({222,202,170}); target.draw(shadow);
    sf::Sprite sprite(hamster,{48,0,48,48});
    sprite.setOrigin(24,48); sprite.setScale(3,3); sprite.setPosition(450,404-bounce);
    target.draw(sprite);
    text(nickname,{225,414,450,32},23,ink);
    text(loadingMessage(phase),{225,454,450,32},22,ink);
    // Indeterminate dots: deliberately no invented server percentage.
    const int active = static_cast<int>(seconds*4.f)%5;
    for (int i=0;i<5;++i) box({394.f+i*24.f,505,16,10},i == active ? sf::Color(153,101,64) : sf::Color(235,212,160),1);
    text(L"\uC7A0\uC2DC\uB9CC \uAE30\uB2E4\uB824 \uC8FC\uC138\uC694",{225,540,450,26},18,brown);
}
