#include "../TP_2023192003/TP_2023192003/npc_script.hpp"
#include <cassert>
#include <cstring>

static int messages=0;
int getX(lua_State* L) {
    auto id=luaL_checkinteger(L,1);
    if(id == 10000 || id == 7) lua_pushinteger(L,10);
    else if(id == 8) lua_pushinteger(L,20);
    else lua_pushnil(L);
    return 1;
}
int getY(lua_State* L) {
    auto id=luaL_checkinteger(L,1);
    if(id == 10000 || id == 7 || id == 8) lua_pushinteger(L,10); else lua_pushnil(L);
    return 1;
}
int sendMessage(lua_State* L) {
    assert(lua_tointeger(L,lua_upvalueindex(1)) == 10000);
    assert(luaL_checkinteger(L,1) == 10000 && luaL_checkinteger(L,2) == 7);
    assert(std::strcmp(luaL_checkstring(L,3),"HELLO") == 0); ++messages; return 0;
}
int main() {
    const npc_script::Bindings bindings{getX,getY,sendMessage};
    std::weak_ptr<lua_State> weak;
    {
        auto loaded=npc_script::loadFile("TP_2023192003/TP_2023192003/npc.lua",10000,bindings);
        assert(loaded); weak=loaded.state; auto L=loaded.state.get(); assert(lua_gettop(L) == 0);
        lua_pushinteger(L,123456); // A caller-owned stack value must survive every invocation.
        std::string error;
        for(int i=0;i<1000;++i) {
            int direction=-1; assert(npc_script::direction(L,direction,error) && direction >= 0 && direction <= 3);
            assert(npc_script::notify(L,7,1,error));
            assert(lua_gettop(L) == 1 && lua_tointeger(L,1) == 123456);
        }
        assert(messages == 1);
        assert(npc_script::notify(L,7,2,error) && messages == 2);
        assert(npc_script::notify(L,8,1,error) && npc_script::notify(L,99,1,error) && messages == 2);
        for(const char* name : {"os","io","package","debug","dofile","loadfile","load","pcall","xpcall"}) {
            lua_getglobal(L,name); assert(lua_isnil(L,-1)); lua_pop(L,1);
        }
        lua_pushnil(L); lua_setglobal(L,"event_player_move");
        assert(!npc_script::notify(L,7,3,error) && lua_gettop(L) == 1);
    }
    assert(weak.expired());
    assert(!npc_script::loadFile("tests/__missing_npc_ai__.lua",10000,bindings));
    assert(!npc_script::loadText("function set_uid(id) end",10000,bindings));
    assert(!npc_script::loadText("syntax ???",10000,bindings));
    auto forever=npc_script::loadText("while true do end",10000,bindings);
    assert(!forever && forever.error.find("budget") != std::string::npos);
    for(const char* body : {"return 4","return -1","return 1.5","return 'left'","error({})","while true do end"}) {
        std::string script="function set_uid(id) end; function event_player_move(p,c) error('notify failed') end; function choose_direction() ";
        script+=body; script+=" end";
        auto loaded=npc_script::loadText(script.c_str(),10000,bindings); assert(loaded);
        lua_pushinteger(loaded.state.get(),77); std::string error;
        for(int i=0;i<100;++i) {
            int direction=-1;
            assert(!npc_script::direction(loaded.state.get(),direction,error) && !error.empty());
            assert(!npc_script::notify(loaded.state.get(),7,1,error));
            assert(lua_gettop(loaded.state.get()) == 1 && lua_gethook(loaded.state.get()) == nullptr);
        }
    }
}
