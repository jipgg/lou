#include "structs.hpp"
#include "lua_util.hpp"
#include "Tag.hpp"

static auto index(lua_State* L) -> int {
    auto& rect = to_object<Tag::Rect>(L, 1);
    const char initial = *luaL_checkstring(L, 2);
    switch (initial) {
        case 'x':
            lua::push(L, rect.x);
        return 1;
        case 'y':
            lua::push(L, rect.y);
        return 1;
        case 'w':
            lua::push(L, rect.w);
        return 1;
        case 'h':
            lua::push(L, rect.h);
        return 1;
    }
    lua::arg_error(L, 2, "invalid field");
}
static auto newindex(lua_State* L) -> int {
    auto& rect = to_object<Tag::Rect>(L, 1);
    const char initial = *luaL_checkstring(L, 2);
    const int val = luaL_checkinteger(L, 3);
    switch (initial) {
        case 'x':
            rect.x = val;
        return 0;
        case 'y':
            rect.y = val;
        return 0;
        case 'w':
            rect.w = val;
        return 0;
        case 'h':
            rect.h = val;
        return 0;
    }
    lua::arg_error(L, 2, "invalid field");
}
static auto tostring(lua_State* L) -> int {
    auto& self = to_object<Tag::Rect>(L, 1);
    lua::push(L, "Rect: {{{}, {}, {}, {}}}", self.x, self.y, self.w, self.h);
    return 1;
}

void Rect::push_metatable(lua_State *L) {
    if (luaL_newmetatable(L, get_metatable_name<Tag::Rect>())) {
        const luaL_Reg meta[] = {
            {"__index", index},
            {"__newindex", newindex},
            {"__tostring", tostring},
            {nullptr, nullptr}
        };
        luaL_register(L, nullptr, meta);
        lua_pushstring(L, "Rect");
        lua_setfield(L, -2, "__type");
    }
}

void Rect::push_constructor(lua_State *L) {
    auto constructor = [](auto L) {
        SDL_Rect rect{
            .x = luaL_optinteger(L, 1, 0),
            .y = luaL_optinteger(L, 2, 0),
            .w = luaL_optinteger(L, 3, 0),
            .h = luaL_optinteger(L, 4, 0),
        };
        new_object<Tag::Rect>(L) = std::move(rect);
        return 1;
    };
    lua_pushcfunction(L, constructor, "Rect");
}
