#include "engine.hpp"
#include "util.hpp"
#include "Namecall_Atom.hpp"

static auto err_invalid_method(lua_State* L, int atom, Tag tag) {
            util::error(L,
                "invalid method for {} -> {}",
                comp::enum_item<Tag>(int(tag)).name,
                comp::enum_item<Namecall_Atom>(atom).name
            );
}

static auto console_namecall(lua_State *L) -> int {
    logger.log("userdatatag is {} in console", lua_lightuserdatatag(L, 1));
    lua_getmetatable(L, 1);
    lua_getfield(L, -1, "__type");
    std::string type = luaL_checkstring(L, -1);
    lua_pop(L, 1);
    luaL_getmetatable(L, typeid(Console).name());
    auto eq = lua_rawequal(L, -1, -2);
    lua_pop(L, 2);
    logger.log("are equal metatables? {}, {}", eq, type);
    assert(eq);

    auto& self = **(Console**)lua_touserdatatagged(L, 1, int(Tag::Console));
    int atom{};
    lua_namecallatom(L, &atom);
    switch (static_cast<Namecall_Atom>(atom)) {
        case Namecall_Atom::comment:
            self.comment(luaL_checkstring(L, 2));
        break;
        case Namecall_Atom::warn: 
            self.warn(luaL_checkstring(L, 2));
        break;
        default:
            err_invalid_method(L, atom, Tag::Console);
        break;
    }
    return 0;

}
auto Console::push_metatable(lua_State* L) -> void {
    if (luaL_newmetatable(L, typeid(Console).name())) {
        const luaL_Reg meta[] = {
            {"__namecall", console_namecall},
            {nullptr, nullptr}
        };
        luaL_register(L, nullptr, meta);
        lua_pushstring(L, "Console");
        lua_setfield(L, -2, "__type");
    }
}
auto Console::push_as_light_userdata(lua_State* L) -> void {
    auto& c = *(Console**)lua_newuserdatatagged(L, sizeof(Console*), int(Tag::Console));
    c = this;
    //lua_pushlightuserdatatagged(L, this, int(Tag::Console));
    push_metatable(L);
    lua_setmetatable(L, -2);
}
static auto engine_index(lua_State* L) -> int {
    return 0;
}
static auto engine_newindex(lua_State *L) -> int {
    auto state = (Engine*)lua_tolightuserdatatagged(L, 1, int(Tag::Engine));
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
static auto engine_namecall(lua_State *L) -> int {
    auto& engine = *(Engine*)lua_tolightuserdatatagged(L, 1, int(Tag::Engine));
    int atom{};
    lua_namecallatom(L, &atom);
    logger.log("atom is {}, {}", atom, comp::enum_item<Namecall_Atom>(atom).name);
    switch (static_cast<Namecall_Atom>(atom)) {
        case Namecall_Atom::get_console:
            engine.console.push_as_light_userdata(L);
            return 1;
        break;
        default:
        break;
    }
    err_invalid_method(L, atom, Tag::Engine);
    return 0;

}
auto Engine::push_metatable(lua_State *L) -> void {
    if (luaL_newmetatable(L, typeid(Engine).name())) {
        const luaL_Reg meta[] = {
            {"__newindex", engine_newindex},
            {"__namecall", engine_namecall},
            {nullptr, nullptr}
        };
        luaL_register(L, nullptr, meta);
        lua_pushstring(L, "Engine");
        lua_setfield(L, -2, "__type");
    }
}
auto Engine::push_as_light_userdata(lua_State* L) -> void {
    lua_pushlightuserdatatagged(L, this, int(Tag::Engine));
    push_metatable(L);
    lua_setmetatable(L, -2);
}
