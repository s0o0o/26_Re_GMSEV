#include "../shared/legacy_packet_adapter.hpp"
#include <cassert>
#include <cstring>
#include <vector>
int main() {
    LegacyPacketAdapter adapter;
    for(const auto region : {world_regions::Id::Clubhouse,world_regions::Id::SunnyPeak}) {
        const auto descriptor=world_regions::describe(region);
        for(int y=0;y<descriptor.height;++y) for(int x=0;x<descriptor.width;++x) {
            sc_packet_move packet{}; packet.size=sizeof(packet); packet.type=S2C_P_MOVE;
            assert(packet_position::write(packet,{region,x,y}));
            const auto original=packet;
            auto* decoded=adapter.decode(&packet,sizeof(packet)); assert(decoded);
            const auto* legacy=reinterpret_cast<sc_packet_move*>(decoded);
            assert(legacy->x==descriptor.originX+x && legacy->y==y);
            assert(std::memcmp(&original,&packet,sizeof(packet))==0); // Wire remains local.
        }
    }
    sc_packet_move move{}; move.size=sizeof(move); move.type=S2C_P_MOVE; move.x=10; move.y=10;
    for(const auto region : {0,3,255}) { move.region=static_cast<std::uint8_t>(region); assert(!adapter.decode(&move,sizeof(move))); }
    move.region=2;
    for(const auto x : {-1,world_regions::fieldWidth,world_regions::fieldWidth+22,32767}) { move.x=static_cast<short>(x); assert(!adapter.decode(&move,sizeof(move))); }
    move.x=10; move.y=world_regions::fieldHeight; assert(!adapter.decode(&move,sizeof(move)));
    const auto before=move; assert(!packet_position::write(move,{})); assert(std::memcmp(&before,&move,sizeof(move))==0);
    move.y=10; move.size=sizeof(move)-1; assert(!adapter.decode(&move,move.size)); // Old MOVE length.
    sc_packet_avatar_info info{}; info.size=sizeof(info); info.type=S2C_P_AVATAR_INFO;
    assert(adapter.decode(&info,sizeof(info))); info.version=1; assert(!adapter.decode(&info,sizeof(info)));
    info.version=PROTOCOL_VERSION; info.size=sizeof(info)-2; assert(!adapter.decode(&info,info.size));
    assert(!is_valid_client_packet(C2S_P_LOGIN,sizeof(cs_packet_login)-1));
    sc_packet_mapInfo map{}; map.size=sizeof(map); map.type=S2C_P_MAPINFO; map.region=2; map.left_x=2; map.top_y=14;
    const auto* local=adapter.decode(&map,sizeof(map)); assert(local);
    assert(reinterpret_cast<const sc_packet_mapInfo*>(local)->left_x==34 && map.left_x==2);
    map.left_x=world_regions::fieldWidth-17; assert(!adapter.decode(&map,sizeof(map)));
    map.left_x=2; map.top_y=world_regions::fieldHeight-3; assert(!adapter.decode(&map,sizeof(map)));
    assert(!adapter.decode(nullptr,0));
}
