#include "hamster_customizer.hpp"
#include "background_blur.hpp"
#include "ui_alignment.hpp"

namespace {
const std::array<sf::Color, HAMSTER_FUR_COLORS> palette = {{
    {255, 184, 0}, {235, 212, 160}, {153, 101, 64}, {160, 170, 185}, {231, 155, 169}
}};
const char* names[] = {"Golden", "Cream", "Cocoa", "Silver", "Rose"};
sf::FloatRect swatch(int i) { return {215.f + i * 100.f, 335.f, 70.f, 32.f}; }
sf::FloatRect option(int i, float y) { return {217.5f + i * 160.f, y, 145.f, 40.f}; }
void label(sf::RenderTarget& target, const sf::Font& font, const char* text,
    sf::FloatRect area, unsigned size, sf::Color color = sf::Color(72, 46, 37)) {
    sf::Text t(text, font, size);
    centerText(t, area);
    t.setFillColor(color);
    target.draw(t);
}
}

bool HamsterCustomizer::load(const std::string& root) {
    loaded_ = false;
    const char* characterNames[] = {"hamtaro","bijou"};
    for(std::uint8_t kind=0;kind<HAMSTER_CHARACTER_KINDS;++kind) {
    const std::string directory = root + "/" + characterNames[kind] + "/";
    sf::Image original;
    if (!original.loadFromFile(directory+"character.png") || original.getSize() != sf::Vector2u(192, 48)) return false;
    sf::Image body;
    std::array<sf::Image, 3> eyes, scarves;
    const auto loadLayer = [&](sf::Image& image, const char* name) {
        return image.loadFromFile(directory + name + ".png") && image.getSize() == original.getSize();
    };
    if (!loadLayer(body, "body-eyeless")) return false;
    const char* eyeNames[] = {"eyes-classic", "eyes-round", "eyes-sleepy"};
    const char* scarfNames[] = {"scarf-none", "scarf-red", "scarf-blue"};
    for (int i=0;i<3;++i) if (!loadLayer(eyes[i], eyeNames[i]) || !loadLayer(scarves[i], scarfNames[i])) return false;
    for (int i=0;i<3;++i) {
        if (!scarfIcons_[kind][i].loadFromImage(scarves[i])) return false;
        scarfIcons_[kind][i].setSmooth(false);
    }
    for (std::uint8_t i = 0; i < HAMSTER_FUR_COLORS; ++i) {
      for (std::uint8_t e=0;e<HAMSTER_EYE_STYLES;++e) {
        sf::Image variant = body;
        variant.copy(eyes[e], 0, 0, sf::IntRect(), true);
        if (i != 0) {
            for (unsigned y = 0; y < 48; ++y) for (unsigned x = 0; x < 192; ++x) {
                const auto p = variant.getPixel(x, y);
                // Warm yellow fur only: preserve white belly, eyes, pink ears and outline.
                const bool golden = kind == 0 && p.r > 160 && p.g > 100 && p.b < 100 && p.r > p.g;
                const bool whiteFur = kind == 1 && (x/48 == 0 || y < 20) && p.r >= 220 && p.g >= 220 && p.b >= 200;
                if (p.a && (golden || whiteFur)) {
                    const float shade = p.r / 255.f;
                    variant.setPixel(x, y, sf::Color(
                        static_cast<sf::Uint8>((whiteFur ? 153.f+palette[i].r*.4f : palette[i].r) * shade),
                        static_cast<sf::Uint8>((whiteFur ? 153.f+palette[i].g*.4f : palette[i].g) * shade),
                        static_cast<sf::Uint8>((whiteFur ? 153.f+palette[i].b*.4f : palette[i].b) * shade), p.a));
                }
            }
        }
        for (std::uint8_t s=0;s<HAMSTER_SCARF_STYLES;++s) {
            sf::Image dressed = variant;
            dressed.copy(scarves[s], 0, 0, sf::IntRect(), true);
            // Keep the source silhouette, including any partially transparent edge pixels.
            for (unsigned y=0;y<48;++y) for (unsigned x=0;x<192;++x) {
                auto p = dressed.getPixel(x,y);
                p.a = original.getPixel(x,y).a;
                dressed.setPixel(x,y,p);
            }
            auto& texture = textures_[characterAppearanceIndex(kind,i,e,s)];
            if (!texture.loadFromImage(dressed)) return false;
            texture.setSmooth(false);
        }
      }
    }
    }
    loaded_ = true;
    return true;
}

const sf::Texture& HamsterCustomizer::texture(std::uint8_t color, std::uint8_t eyes, std::uint8_t scarf, std::uint8_t kind) const {
    return textures_[characterAppearanceIndex(validCharacterKind(kind) ? kind : 0,validFurColor(color) ? color : 0,
        validEyeStyle(eyes) ? eyes : 0, validScarfStyle(scarf) ? scarf : 0)];
}

bool HamsterCustomizer::setBackdrop(const sf::Texture& texture) {
    hasBackdrop_ = blurredBackdrop_.loadFromImage(blurBackground(texture.copyToImage()));
    return hasBackdrop_;
}

