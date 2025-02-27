#include "engine.hpp"
#include "util.hpp"
#include "Namecall_Atom.hpp"

auto Console::Meta::index(lua_State *L) -> int {
    return 0;
}
auto Console::Meta::newindex(lua_State *L) -> int {
    return 0;
}
auto Console::Meta::namecall(lua_State *L) -> int {
    auto console = check_type<Tag::Console>(L, 1);
    if (not console) {
        return 0;
    }
    int atom{};
    lua_namecallatom(L, &atom);
    switch (static_cast<Namecall_Atom>(atom)) {
        case Namecall_Atom::comment:
            console->comment(luaL_checkstring(L, 2));
        break;
        case Namecall_Atom::warn: 
            console->warn(luaL_checkstring(L, 2));
        break;
        default:
            console->error(std::format("invalid method for {}", typeid(Console).name()));
        break;
    }
    return 0;

}
void Console::Meta::init(lua_State *L) {
    luaL_newmetatable(L, typeid(Console).name());
    const luaL_Reg meta[] = {
        {"__namecall", namecall},
        {nullptr, nullptr}
    };
    luaL_register(L, nullptr, meta);
    lua_pop(L, 1);
}

auto Engine::Meta::index(lua_State *L) -> int {
    return 0;
}
auto Engine::Meta::newindex(lua_State *L) -> int {
    auto state = to_type<Tag::Engine>(L, 1);
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
auto Engine::Meta::namecall(lua_State *L) -> int {
    auto state = to_type<Tag::Engine>(L, 1);
    int atom{};
    lua_namecallatom(L, &atom);
    switch (static_cast<Namecall_Atom>(atom)) {
        case Namecall_Atom::get_console:
            push_type<Tag::Console>(L, &state->console);
        return 1;
        default:
            state->console.error(std::format("invalid method for {}", typeid(Engine).name()));
        break;
    }
    return 0;

}
void Engine::Meta::init(lua_State *L) {
    luaL_newmetatable(L, typeid(Engine).name());
    const luaL_Reg meta[] = {
        {"__newindex", newindex},
        {"__namecall", namecall},
        {nullptr, nullptr}
    };
    luaL_register(L, nullptr, meta);
    lua_pop(L, 1);
}
