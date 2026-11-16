#define main hamster_game_main
#include "../TP_2023192003_CLIENT/client_sample/client.cpp"
#undef main
#include <cassert>
#include <filesystem>
int main() {
    std::puts("native wardrobe wire: begin"); std::fflush(stdout);
    const auto root=std::filesystem::current_path();
    std::filesystem::current_path(root/"TP_2023192003_CLIENT/client_sample");
    assert(gameFont().loadFromFile("neodgm.ttf") && hamsterCustomizer().load("assets/characters"));
    sf::Texture placeholder; assert(placeholder.loadFromFile("assets/characters/hamtaro/character.png")); myHam=&placeholder;
    loginFlow.finishCredentials(); loginFlow.connect(); loginFlow.accept();
    g_myid=avatar.id=1; avatar.setAppearance({}); moveAvatar({world_regions::Id::Clubhouse,11,12}); avatar.show();
    using avatar_appearance::Value; const Value changed{1,4,2,2};
    assert(wardrobeState.open(avatar.position,avatar.appearance) && wardrobeState.edit(changed));
    cs_packet_appearance request{}; assert(wardrobeState.submit(100,request));
    assert(avatar.appearance==Value{}); // Draft is not visible in the authoritative world.
    avatar.get_sprite().setTextureRect({96,0,48,48});
    const auto rect=avatar.get_sprite().getTextureRect();
    sc_packet_appearance_change change{sizeof(change),S2C_P_APPEARANCE_CHANGE,1,1,changed};
    process_data(reinterpret_cast<char*>(&change),sizeof(change));
    assert(avatar.appearance==changed && avatar.get_sprite().getTextureRect()==rect);
    assert(avatar.get_sprite().getTexture()==&hamsterCustomizer().worldTexture(4,2,2,1));
    sc_packet_appearance_result result{sizeof(result),S2C_P_APPEARANCE_RESULT,request.requestId,wardrobe::Result::Saved,changed};
    process_data(reinterpret_cast<char*>(&result),sizeof(result));
    assert(wardrobeState.stage==wardrobe::Stage::Applied && wardrobeState.baseline==changed);
    sc_packet_enter enter{}; enter.size=sizeof(enter); enter.type=S2C_P_ENTER; enter.id=2; enter.x=12; enter.y=12;
    enter.character_kind=1; enter.fur_color=4; enter.eye_style=2; enter.scarf_style=2; strcpy_s(enter.name,"WARD");
    ProcessPacket(reinterpret_cast<char*>(&enter)); assert(players.at(2).appearance==changed);
    change.id=2; change.appearance={0,2,1,1}; ProcessPacket(reinterpret_cast<char*>(&change));
    assert(players.at(2).appearance==change.appearance);
    change.region=2; change.appearance={}; ProcessPacket(reinterpret_cast<char*>(&change));
    assert(players.at(2).appearance!=Value{}); // Stale cross-region change is ignored.
    change.id=3; change.region=1; ProcessPacket(reinterpret_cast<char*>(&change)); assert(!players.count(3));
    assert(wardrobeState.close() && wardrobeState.open(avatar.position,avatar.appearance));
    moveAvatar({world_regions::Id::SunnyPeak,10,10}); assert(!wardrobeState.visible());
    gameConnectionLost(); assert(!wardrobeState.connected && !wardrobeState.pending());
    sf::RenderTexture scene; assert(scene.create(900,400)); scene.clear({255,248,228});
    for(int kind=0;kind<2;++kind) for(int direction=0;direction<4;++direction) {
        sf::Sprite dressed(hamsterCustomizer().worldTexture(4,2,2,static_cast<std::uint8_t>(kind)),{48*direction,0,48,48});
        world_character::place(dressed,130.f+direction*170,100.f+kind*160); scene.draw(dressed);
    }
    scene.display(); assert(scene.getTexture().copyToImage().saveToFile((root/"build/tests/wardrobe-wire-preview.png").string()));
    std::filesystem::current_path(root);
    std::puts("native wardrobe wire: no optimistic change, self/observer/late ENTER, facing preserved, stale region, close/disconnect PASS (wire fixture; UI checked separately)");
}
