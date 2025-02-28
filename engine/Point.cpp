#include "structs.hpp"
#include "lua_util.hpp"
#include "Tag.hpp"

constexpr auto Self = Tag::Point;

static auto index(lua_State* L) -> int {
    auto& self = to_object<Self>(L, 1);
    const char initial = *luaL_checkstring(L, 2);
    switch (initial) {
        case 'x':
            lua::push(L, self.x);
        return 1;
        case 'y':
            lua::push(L, self.y);
        return 1;
    }
    lua::arg_error(L, 2, "invalid field");
}
static auto newindex(lua_State* L) -> int {
    auto& self = to_object<Self>(L, 1);
    const char initial = *luaL_checkstring(L, 2);
    const int val = luaL_checkinteger(L, 3);
    switch (initial) {
        case 'x':
            self.x = val;
        return 0;
        case 'y':
            self.y = val;
        return 0;
    }
    lua::arg_error(L, 2, "invalid field");
}
static auto tostring(lua_State* L) -> int {
    auto& self = to_object<Self>(L, 1);
    lua::push(L, "Point: {{{}, {}}}", self.x, self.y);
    return 1;
}

void Point::push_metatable(lua_State *L) {
    if (luaL_newmetatable(L, get_metatable_name<Self>())) {
        const luaL_Reg meta[] = {
            {"__index", index},
            {"__newindex", newindex},
            {"__tostring", tostring},
            {nullptr, nullptr}
        };
        luaL_register(L, nullptr, meta);
        lua_pushstring(L, "Point");
        lua_setfield(L, -2, "__type");
    }
}

void Point::push_constructor(lua_State *L) {
    auto constructor = [](auto L) {
        SDL_Point self{
            .x = luaL_optinteger(L, 1, 0),
            .y = luaL_optinteger(L, 2, 0),
        };
        new_object<Self>(L) = std::move(self);
        return 1;
    };
    lua_pushcfunction(L, constructor, "Point");
}
