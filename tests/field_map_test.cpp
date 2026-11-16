#define main production_client_main
#include "../TP_2023192003_CLIENT/client_sample/client.cpp"
#undef main
#include <cassert>
#include <filesystem>
int main() {
    const auto root=std::filesystem::current_path();std::filesystem::current_path(root/"TP_2023192003_CLIENT/client_sample");
    assert(gameFont().loadFromFile("neodgm.ttf"));assert(clubhouseRenderer().loadOutdoor("assets/maps/sunny-peak-sheet.png"));
    sf::RenderWindow window(sf::VideoMode(900,800),"Field map regression",sf::Style::None);window.setVisible(false);g_window=&window;
    WindowDisplay::resize(window);travelState={};wardrobeState={};moveAvatar({world_regions::Id::SunnyPeak,90,96});
    sf::Event e{};
    auto key=[&](sf::Keyboard::Key k) {e={};e.type=sf::Event::KeyPressed;e.key.code=k;return handleFieldMapInput(e,{-1,-1});};
    auto release=[&]() {e={};e.type=sf::Event::KeyReleased;e.key.code=sf::Keyboard::M;handleFieldMapInput(e,{-1,-1});};
    isChatMode=true;assert(!key(sf::Keyboard::M) && !fieldMapPanel().visible());release();isChatMode=false;
    travelState.status=world_travel::ClientStatus::Pending;assert(!key(sf::Keyboard::M));release();travelState={};
    wardrobeState.open({world_regions::Id::Clubhouse,4,4},{});assert(!key(sf::Keyboard::M));release();wardrobeState={};
    assert(key(sf::Keyboard::M) && fieldMapPanel().visible());assert(key(sf::Keyboard::M) && fieldMapPanel().visible());release();
    assert(key(sf::Keyboard::R) && !travelState.pending());assert(key(sf::Keyboard::Left));
    assert(key(sf::Keyboard::Escape) && !fieldMapPanel().visible());
    auto click=[&](sf::FloatRect bounds,bool releaseInside=true) {
        const auto pixel=window.mapCoordsToPixel({bounds.left+bounds.width/2,bounds.top+bounds.height/2});
        e={};e.type=sf::Event::MouseButtonPressed;e.mouseButton.button=sf::Mouse::Left;handleFieldMapInput(e,window.mapPixelToCoords(pixel));
        e.type=sf::Event::MouseButtonReleased;handleFieldMapInput(e,releaseInside?window.mapPixelToCoords(pixel):sf::Vector2f{-1,-1});
    };
    click(FieldMapPanel::entry,false);assert(!fieldMapPanel().visible());click(FieldMapPanel::entry);assert(fieldMapPanel().visible());
    moveAvatar({world_regions::Id::Clubhouse,4,4});assert(!fieldMapPanel().visible());assert(!key(sf::Keyboard::M));release();
    moveAvatar({world_regions::Id::SunnyPeak,90,96});assert(key(sf::Keyboard::M));release();gameConnectionLost();assert(!fieldMapPanel().visible());
    assert(!key(sf::Keyboard::M));release();travelState={};
    for(const auto size:{sf::Vector2u{900,800},sf::Vector2u{1280,720},sf::Vector2u{800,900}}) {
        window.setSize(size);WindowDisplay::resize(window);click(FieldMapPanel::entry);assert(fieldMapPanel().visible());
        sf::RenderTexture out;assert(out.create(size.x,size.y));out.clear({32,24,24});
        const auto fit=display_layout::fit(size.x,size.y);sf::View view(sf::FloatRect(0,0,900,800));
        view.setViewport({fit.left/size.x,fit.top/size.y,fit.width/size.x,fit.height/size.y});out.setView(view);
        clubhouseRenderer().draw(out,avatar.position.region,g_left_x,g_top_y);fieldMapPanel().draw(out,gameFont(),avatar.position);out.display();
        assert(out.getTexture().copyToImage().saveToFile((root/("build/tests/field-map-"+std::to_string(size.x)+"x"+std::to_string(size.y)+".png")).string()));
        click(FieldMapPanel::closeButton);assert(!fieldMapPanel().visible());
    }
    for(const auto p:{world_regions::Position{world_regions::Id::SunnyPeak,0,0},{world_regions::Id::SunnyPeak,255,255}}) {
        const auto point=FieldMapPanel::mapPoint(p);assert(FieldMapPanel::mapArea.contains(point));
    }
    assert(FieldMapPanel::mapPoint({world_regions::Id::Clubhouse,10,10})==sf::Vector2f(-1,-1));
    assert(FieldMapPanel::mapPoint({world_regions::Id::SunnyPeak,256,0})==sf::Vector2f(-1,-1));
    sf::RenderTexture entry;assert(entry.create(900,800));entry.clear();
    clubhouseRenderer().draw(entry,avatar.position.region,g_left_x,g_top_y);
    travelPanel.draw(entry,gameFont(),avatar.position.region,travelState);
    fieldMapPanel().drawEntry(entry,gameFont(),false);entry.display();
    assert(entry.getTexture().copyToImage().saveToFile((root/"build/tests/field-map-entry.png").string()));
    fieldMapPanel().reset();window.close();g_window=nullptr;std::filesystem::current_path(root);
    std::puts("Field map production input: M/repeat/chat/pending/wardrobe/modal/region/disconnect/click/resize/markers + terrain renderer PASS");
}
