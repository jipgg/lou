#include "engine.hpp"
#include "util.hpp"

auto rectIndex(lua_State* L) -> int {
    lua_pushstring(L, "yo");
    return 1;
}

auto lua_rectCtor(lua_State* L) -> int {
    SDL_Rect rect{
        .x = luaL_optinteger(L, 1, 0),
        .y = luaL_optinteger(L, 2, 0),
        .w = luaL_optinteger(L, 3, 0),
        .h = luaL_optinteger(L, 4, 0),
    };
    newRect(L) = rect;
    return 1;
}

auto initRect(lua_State *L) -> void {
    if (luaL_newmetatable(L, "Rect")) {
        const luaL_Reg meta[] = {
            {"__index", rectIndex},
            {nullptr, nullptr}
        };
        luaL_register(L, nullptr, meta);
        lua_pushstring(L, "Rect");
        lua_setfield(L, -2, "__type");
        lua_setuserdatametatable(L, int(Tag::Rect), -1);
    }
    lua_pop(L, 1);
    lua_pushcfunction(L, lua_rectCtor, "rectCtor");
    lua_setglobal(L, "Rect");
}
