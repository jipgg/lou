#include "engine.hpp"
#include "lua_util.hpp"
#include "Namecall_Atom.hpp"
#include "Tag.hpp"


[[noreturn]] static auto err_invalid_method(lua_State* L, int atom, Tag tag) {
    lua::error(L,
        "invalid method for {} -> {}",
        compile_time::enum_item<Tag>(tag).name,
        compile_time::enum_item<Namecall_Atom>(atom).name
    );
}

static auto mouse_index(lua_State* L) -> int {
    auto& self = to_object<Tag::Mouse>(L, 1);
    std::string_view key = luaL_checkstring(L, 2);
    // gotta watch out with this. if accessed directly with `lou.mouse.x`
    // it returns the a constant initial value because of sandboxing.
    // easy workaround is first putting `lou.mouse` in a local variable,
    // but this might be quite confusing and cause complicated bugs for
    // the ones who are unaware of this fact. might make these methods
    // specifically for avoiding this confusion.
    float x, y;
    SDL_GetMouseState(&x, &y);
    switch (key.at(0)) {
        case 'x': 
            lua::push(L, x);
        return 1;
        case 'y':
            lua::push(L, y);
        return 1;
    }
    lua::arg_error(L, 2, "invalid field '{}'", key);
}
static auto mouse_namecall(lua_State* L) -> int {
    auto& self = to_object<Tag::Mouse>(L, 1);
    int atom{};
    lua_namecallatom(L, &atom);
    switch (static_cast<Namecall_Atom>(atom)) {
        case Namecall_Atom::pressed:
            self.pressed.add(L, 2);
        return 0;
        case Namecall_Atom::released:
            self.released.add(L, 2);
        return 0;
        case Namecall_Atom::moved:
            self.moved.add(L, 2);
        return 0;
        case Namecall_Atom::position: {
            float x, y;
            SDL_GetMouseState(&x, &y);
            lua::push(L, x);
            lua::push(L, y);
            return 2;
        }
        default:
            err_invalid_method(L, atom, Tag::Console);
        break;
    }
}
void Mouse::push_metatable(lua_State *L) {
    if (luaL_newmetatable(L, get_metatable_name<Tag::Mouse>())) {
        const luaL_Reg meta[] = {
            {"__index", mouse_index},
            {"__namecall", mouse_namecall},
            {nullptr, nullptr}
        };
        luaL_register(L, nullptr, meta);
        lua_pushstring(L, "Lou_Mouse");
        lua_setfield(L, -2, "__type");
    }
}
static auto keyboard_is_pressed(lua_State* L) -> int {
    const auto key_states = SDL_GetKeyboardState(nullptr);
    const auto key = SDL_GetScancodeFromName(luaL_checkstring(L, 2));
    lua::push(L, static_cast<bool>(key_states[key]));
    return 1;
}

static auto keyboard_namecall(lua_State* L) -> int {
    auto& self = to_object<Tag::Keyboard>(L, 1);
    int atom{};
    lua_namecallatom(L, &atom);
    switch (static_cast<Namecall_Atom>(atom)) {
        case Namecall_Atom::pressed:
            self.pressed.add(L, 2);
        return 0;
        case Namecall_Atom::released:
            self.released.add(L, 2);
        return 0;
        case Namecall_Atom::is_pressed:
        return keyboard_is_pressed(L);
        default:
            err_invalid_method(L, atom, Tag::Keyboard);
        break;
    }
}

void Keyboard::push_metatable(lua_State* L) {
    if (luaL_newmetatable(L, get_metatable_name<Tag::Keyboard>())) {
        const luaL_Reg meta[] = {
            {"__namecall", keyboard_namecall},
            {nullptr, nullptr}
        };
        luaL_register(L, nullptr, meta);
        lua_pushstring(L, "Lou_Keyboard");
        lua_setfield(L, -2, "__type");
    }
}

static auto console_namecall(lua_State *L) -> int {
    auto& self = to_object<Tag::Console>(L, 1);
    int atom{};
    lua_namecallatom(L, &atom);
    switch (static_cast<Namecall_Atom>(atom)) {
        case Namecall_Atom::print:
            self.comment(lua::tuple_tostring(L, 2));
        break;
        case Namecall_Atom::warn: 
            self.warn(lua::tuple_tostring(L, 2));
        break;
        case Namecall_Atom::error:
            self.error(lua::tuple_tostring(L,2));
        break;
        default:
            err_invalid_method(L, atom, Tag::Console);
        break;
    }
    return 0;

}
auto Console::push_metatable(lua_State* L) -> void {
    if (luaL_newmetatable(L, get_metatable_name<Tag::Console>())) {
        const luaL_Reg meta[] = {
            {"__namecall", console_namecall},
            {nullptr, nullptr}
        };
        luaL_register(L, nullptr, meta);
        lua_pushstring(L, "Lou_Console");
        lua_setfield(L, -2, "__type");
    }
}
static auto engine_index(lua_State* L) -> int {
    auto& state = to_object<Tag::Engine>(L, 1);
    std::string_view index = luaL_checkstring(L, 2);
    if (index == "console") {
        // no need to cache a lua_ref, cause it is a field in the global 'lou'.
        // when sandboxed this automatically resolves to 1 invocation.
        // Pretty cool. Only works with a global __index, however.
        push_reference<Tag::Console>(L, state.console);
        return 1;
    } else if (index == "keyboard") {
        push_reference<Tag::Keyboard>(L, state.keyboard);
        return 1;
    } else if (index == "mouse") {
        push_reference<Tag::Mouse>(L, state.mouse);
        return 1;
    }
    lua::error(L, "invalid field '{}'", index);
}
static auto engine_newindex(lua_State *L) -> int {
    auto& state = to_object<Tag::Engine>(L, 1);
    std::string_view index = luaL_checkstring(L, 2);
    lua::error(L, "invalid field '{}'", index);
}
static auto engine_namecall(lua_State *L) -> int {
    auto& engine = to_object<Tag::Engine>(L, 1);
    int atom{};
    lua_namecallatom(L, &atom);
    logger.log("atom is {}, {}", atom, compile_time::enum_item<Namecall_Atom>(atom).name);
    switch (static_cast<Namecall_Atom>(atom)) {
        case Namecall_Atom::on_update:
            engine.update_callback.add(L, 2);
        return 0;
        case Namecall_Atom::on_draw:
            engine.draw_callback.add(L, 2);
        return 0;
        default:
        break;
    }
    err_invalid_method(L, atom, Tag::Engine);
}
auto Engine::push_metatable(lua_State *L) -> void {
    if (luaL_newmetatable(L, get_metatable_name<Tag::Engine>())) {
        const luaL_Reg meta[] = {
            {"__index", engine_index},
            {"__newindex", engine_newindex},
            {"__namecall", engine_namecall},
            {nullptr, nullptr}
        };
        luaL_register(L, nullptr, meta);
        lua_pushstring(L, "Lou_Engine");
        lua_setfield(L, -2, "__type");
    }
}
