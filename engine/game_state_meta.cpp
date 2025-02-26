#include "engine.hpp"
#include "util.hpp"
#include "Namecall_Atom.hpp"


constexpr auto Tag = Light_Type_Tag::game_state;

auto Game_State_Meta::index(lua_State *L) -> int {
    return 0;
}
auto Game_State_Meta::newindex(lua_State *L) -> int {
    auto state = to_light_userdata<Tag>(L, 1);
    std::string_view index = luaL_checkstring(L, 2);
    auto ref = Lua_Ref(L, 3);
    if (index == "update") {
        state->callbacks.update = std::move(ref);
    } else if (index == "draw") {
        state->callbacks.draw = std::move(ref);
    } else if (index == "key_up") {
        state->callbacks.key_up = std::move(ref);
    } else if (index == "key_down") {
        state->callbacks.key_down = std::move(ref);
    }
    return 0;

}
auto Game_State_Meta::namecall(lua_State *L) -> int {
    auto state = to_light_userdata<Tag>(L, 1);
    int atom{};
    lua_namecallatom(L, &atom);
    switch (static_cast<Namecall_Atom>(atom)) {
        case Namecall_Atom::print:
            state->print(luaL_checkstring(L, 2));
        break;
        default:
            util::error(L, "invalid method");
        break;
    }
    return 0;

}
void Game_State_Meta::push_metatable(lua_State *L) {
    const luaL_Reg meta[] = {
        {"__index", index},
        {"__newindex", newindex},
        {"__namecall", namecall},
        {nullptr, nullptr}
    };
    lua_newtable(L);
    luaL_register(L, nullptr, meta);
}
