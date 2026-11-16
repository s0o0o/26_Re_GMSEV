#include "../TP_2023192003_CLIENT/client_sample/town_npc_renderer.hpp"
#include "../TP_2023192003_CLIENT/client_sample/profile_layout.hpp"
#include "../TP_2023192003_CLIENT/client_sample/display_layout.hpp"
#include <iostream>
#include <cassert>
#include <filesystem>
#include <fstream>
#include <vector>
int main() {
    // Decode the legacy Korean filename through a native filesystem path.
    for(const auto& filename : {std::filesystem::path(L"monHam.png"),std::filesystem::path(L"\uD6C8\uC2A4\uD130\uC791\uC740\uBC84\uC804.png")}) {
        std::ifstream input(std::filesystem::path("TP_2023192003_CLIENT/client_sample")/filename,std::ios::binary);
        const std::vector<char> bytes((std::istreambuf_iterator<char>(input)),{});
        sf::Image source;assert(!bytes.empty() && source.loadFromMemory(bytes.data(),bytes.size()));
        const auto size=source.getSize();
        std::cout<<"hostile source "<<size.x<<"x"<<size.y<<"; atlas cells52, single-frame crop uses source size\n";
        assert(size.y<=52 && (size.x<=52 || size.x>=104));
    }
    TownNpcRenderer renderer;
    assert(renderer.load("TP_2023192003_CLIENT/client_sample/assets/npcs/source"));
    for(int type=100;type<=102;++type) {
        const auto image=renderer.texture(type).copyToImage();
        for(int direction=0;direction<4;++direction) for(int phase=0;phase<2;++phase) {
            const auto r=TownNpcRenderer::frame(type,phase*.4f,static_cast<town_motion::Facing>(direction));
            int outside=0,l=999,t=999,right=0,bottom=0;
            sf::IntRect cell=r;
            if(type==100 && direction==0) cell={phase*50,79,50,50};
            if(type==101 && direction==0) cell=phase ? sf::IntRect(154,0,38,40) : sf::IntRect(0,0,41,40);
            if(type==101 && direction==1) cell={77,0,40,40};
            if(type==100 && direction==1) cell={150,179,50,50};
            if(type==100 && direction>=2) cell={200,129,50,50};
            if(type==101 && direction>=2) cell={40,0,37,40};
            if(type==102) cell=phase ? sf::IntRect(100,53,47,42) : sf::IntRect(55,53,45,42);
            for(int y=cell.top;y<cell.top+cell.height;++y)
                for(int x=cell.left;x<cell.left+cell.width;++x)
                    if(image.getPixel(x,y).a) {
                        l=(std::min)(l,x);t=(std::min)(t,y);right=(std::max)(right,x);bottom=(std::max)(bottom,y);
                        if(!r.contains(x,y)) ++outside;
                    }
            std::cout<<type<<" dir="<<direction<<" phase="<<phase<<" rect="<<r.left<<","<<r.top<<","<<r.width<<","<<r.height
                <<" nearby_bbox="<<l<<","<<t<<".."<<right<<","<<bottom<<" outside="<<outside<<"\n";
            assert(outside==0 && l<=right && t<=bottom);
            int captured=0,expected=0;
            for(int y=cell.top;y<cell.top+cell.height;++y) for(int x=cell.left;x<cell.left+cell.width;++x)
                if(image.getPixel(x,y).a) ++expected;
            for(int y=r.top;y<r.top+r.height;++y) for(int x=r.left;x<r.left+r.width;++x)
                if(image.getPixel(x,y).a) ++captured;
            assert(captured==expected); // No missing pixels AND no adjacent-frame contamination.
        }
    }
    sf::Font font; assert(font.loadFromFile("TP_2023192003_CLIENT/client_sample/neodgm.ttf"));
    sf::RenderTexture output; assert(output.create(1000,1100)); output.clear({255,248,228});
    auto label=[&](const char* value,float x,float y) { sf::Text t(value,font,22);t.setFillColor({72,46,37});t.setPosition(x,y);output.draw(t); };
    label("Original crop",200,10);label("Fixed crop",620,10);
    const int types[]={100,100,101,101,102};
    const int dirs[]={0,0,0,1,0},phases[]={0,1,1,0,1};
    const sf::IntRect old[]={{3,79,30,39},{53,79,30,39},{157,5,35,33},{80,5,35,33},{101,56,43,37}};
    for(int i=0;i<5;++i) {
        label(i<2?"Boss":(i<4?"Pashmina":"Snoozer"),20,70+i*190.f);
        const auto fixed=TownNpcRenderer::frame(types[i],phases[i]*.4f,static_cast<town_motion::Facing>(dirs[i]));
        for(int side=0;side<2;++side) {
            const auto rect=side ? fixed : old[i];
            sf::Sprite s(renderer.texture(types[i]),rect);s.setScale(4,4);s.setPosition(220+side*420.f,55+i*190.f);output.draw(s);
        }
    }
    output.display();assert(output.getTexture().copyToImage().saveToFile("build/tests/sprite-frame-before-after.png"));
    sf::Texture profile;assert(profile.loadFromFile("TP_2023192003_CLIENT/client_sample/profile.png"));
    for(auto size : {sf::Vector2u{900,800},sf::Vector2u{1280,720},sf::Vector2u{800,900}}) {
        sf::RenderTexture hud;assert(hud.create(size.x,size.y));hud.clear({48,32,37});
        const auto fit=display_layout::fit(size.x,size.y);
        sf::View view(sf::FloatRect(0,0,900,800));
        view.setViewport({fit.left/size.x,fit.top/size.y,fit.width/size.x,fit.height/size.y});hud.setView(view);
        sf::Sprite back(profile);back.setPosition(15,664);hud.draw(back);
        const sf::String names[]={"Ham","WardrobeDB","WWWWWWWWWWWW",L"\uD584\uC2A4\uD130\uD584\uC2A4\uD130\uD584\uC2A4\uD130\uD584\uC2A4\uD130"};
        for(const auto& name: names) {
            sf::Text text(name,font,30);profile_layout::placeName(text);
            const auto b=text.getGlobalBounds(),slot=profile_layout::nameSlot();
            assert(b.left>=slot.left && b.top>=slot.top && b.left+b.width<=slot.left+slot.width && b.top+b.height<=slot.top+slot.height);
        }
        sf::Text text("WardrobeDB",font,30);profile_layout::placeName(text);text.setFillColor(sf::Color::Black);hud.draw(text);
        hud.display();assert(hud.getTexture().copyToImage().saveToFile("build/tests/profile-name-"+std::to_string(size.x)+"x"+std::to_string(size.y)+".png"));
    }
    std::cout<<"24 resident frames: authored-cell pixel completeness/no contamination; long-name slot and renderer previews PASS\n";
}
