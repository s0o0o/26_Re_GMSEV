#pragma once
#include "include/lua.hpp"
#include <memory>
#include <cstdint>
#include <climits>
#include <string>
#include <initializer_list>

// Trusted project scripts, not a hostile-code sandbox or a wall-clock/memory limit.
// Runtime callers own world -> NPC combat -> Lua mutex before using these functions.
namespace npc_script {
using Handle=std::shared_ptr<lua_State>;
struct Bindings { lua_CFunction getX,getY,sendMessage; };
struct Loaded { Handle state; std::string error; explicit operator bool() const { return bool(state); } };
struct StackScope {
    lua_State* state; int top;
    explicit StackScope(lua_State* L) : state(L),top(lua_gettop(L)) {}
    ~StackScope() { lua_settop(state,top); }
};
inline void budgetExceeded(lua_State* L,lua_Debug*) { luaL_error(L,"NPC instruction budget exceeded"); }
inline bool protectedCall(lua_State* L,int arguments,int results,std::string& error) {
    const auto oldHook=lua_gethook(L); const int oldMask=lua_gethookmask(L),oldCount=lua_gethookcount(L);
    lua_sethook(L,budgetExceeded,LUA_MASKCOUNT,10000);
    const int status=lua_pcall(L,arguments,results,0);
    lua_sethook(L,oldHook,oldMask,oldCount);
    if(status == LUA_OK) return true;
    error=lua_type(L,-1) == LUA_TSTRING ? lua_tostring(L,-1) : "non-string Lua error";
    return false;
}
// The lookup itself stays inside pcall (even a global __index cannot escape protection).
inline int dispatch(lua_State* L) {
    const char* name=lua_tostring(L,1); const int arguments=lua_gettop(L)-1;
    lua_getglobal(L,name);
    if(!lua_isfunction(L,-1)) return luaL_error(L,"missing NPC callback: %s",name);
    lua_remove(L,1); lua_insert(L,1);
    lua_call(L,arguments,LUA_MULTRET);
    return lua_gettop(L);
}
inline int contract(lua_State* L) {
    for(const char* name : {"set_uid","event_player_move","choose_direction"}) {
        lua_getglobal(L,name);
        if(!lua_isfunction(L,-1)) return luaL_error(L,"missing NPC callback: %s",name);
        lua_pop(L,1);
    }
    return 0;
}
inline int openLibraries(lua_State* L) {
    for(const luaL_Reg& library : {luaL_Reg{LUA_GNAME,luaopen_base},luaL_Reg{LUA_MATHLIBNAME,luaopen_math},
            luaL_Reg{LUA_STRLIBNAME,luaopen_string},luaL_Reg{LUA_TABLIBNAME,luaopen_table},luaL_Reg{LUA_UTF8LIBNAME,luaopen_utf8}}) {
        luaL_requiref(L,library.name,library.func,1); lua_pop(L,1);
    }
    // Avoid filesystem/dynamic loading and script-level interception of the budget error.
    for(const char* name : {"dofile","loadfile","load","pcall","xpcall","collectgarbage"}) {
        lua_pushnil(L); lua_setglobal(L,name);
    }
    return 0;
}
inline bool invoke(lua_State* L,const char* name,std::initializer_list<lua_Integer> arguments,int results,std::string& error) {
    lua_pushcfunction(L,dispatch); lua_pushstring(L,name);
    for(auto argument : arguments) lua_pushinteger(L,argument);
    return protectedCall(L,static_cast<int>(arguments.size())+1,results,error);
}
inline Loaded load(const char* source,bool file,int owner,Bindings bindings) {
    Handle state(luaL_newstate(),[](lua_State* L) { if(L) lua_close(L); });
    if(!state) return {nullptr,"Lua allocation failed"};
    auto L=state.get(); StackScope stack(L); std::string error;
    lua_pushcfunction(L,openLibraries);
    if(!protectedCall(L,0,0,error)) return {nullptr,error};
    if(!bindings.getX || !bindings.getY || !bindings.sendMessage) return {nullptr,"missing C API bindings"};
    lua_pushcfunction(L,bindings.getX); lua_setglobal(L,"API_get_x");
    lua_pushcfunction(L,bindings.getY); lua_setglobal(L,"API_get_y");
    lua_pushinteger(L,owner); lua_pushcclosure(L,bindings.sendMessage,1); lua_setglobal(L,"API_SendMessage");
    const int status=file ? luaL_loadfile(L,source) : luaL_loadstring(L,source);
    if(status != LUA_OK) return {nullptr,lua_type(L,-1) == LUA_TSTRING ? lua_tostring(L,-1) : "Lua load failed"};
    if(!protectedCall(L,0,0,error)) return {nullptr,error};
    lua_pushcfunction(L,contract);
    if(!protectedCall(L,0,0,error) || !invoke(L,"set_uid",{owner},0,error)) return {nullptr,error};
    return {std::move(state),{}};
}
inline Loaded loadFile(const char* path,int owner,Bindings bindings) { return load(path,true,owner,bindings); }
inline Loaded loadText(const char* text,int owner,Bindings bindings) { return load(text,false,owner,bindings); }
inline bool notify(lua_State* L,int player,std::uint64_t connection,std::string& error) {
    if(!L) { error="missing Lua state"; return false; }
    StackScope stack(L);
    return invoke(L,"event_player_move",{player,static_cast<lua_Integer>(connection)},0,error);
}
inline bool direction(lua_State* L,int& output,std::string& error) {
    if(!L) { error="missing Lua state"; return false; }
    StackScope stack(L);
    if(!invoke(L,"choose_direction",{},1,error)) return false;
    if(!lua_isinteger(L,-1) || lua_tointeger(L,-1)<0 || lua_tointeger(L,-1)>3) {
        error="choose_direction must return integer 0..3"; return false;
    }
    output=static_cast<int>(lua_tointeger(L,-1)); return true;
}
}
