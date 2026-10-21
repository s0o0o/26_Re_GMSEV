#include "background_blur.hpp"
#include <array>
#include <algorithm>
#include <cmath>

sf::Image blurBackground(const sf::Image& source) {
    const auto size = source.getSize();
    if (!size.x || !size.y) return source;
    constexpr int radius = 10;
    constexpr float sigma = 5.f;
    std::array<float, radius * 2 + 1> weights{};
    float total = 0;
    for (int i=-radius;i<=radius;++i) {
        weights[i+radius] = std::exp(-float(i*i)/(2*sigma*sigma));
        total += weights[i+radius];
    }
    for (auto& weight : weights) weight /= total;
    sf::Image horizontal, result;
    horizontal.create(size.x,size.y);
    result.create(size.x,size.y);
    const auto bound = [](int value, int high) { return value < 0 ? 0 : (value > high ? high : value); };
    for (int pass=0;pass<2;++pass) {
        const auto& input = pass == 0 ? source : horizontal;
        auto& output = pass == 0 ? horizontal : result;
        for (unsigned y=0;y<size.y;++y) for (unsigned x=0;x<size.x;++x) {
            float r=0,g=0,b=0,a=0;
            for (int i=-radius;i<=radius;++i) {
                const unsigned sx = pass == 0 ? bound(int(x)+i,int(size.x)-1) : x;
                const unsigned sy = pass == 1 ? bound(int(y)+i,int(size.y)-1) : y;
                const auto pixel = input.getPixel(sx,sy);
                const float w = weights[i+radius];
                r+=pixel.r*w; g+=pixel.g*w; b+=pixel.b*w; a+=pixel.a*w;
            }
            output.setPixel(x,y,{static_cast<sf::Uint8>(std::lround(r)),
                static_cast<sf::Uint8>(std::lround(g)), static_cast<sf::Uint8>(std::lround(b)),
                static_cast<sf::Uint8>(std::lround(a))});
        }
    }
    return result;
}
