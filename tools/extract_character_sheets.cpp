#include <SFML/Graphics/Image.hpp>
#include <array>
#include <string>
#include <iostream>

struct Slot { unsigned x, width; };
int main() {
    const std::string root = "TP_2023192003_CLIENT/client_sample/assets/characters/";
    const std::array<const char*,2> names = {{"hamtaro","bijou"}};
    // First-row standing frames: rear, front, left, right. Bounds retain bows/ears.
    const std::array<std::array<Slot,4>,2> slots = {{{{{123,31},{9,31},{86,31},{86,31}}},
        {{{122,32},{5,37},{85,32},{85,32}}}}};
    for (int character=0;character<2;++character) {
        sf::Image sheet, atlas;
        if (!sheet.loadFromFile(root+"source/"+names[character]+"-sheet.png")) return 1;
        sf::Image overview;
        overview.create(1920,126,sf::Color(255,248,228));
        for(unsigned y=0;y<42;++y)for(unsigned x=0;x<640;++x) {
            auto p=sheet.getPixel(x,y);
            if(!p.a)p=sf::Color(255,248,228);
            for(unsigned dy=0;dy<3;++dy)for(unsigned dx=0;dx<3;++dx)overview.setPixel(x*3+dx,y*3+dy,p);
        }
        if(!overview.saveToFile("build/tests/"+std::string(names[character])+"-crop-source.png"))return 1;
        atlas.create(192,48,sf::Color::Transparent);
        for (unsigned frame=0;frame<4;++frame) {
            const auto slot=slots[character][frame];
            unsigned top=42,bottom=0;
            for (unsigned y=0;y<42;++y) for(unsigned x=slot.x;x<slot.x+slot.width;++x)
                if(sheet.getPixel(x,y).a){ if(y<top)top=y; if(y>bottom)bottom=y; }
            if(top>bottom) return 1;
            const unsigned width=slot.width*5/4, height=(bottom-top+1)*5/4;
            if(width>48 || height>46) return 1;
            const unsigned left=(48-width)/2, targetTop=46-height;
            for(unsigned y=0;y<height;++y) for(unsigned x=0;x<width;++x)
                atlas.setPixel(frame*48+left+x,targetTop+y,
                    sheet.getPixel(slot.x+(frame == 3 ? width-1-x : x)*slot.width/width,top+y*(bottom-top+1)/height));
        }
        if(!atlas.saveToFile(root+names[character]+"/character.png"))return 1;
        std::cout << "Cropped " << names[character] << " 4-direction atlas\n";
    }
}