void HamsterCustomizer::handleEvent(const sf::Event& event, const sf::RenderWindow& window) {
    if (event.type == sf::Event::KeyPressed) {
        if (event.key.code == sf::Keyboard::Right) selected_ = (selected_ + 1) % HAMSTER_FUR_COLORS;
        if (event.key.code == sf::Keyboard::Left) selected_ = (selected_ + HAMSTER_FUR_COLORS - 1) % HAMSTER_FUR_COLORS;
        if (event.key.code == sf::Keyboard::Down) direction_ = (direction_ + 1) % 4;
        if (event.key.code == sf::Keyboard::Up) direction_ = (direction_ + 3) % 4;
        if (event.key.code == sf::Keyboard::F1) eyes_ = 0;
        if (event.key.code == sf::Keyboard::F2) eyes_ = 1;
        if (event.key.code == sf::Keyboard::F3) eyes_ = 2;
        if (event.key.code == sf::Keyboard::F4) scarf_ = (scarf_ + 1) % HAMSTER_SCARF_STYLES;
        if (event.key.code == sf::Keyboard::F5) character_ = (character_ + 1) % HAMSTER_CHARACTER_KINDS;
    }
    if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
        const auto point = window.mapPixelToCoords({event.mouseButton.x, event.mouseButton.y});
        if(sf::FloatRect(285,120,150,30).contains(point)) character_=0;
        if(sf::FloatRect(465,120,150,30).contains(point)) character_=1;
        for (std::uint8_t i = 0; i < HAMSTER_FUR_COLORS; ++i) if (swatch(i).contains(point)) selected_ = i;
        for (std::uint8_t i=0;i<3;++i) {
            if (option(i,390).contains(point)) eyes_ = i;
            if (option(i,440).contains(point)) scarf_ = i;
        }
    }
}

void HamsterCustomizer::draw(sf::RenderTarget& target, const sf::Font& font) const {
    if (hasBackdrop_) target.draw(sf::Sprite(blurredBackdrop_));
    sf::RectangleShape dim(target.getView().getSize());
    dim.setFillColor({0,0,0,100});
    target.draw(dim);
    sf::RectangleShape panel({580.f, 480.f});
    panel.setPosition(160.f, 70.f);
    panel.setFillColor({255, 248, 228});
    panel.setOutlineColor({130, 86, 54});
    panel.setOutlineThickness(6.f);
    target.draw(panel);
    label(target, font, "MAKE YOUR HAMSTER", {160,80,580,32}, 28);
    if (!loaded_) { label(target, font, "Character preview unavailable", {160,220,580,40}, 24); return; }
    const char* characterLabels[] = {"Hamtaro", "Bijou"};
    for(int i=0;i<2;++i) {
        sf::RectangleShape button({150,30});
        button.setPosition(i == 0 ? 285.f : 465.f,120);
        button.setFillColor(i == character_ ? sf::Color(242,212,155) : sf::Color(255,251,239));
        button.setOutlineColor({130,86,54});
        button.setOutlineThickness(i == character_ ? 3.f : 1.f);
        target.draw(button);
        label(target,font,characterLabels[i],button.getGlobalBounds(),19);
    }
    sf::Sprite preview(texture(selected_, eyes_, scarf_, character_), {direction_ * 48, 0, 48, 48});
    preview.setScale(3.f, 3.f);
    preview.setPosition(378.f, 146.f);
    target.draw(preview);
    label(target, font, character_ == 1 && selected_ == 0 ? "Snow" : names[selected_], {160,298,580,30}, 22);
    for (int i = 0; i < HAMSTER_FUR_COLORS; ++i) {
        const auto bounds = swatch(i);
        sf::RectangleShape button({bounds.width, bounds.height});
        button.setPosition(bounds.left, bounds.top);
        button.setFillColor(character_ == 1 && i == 0 ? sf::Color(255,248,232) : palette[i]);
        button.setOutlineColor(i == selected_ ? sf::Color(72, 46, 37) : sf::Color(200, 182, 156));
        button.setOutlineThickness(i == selected_ ? 5.f : 2.f);
        target.draw(button);
    }
    for (int row=0;row<2;++row) for (int i=0;i<3;++i) {
        const auto bounds = option(i, row == 0 ? 390.f : 440.f);
        const bool selected = i == (row == 0 ? eyes_ : scarf_);
        sf::RectangleShape button({bounds.width, bounds.height});
        button.setPosition(bounds.left, bounds.top);
        button.setFillColor(selected ? sf::Color(242,212,155) : sf::Color(255,251,239));
        button.setOutlineColor({130,86,54});
        button.setOutlineThickness(selected ? 3.f : 1.f);
        target.draw(button);
        if (row == 1 && i == 0) {
            // Empty equipment slot: circle and slash, independent of language.
            sf::CircleShape empty(12.f);
            empty.setOrigin(12,12);
            empty.setPosition(bounds.left+bounds.width/2, bounds.top+bounds.height/2);
            empty.setFillColor(sf::Color::Transparent);
            empty.setOutlineColor({130,86,54});
            empty.setOutlineThickness(2);
            target.draw(empty);
            sf::RectangleShape slash({28,2});
            slash.setOrigin(14,1);
            slash.setPosition(empty.getPosition());
            slash.setRotation(-45);
            slash.setFillColor({130,86,54});
            target.draw(slash);
        } else {
            const sf::IntRect crop = row == 0 ? sf::IntRect(56,18,32,14) : sf::IntRect(56,31,32,17);
            sf::Sprite icon(row == 0 ? texture(selected_,i,0,character_) : scarfIcons_[character_][i],crop);
            icon.setOrigin(crop.width/2.f,crop.height/2.f);
            icon.setScale(1.75f,1.75f);
            icon.setPosition(bounds.left+bounds.width/2,bounds.top+bounds.height/2);
            target.draw(icon);
        }
    }
    label(target, font, "Left-Right: fur / Up-Down: turn", {160,490,580,25}, 17);
    label(target, font, "F1-F3: eyes / F4: scarf / F5: character", {160,518,580,25}, 16);
}
