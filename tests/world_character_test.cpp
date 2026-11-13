#define main production_client_main
#include "../TP_2023192003_CLIENT/client_sample/client.cpp"
#undef main
#include <cassert>
#include <filesystem>
int main() {
    const auto root=std::filesystem::current_path();
    std::filesystem::current_path(root/"TP_2023192003_CLIENT/client_sample");
    assert(hamsterCustomizer().load("assets/characters"));
    const char* names[]={"hamtaro","bijou"};
    const unsigned slots[2][4]={{123,9,86,86},{122,5,85,85}};
    const unsigned widths[2][4]={{31,31,31,31},{32,37,32,32}};
    for(unsigned kind=0;kind<2;++kind) {
        sf::Image source; assert(source.loadFromFile(std::string("assets/characters/source/")+names[kind]+"-sheet.png"));
        const auto world=hamsterCustomizer().worldTexture(0,0,0,kind).copyToImage();
        for(unsigned f=0;f<4;++f) {
            unsigned top=42,bottom=0;
            for(unsigned y=0;y<42;++y) for(unsigned x=slots[kind][f];x<slots[kind][f]+widths[kind][f];++x)
                if(source.getPixel(x,y).a){top=(std::min)(top,y);bottom=(std::max)(bottom,y);}
            const unsigned height=bottom-top+1,width=widths[kind][f],left=(48-width)/2;
            for(unsigned y=0;y<height;++y) for(unsigned x=0;x<width;++x) {
                const unsigned sx=slots[kind][f]+(f==3 ? width-1-x : x);
                const auto actual=world.getPixel(f*48+left+x,46-height+y),expected=source.getPixel(sx,top+y);
                if(actual.a!=expected.a || (expected.a && actual!=expected)) std::cerr<<"Mismatch kind="<<kind<<" frame="<<f<<" x="<<x<<" y="<<y<<std::endl;
                assert(actual.a==expected.a && (!expected.a || actual==expected));
            }
        }
        for(unsigned fur=0;fur<HAMSTER_FUR_COLORS;++fur) for(unsigned eyes=0;eyes<HAMSTER_EYE_STYLES;++eyes) for(unsigned scarf=0;scarf<HAMSTER_SCARF_STYLES;++scarf) {
            const auto& tex=hamsterCustomizer().worldTexture(fur,eyes,scarf,kind);
            assert(tex.getSize()==sf::Vector2u(192,48) && !tex.isSmooth());
            const auto variant=tex.copyToImage();
            for(unsigned y=0;y<48;++y)for(unsigned x=0;x<192;++x) assert(variant.getPixel(x,y).a==world.getPixel(x,y).a);
        }
    }
    sf::Sprite sprite; world_character::place(sprite,100,200);
    assert(sprite.getScale()==sf::Vector2f(2,2) && sprite.getOrigin()==sf::Vector2f(24,46));
    assert(sprite.getTransform().transformPoint(24,46)==sf::Vector2f(125,246));
    bool rejected=false; sf::Image empty;
    try { world_character::nativeAtlas(empty,empty,0); } catch(const std::invalid_argument&) { rejected=true; }
    assert(rejected);
    // Both self and ENTER peers use the same OBJECT draw path in production client_main.
    std::cout<<"World character: 8 source frames pixel-exact, 90 appearances alpha/nearest, shared integer scale/feet passed.\n";
}
