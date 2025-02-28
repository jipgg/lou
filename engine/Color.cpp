#include "structs.hpp"
#include "lua_util.hpp"
#include "Tag.hpp"

static auto index(lua_State* L) -> int {
    auto& self = to_object<Tag::Color>(L, 1);
    const char initial = *luaL_checkstring(L, 2);
    switch (initial) {
        case 'r':
            lua::push(L, self.r);
        return 1;
        case 'g':
            lua::push(L, self.g);
        return 1;
        case 'b':
            lua::push(L, self.b);
        return 1;
        case 'a':
            lua::push(L, self.a);
        return 1;
    }
    lua::arg_error(L, 2, "invalid field");
}
static auto newindex(lua_State* L) -> int {
    auto& self = to_object<Tag::Color>(L, 1);
    const char initial = *luaL_checkstring(L, 2);
    const uint8_t val = static_cast<uint8_t>(luaL_checkinteger(L, 3));
    switch (initial) {
        case 'r':
            self.r = val;
        return 0;
        case 'g':
            self.g = val;
        return 0;
        case 'b':
            self.b = val;
        return 0;
        case 'a':
            self.a = val;
        return 0;
    }
    lua::arg_error(L, 2, "invalid field");
}
static auto tostring(lua_State* L) -> int {
    auto& self = to_object<Tag::Color>(L, 1);
    lua::push(L, "Color: {{{}, {}, {}, {}}}", self.r, self.g, self.b, self.a);
    return 1;
}

void Color::push_metatable(lua_State *L) {
    if (luaL_newmetatable(L, get_metatable_name<Tag::Color>())) {
        const luaL_Reg meta[] = {
            {"__index", index},
            {"__newindex", newindex},
            {"__tostring", tostring},
            {nullptr, nullptr}
        };
        luaL_register(L, nullptr, meta);
        lua_pushstring(L, "Color");
        lua_setfield(L, -2, "__type");
    }
}

void Color::push_constructor(lua_State *L) {
    auto constructor = [](auto L) {
        SDL_Color self{
            .r = static_cast<uint8_t>(luaL_optinteger(L, 1, 0)),
            .g = static_cast<uint8_t>(luaL_optinteger(L, 2, 0)),
            .b = static_cast<uint8_t>(luaL_optinteger(L, 3, 0)),
            .a = static_cast<uint8_t>(luaL_optinteger(L, 4, 0)),
        };
        new_object<Tag::Color>(L) = std::move(self);
        return 1;
    };
    lua_pushcfunction(L, constructor, "Color");
}
