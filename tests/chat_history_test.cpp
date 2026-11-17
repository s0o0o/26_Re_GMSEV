#include "../TP_2023192003_CLIENT/client_sample/chat_panel.hpp"
#include <cassert>
int main(){
    ChatPanel p;p.place(0,true);for(int i=0;i<20;++i)p.receive("Ham","Message "+std::to_string(i));
    assert(p.rows().size()==20);p.scroll(5);assert(p.offset()==5);
    const auto old=p.rows()[p.rows().size()-p.offset()-5];p.receive("Ham",std::string(119,'x'));
    assert(p.unread()==1 && p.offset()==8 && p.rows()[p.rows().size()-p.offset()-5]==old);
    p.scroll(10000);assert(p.offset()==p.rows().size()-5);for(int i=0;i<100;++i)p.receive("Ham","later");assert(p.lines().size()==64 && p.offset()<=p.rows().size()-5);
    sf::Event e{};e.type=sf::Event::KeyPressed;e.key.code=sf::Keyboard::End;p.handle(e,{-1,-1},false,true,0,[](auto&){return true;});assert(!p.offset() && !p.unread());
    e.key.code=sf::Keyboard::PageUp;p.handle(e,{-1,-1},true,true,0,[](auto&){return true;});assert(!p.offset());
    p.handle(e,{-1,-1},false,true,0,[](auto&){return true;});assert(p.offset()==5);
    e.key.code=sf::Keyboard::PageDown;p.handle(e,{-1,-1},false,true,0,[](auto&){return true;});assert(!p.offset());
    e.type=sf::Event::MouseWheelScrolled;e.mouseWheelScroll.wheel=sf::Mouse::VerticalWheel;e.mouseWheelScroll.delta=1;
    assert(!p.handle(e,{800,100},false,true,0,[](auto&){return true;}));assert(p.handle(e,{30,p.bounds().top+50},false,true,0,[](auto&){return true;}) && p.offset()==3);
    p.reset();assert(p.rows().empty() && !p.offset() && !p.unread());
    for(const auto& row:ChatPanel::wrap(sf::String(L"가나다라마바사아자차카타파하가나다라마바사아자차카타파하")))assert(row.getSize()<=21);
    std::puts("Chat wrap/scroll/receive-anchor/eviction/unread/modal/reset PASS");
}
