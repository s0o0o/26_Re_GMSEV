#pragma once
#include "hamster_customizer.hpp"
#include "background_blur.hpp"
#include "ui_alignment.hpp"
#include "../../shared/wardrobe_client_state.hpp"

class WardrobePanel {
    sf::Texture icon_,backdrop_;
    bool hasBackdrop_=false,wDown_=false;
    int category_=0,armed_=0;
    static void text(sf::RenderTarget& target,const sf::Font& font,const sf::String& value,sf::FloatRect area,unsigned size=20) {
        sf::Text label(value,font,size); label.setFillColor({66,39,24}); centerText(label,area); target.draw(label);
    }
    static void box(sf::RenderTarget& target,sf::FloatRect area,bool selected,bool disabled,bool hovered) {
        sf::RectangleShape shape({area.width,area.height}); shape.setPosition(area.left,area.top);
        shape.setFillColor(disabled ? sf::Color(224,216,190) : selected ? sf::Color(242,212,155)
            : hovered ? sf::Color(255,231,166) : sf::Color(255,248,228));
        shape.setOutlineColor({130,79,28}); shape.setOutlineThickness(selected ? -3.f : -2.f); target.draw(shape);
    }
    int count() const { const int counts[]={2,5,3,3}; return counts[category_]; }
    std::uint8_t selected(avatar_appearance::Value value) const {
        switch(category_) { case 0:return value.kind; case 1:return value.fur; case 2:return value.eyes; default:return value.scarf; }
    }
    void choose(wardrobe::ClientState& state,int index) {
        auto value=state.draft;
        auto v=static_cast<std::uint8_t>(index);
        switch(category_) { case 0:value.kind=v;break; case 1:value.fur=v;break; case 2:value.eyes=v;break; default:value.scarf=v; }
        state.edit(value);
    }
    int hit(sf::Vector2f point) const {
        for(int i=0;i<4;++i) if(tab(i).contains(point)) return 1+i;
        for(int i=0;i<count();++i) if(item(i).contains(point)) return 10+i;
        if(apply.contains(point)) return 100;
        if(cancel.contains(point)) return 101;
        return 0;
    }
public:
    enum class Action { None, Opened, Apply, Closed };
    const sf::FloatRect entry{650,124,230,44},apply{260,650,180,48},cancel{460,650,180,48};
    static sf::FloatRect tab(int i) { return {150.f+i*150.f,290,140,48}; }
    sf::FloatRect item(int i) const { return {(900.f-(count()*116.f-12))/2+i*116.f,380,104,100}; }
    int category() const { return category_; }
    bool loadIcon(const std::string& path) {
        sf::Image image; if(!image.loadFromFile(path) || image.getSize().x<18 || image.getSize().y<18) return false;
        sf::Image first; first.create(18,18,sf::Color::Transparent);
        first.copy(image,0,0,{0,0,18,18});
        first.createMaskFromColor({0,128,128}); first.createMaskFromColor({144,144,144});
        if(!icon_.loadFromImage(first)) return false;
        icon_.setSmooth(false); return true;
    }
    bool setBackdrop(const sf::Image& image) {
        hasBackdrop_=backdrop_.loadFromImage(blurBackground(image)); return hasBackdrop_;
    }
    bool capture(const sf::RenderWindow& window) {
        const auto viewport=window.getViewport(window.getView());
        if(viewport.width<=0 || viewport.height<=0) return false;
        sf::Texture texture; if(!texture.create(window.getSize().x,window.getSize().y)) return false;
        texture.update(window); sf::Image image; image.create(viewport.width,viewport.height);
        image.copy(texture.copyToImage(),0,0,viewport); return setBackdrop(image);
    }
    Action handle(const sf::Event& event,sf::Vector2f point,wardrobe::ClientState& state,
                  world_regions::Position position,avatar_appearance::Value current,bool blocked) {
        if(event.type==sf::Event::LostFocus) { armed_=0; wDown_=false; }
        if(event.type==sf::Event::KeyReleased && event.key.code==sf::Keyboard::W) wDown_=false;
        bool freshW=false;
        if(event.type==sf::Event::KeyPressed && event.key.code==sf::Keyboard::W) { freshW=!wDown_; wDown_=true; }
        if(!state.visible()) {
            if(blocked || position.region!=world_regions::Id::Clubhouse) { armed_=0; return Action::None; }
            if(event.type==sf::Event::MouseButtonPressed && event.mouseButton.button==sf::Mouse::Left) armed_=entry.contains(point) ? 200 : 0;
            bool clicked=false;
            if(event.type==sf::Event::MouseButtonReleased && event.mouseButton.button==sf::Mouse::Left) {
                clicked=armed_==200 && entry.contains(point); armed_=0;
            }
            if((freshW || clicked) && state.open(position,current)) { category_=0; return Action::Opened; }
            return Action::None;
        }
        if(state.pending()) { armed_=0; return Action::None; }
        if(event.type==sf::Event::KeyPressed) {
            switch(event.key.code) {
            case sf::Keyboard::Tab: if(state.connected) category_=(category_+1)%4; break;
            case sf::Keyboard::Left: choose(state,(selected(state.draft)+count()-1)%count()); break;
            case sf::Keyboard::Right: choose(state,(selected(state.draft)+1)%count()); break;
            case sf::Keyboard::Enter: if(state.connected) return Action::Apply; break;
            case sf::Keyboard::Escape: if(state.close()) return Action::Closed; break;
            default: break;
            }
        }
        if(event.type==sf::Event::MouseButtonPressed && event.mouseButton.button==sf::Mouse::Left) armed_=hit(point);
        if(event.type==sf::Event::MouseButtonReleased && event.mouseButton.button==sf::Mouse::Left) {
            const int action=hit(point),old=armed_; armed_=0;
            if(action && action==old) {
                if(action>=1 && action<=4 && state.connected) category_=action-1;
                else if(action>=10 && action<15) choose(state,action-10);
                else if(action==100 && state.connected) return Action::Apply;
                else if(action==101 && state.close()) return Action::Closed;
            }
        }
        return Action::None;
    }
    void drawEntry(sf::RenderTarget& target,const sf::Font& font,bool disabled,sf::Vector2f mouse) const {
        box(target,entry,armed_==200,disabled,entry.contains(mouse));
        text(target,font,L"[W] \uC637\uC7A5 \uC5F4\uAE30",entry,22);
    }
    void draw(sf::RenderTarget& target,const sf::Font& font,const HamsterCustomizer& customizer,
              const wardrobe::ClientState& state,sf::Vector2f mouse={-1,-1}) const {
        if(hasBackdrop_) {
            sf::Sprite bg(backdrop_); bg.setScale(900.f/backdrop_.getSize().x,800.f/backdrop_.getSize().y); target.draw(bg);
        }
        sf::RectangleShape dim({900,800}); dim.setFillColor({0,0,0,100}); target.draw(dim);
        box(target,{115,70,670,660},false,false,false);
        if(icon_.getSize().x) {
            sf::Sprite icon(icon_); icon.setScale(2,2); icon.setPosition(160,92); target.draw(icon);
        }
        text(target,font,L"\uD074\uB7FD\uD558\uC6B0\uC2A4 \uC637\uC7A5",{205,90,490,36},28);
        const auto v=state.draft;
        const wchar_t* facing[]={L"\uB4A4",L"\uC55E",L"\uC67C\uCABD",L"\uC624\uB978\uCABD"};
        for(int i=0;i<4;++i) {
            sf::Sprite preview(customizer.texture(v.fur,v.eyes,v.scarf,v.kind),{48*i,0,48,48});
            preview.setScale(2,2); preview.setPosition(222.f+i*120,145); target.draw(preview);
            text(target,font,facing[i],{217.f+i*120,245,106,24},18);
        }
        const wchar_t* categories[]={L"\uCE90\uB9AD\uD130",L"\uD138\uC0C9",L"\uB208",L"\uBAA9\uB3C4\uB9AC"};
        const bool disabled=state.pending() || !state.connected;
        for(int i=0;i<4;++i) {
            const auto bounds=tab(i); box(target,bounds,i==category_ || armed_==i+1,disabled,bounds.contains(mouse));
            sf::IntRect crop=i==2 ? sf::IntRect(56,18,32,14) : i==3 ? sf::IntRect(56,31,32,17) : sf::IntRect(48,0,48,48);
            sf::Sprite icon(customizer.texture(v.fur,v.eyes,i==3 ? 1 : 0,v.kind),crop);
            icon.setOrigin(crop.width/2.f,crop.height/2.f); icon.setScale(i<2 ? .55f : .8f,i<2 ? .55f : .8f);
            icon.setPosition(bounds.left+24,bounds.top+24); target.draw(icon);
            text(target,font,categories[i],{bounds.left+42,bounds.top,bounds.width-46,bounds.height},20);
        }
        text(target,font,categories[category_],{150,347,600,26},18);
        const char* names[]={"Golden","Cream","Cocoa","Silver","Rose"};
        const sf::Color colors[]={{255,184,0},{235,212,160},{153,101,64},{160,170,185},{231,155,169}};
        const char* eyeNames[]={"Classic","Round","Sleepy"}; const char* scarfNames[]={"None","Red","Blue"};
        for(int i=0;i<count();++i) {
            const auto bounds=item(i); box(target,bounds,i==selected(v) || armed_==10+i,disabled,bounds.contains(mouse));
            if(category_==1) {
                sf::RectangleShape swatch({60,30}); swatch.setPosition(bounds.left+22,bounds.top+17); swatch.setFillColor(colors[i]); target.draw(swatch);
            } else {
                auto value=v; if(category_==0) value.kind=static_cast<std::uint8_t>(i);
                if(category_==2) value.eyes=static_cast<std::uint8_t>(i);
                if(category_==3) value.scarf=static_cast<std::uint8_t>(i);
                sf::Sprite icon(customizer.texture(value.fur,value.eyes,value.scarf,value.kind),{48,0,48,48});
                icon.setPosition(bounds.left+28,bounds.top+8); target.draw(icon);
            }
            text(target,font,category_==0 ? (i==0 ? "Hamtaro" : "Bijou") : category_==1 ? names[i] : category_==2 ? eyeNames[i] : scarfNames[i],
                {bounds.left,bounds.top+65,bounds.width,25},17);
        }
        text(target,font,L"Tab: \uBD84\uB958 / \uC88C\uC6B0: \uC120\uD0DD",{150,508,600,24},16);
        text(target,font,state.pending() ? L"\uC800\uC7A5 \uC644\uB8CC\uAE4C\uC9C0 \uAE30\uB2E4\uB824\uC8FC\uC138\uC694" :
            (!state.connected ? L"Esc: \uB2EB\uAE30" : L"Enter: \uC801\uC6A9 / Esc: \uB2EB\uAE30"),{150,534,600,24},16);
        sf::String status=L"\uC801\uC6A9\uD574\uC57C \uC2E4\uC81C \uC678\uD615\uC774 \uBC14\uB00C\uC5B4\uC694";
        switch(state.stage) {
        case wardrobe::Stage::Pending: status=L"\uC678\uD615\uC744 \uC800\uC7A5\uD558\uACE0 \uC788\uC5B4\uC694..."; break;
        case wardrobe::Stage::Applied: status=L"\uC800\uC7A5 \uC644\uB8CC. \uC7AC\uC811\uC18D \uD6C4\uC5D0\uB3C4 \uC720\uC9C0\uB3FC\uC694"; break;
        case wardrobe::Stage::Rejected: status=L"\uC800\uC7A5 \uC2E4\uD328. \uB2E4\uC2DC \uC801\uC6A9\uD558\uAC70\uB098 \uB2EB\uC544\uC8FC\uC138\uC694"; break;
        case wardrobe::Stage::TimedOut: status=L"\uC751\uB2F5 \uC2DC\uAC04 \uCD08\uACFC. \uB2E4\uC2DC \uC801\uC6A9\uD558\uAC70\uB098 \uC678\uD615\uC744 \uD655\uC778\uD558\uC138\uC694"; break;
        case wardrobe::Stage::Disconnected: status=L"\uC5F0\uACB0 \uC885\uB8CC. \uB2EB\uACE0 \uC7AC\uC811\uC18D\uD574\uC8FC\uC138\uC694"; break;
        default: break;
        }
        text(target,font,status,{145,575,610,46},18);
        box(target,apply,armed_==100,disabled,apply.contains(mouse)); text(target,font,state.pending() ? sf::String(L"\uC800\uC7A5 \uC911...") : sf::String(L"\uC801\uC6A9"),apply,22);
        box(target,cancel,armed_==101,state.pending(),cancel.contains(mouse)); text(target,font,state.stage==wardrobe::Stage::Editing ? sf::String(L"\uCDE8\uC18C") : sf::String(L"\uB2EB\uAE30"),cancel,22);
    }
};
