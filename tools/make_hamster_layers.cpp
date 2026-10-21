// Deterministic pixel source for the game's tiny, grid-aligned production layers.
// The AI concept sheet is a design reference, not sampled as a sprite atlas.
#include <SFML/Graphics/Image.hpp>
#include <array>
#include <iostream>

namespace {
struct Rect { unsigned x, y, w, h; };
const std::array<Rect, 4> eyes = {{{56,17,10,9}, {75,17,11,9}, {110,17,12,9}, {169,17,13,9}}};
const sf::Color ink(67, 20, 17), cream(255, 248, 232);
void rect(sf::Image& image, unsigned x, unsigned y, unsigned w, unsigned h, sf::Color color) {
    for (unsigned dy = 0; dy < h; ++dy) for (unsigned dx = 0; dx < w; ++dx)
        image.setPixel(x + dx, y + dy, color);
}
bool save(const sf::Image& image, const char* name) {
    return image.saveToFile(std::string("TP_2023192003_CLIENT/client_sample/assets/appearance/") + name + ".png");
}
}
int main() {
    sf::Image original;
    if (!original.loadFromFile("TP_2023192003_CLIENT/client_sample/character.png") ||
        original.getSize() != sf::Vector2u(192, 48)) return 1;
    sf::Image body = original, classic, round, sleepy, empty;
    classic.create(192,48,sf::Color::Transparent);
    round = sleepy = empty = classic;
    for (const auto& r : eyes) {
        for (unsigned y = r.y; y < r.y+r.h; ++y) for (unsigned x = r.x; x < r.x+r.w; ++x) {
            classic.setPixel(x,y,original.getPixel(x,y));
            auto skin = cream;
            skin.a = original.getPixel(x,y).a;
            body.setPixel(x,y,skin);
        }
        const unsigned cx = r.x + r.w/2, cy = r.y + 4;
        rect(round,cx-2,cy-2,4,5,ink);
        rect(round,cx-1,cy-3,2,7,ink);
        round.setPixel(cx-1,cy-2,sf::Color::White);
        rect(sleepy,cx-3,cy,7,2,ink);
        sleepy.setPixel(cx-3,cy-1,ink);
        sleepy.setPixel(cx+3,cy-1,ink);
    }
    for (unsigned y=0;y<48;++y) for (unsigned x=0;x<192;++x) {
        if (original.getPixel(x,y).a == 0) {
            round.setPixel(x,y,sf::Color::Transparent);
            sleepy.setPixel(x,y,sf::Color::Transparent);
        }
    }
    if (!save(body,"body-eyeless") || !save(classic,"eyes-classic") ||
        !save(round,"eyes-round") || !save(sleepy,"eyes-sleepy") || !save(empty,"scarf-none")) return 1;
    for (int style=0; style<2; ++style) {
        sf::Image scarf = empty;
        const sf::Color dark = style == 0 ? sf::Color(123,30,40) : sf::Color(30,61,118);
        const sf::Color fill = style == 0 ? sf::Color(219,64,74) : sf::Color(70,127,215);
        const sf::Color light = style == 0 ? sf::Color(255,127,126) : sf::Color(130,188,255);
        // Back, front, left, right. A narrow band below the mouth and a short tail.
        const std::array<Rect,4> bands = {{{7,33,34,5},{54,34,35,5},{103,34,29,5},{156,34,29,5}}};
        for (int f=0;f<4;++f) {
            const auto& b = bands[f];
            rect(scarf,b.x,b.y,b.w,b.h,dark);
            rect(scarf,b.x+1,b.y+1,b.w-2,3,fill);
            rect(scarf,b.x+2,b.y+1,b.w-4,1,light);
            const unsigned tailX = f == 3 ? b.x+2 : b.x+b.w-7;
            rect(scarf,tailX,b.y+3,5,8,dark);
            rect(scarf,tailX+1,b.y+3,3,7,fill);
            rect(scarf,tailX+1,b.y+8,3,1,light);
        }
        // Stay inside the original body silhouette and never paint transparent air.
        for (unsigned y=0;y<48;++y) for (unsigned x=0;x<192;++x)
            if (original.getPixel(x,y).a == 0) scarf.setPixel(x,y,sf::Color::Transparent);
        if (!save(scarf,style == 0 ? "scarf-red" : "scarf-blue")) return 1;
    }
    std::cout << "Created 7 aligned 192x48 appearance layers.\n";
}
