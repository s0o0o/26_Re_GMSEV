// Execute the real client ENTER/LEAVE path without running its network/login main.
#define main production_client_main_not_executed
#include "../TP_2023192003_CLIENT/client_sample/client.cpp"
#undef main
#include <cassert>
int main() {
    assert(townNpcRenderer().load("TP_2023192003_CLIENT/client_sample/assets/npcs/source"));
    assert(gameFont().loadFromFile("TP_2023192003_CLIENT/client_sample/neodgm.ttf"));
    sf::RenderWindow window(sf::VideoMode(900,800),"Town client packet regression"); window.setVisible(false); g_window=&window;
    g_left_x=world_layout::cameraX(clubhouse::spawnX); g_top_y=world_layout::cameraY(clubhouse::spawnY);
    for(const auto& definition : town_npcs::residents) {
        sc_packet_enter packet{}; packet.size=sizeof(packet); packet.type=S2C_P_ENTER;
        packet.id=definition.id; packet.monType=definition.type; packet.x=definition.x; packet.y=definition.y;
        packet.hp=packet.max_hp=1; strcpy_s(packet.name,definition.name);
        ProcessPacket(reinterpret_cast<char*>(&packet));
        const auto& object=players.at(definition.id);
        assert(object._monType==definition.type && object.id==definition.id && std::strcmp(object.name,definition.name)==0);
        assert(combatHud().hp(definition.id)==-1);
        sc_packet_move move{}; move.size=sizeof(move); move.type=S2C_P_MOVE; move.id=definition.id;
        move.x=definition.x+1; move.y=definition.y;
        ProcessPacket(reinterpret_cast<char*>(&move));
        assert(players.at(definition.id).townMotion.facing==town_motion::Right);
        const auto sample=players.at(definition.id).townMotion.sample(combatClock.getElapsedTime().asSeconds());
        assert(sample.x>=definition.x && sample.x<=definition.x+1);
        window.clear(); players.at(definition.id).draw(); window.display();
    }
    sc_packet_enter unknown{}; unknown.size=sizeof(unknown); unknown.type=S2C_P_ENTER;
    unknown.id=town_npcs::baseId+99; unknown.monType=999;
    ProcessPacket(reinterpret_cast<char*>(&unknown)); assert(!players.count(unknown.id));
    sf::Image pixels; pixels.create(104,52,sf::Color::White); sf::Texture texture; assert(texture.loadFromImage(pixels));
    monHamster=&texture; hunHamster=&texture;
    for(int type : {1,2}) {
        sc_packet_enter monster{}; monster.size=sizeof(monster); monster.type=S2C_P_ENTER; monster.id=MAX_USER+type;
        monster.monType=type; monster.hp=monster.max_hp=5; strcpy_s(monster.name,"Field");
        ProcessPacket(reinterpret_cast<char*>(&monster));
        assert(players.at(monster.id)._monType==type && combatHud().hp(monster.id)==5);
    }
    for(const auto& definition : town_npcs::residents) {
        sc_packet_leave leave{}; leave.size=sizeof(leave); leave.type=S2C_P_LEAVE; leave.id=definition.id;
        ProcessPacket(reinterpret_cast<char*>(&leave)); assert(!players.count(leave.id));
    }
    players.clear(); window.close(); g_window=nullptr;
    std::cout<<"Production client: resident ENTER/draw/LEAVE, no HP HUD, unknown type rejection, hostile types retained passed\n";
}
