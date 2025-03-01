#include "Lou.hpp"
#include "common.hpp"

// quality-of-life
constexpr auto State = Tag::Lou_State;
constexpr auto Mouse = Tag::Lou_Mouse;
constexpr auto Keyboard = Tag::Lou_Keyboard;
constexpr auto Console = Tag::Lou_Console;
constexpr auto Window = Tag::Lou_Window;
constexpr auto Renderer = Tag::Lou_Renderer;
template <Tag Val>
[[noreturn]] static auto err_invalid_method(lua_State* L, int atom) {
    lua::error(L,
        "invalid method for {} -> {}",
        compile_time::enum_item<Val>().name,
        compile_time::enum_item<Namecall_Atom>(atom).name
    );
}
template <Tag Val>
constexpr auto set_type_metamethod(lua_State* L) {
    constexpr auto name = compile_time::enum_info<Val>().name;
    lua_pushlstring(L, name.data(), name.size());
    lua_setfield(L, -2, "__type");
}
template <Tag Val>
constexpr auto new_metatable(lua_State* L) -> bool {
    return luaL_newmetatable(L, get_metatable_name<Val>());
}
template <Tag Val>
constexpr void basic_push_metatable(lua_State* L, const luaL_Reg* meta) {
    if (new_metatable<Val>(L)) {
        luaL_register(L, nullptr, meta);
        set_type_metamethod<Val>(L);
    }
}
static void check_sdl(lua_State* L, bool result) {
    if (result) return;
    lua::error(L, SDL_GetError());
}
// Point meta implementation
static auto point_index(lua_State* L) -> int {
    auto& self = to_tagged<Tag::Point>(L, 1);
    const char initial = *luaL_checkstring(L, 2);
    switch (initial) {
        case 'x': lua::push(L, self.x); return 1;
        case 'y': lua::push(L, self.y); return 1;
    }
    lua::arg_error(L, 2, "invalid field");
}
static auto point_newindex(lua_State* L) -> int {
    auto& self = to_tagged<Tag::Point>(L, 1);
    const char initial = *luaL_checkstring(L, 2);
    const float val = static_cast<float>(luaL_checknumber(L, 3));
    switch (initial) {
        case 'x': self.x = val; return 0;
        case 'y': self.y = val; return 0;
    }
    lua::arg_error(L, 2, "invalid field");
}
static auto point_tostring(lua_State* L) -> int {
    auto& self = to_tagged<Tag::Point>(L, 1);
    lua::push(L, "Point: {{{}, {}}}", self.x, self.y);
    return 1;
}
void Point::push_metatable(lua_State *L) {
    constexpr luaL_Reg meta[] = {
        {"__index", point_index},
        {"__newindex", point_newindex},
        {"__tostring", point_tostring},
        {nullptr, nullptr}
    };
    basic_push_metatable<Tag::Point>(L, meta);
}
void Point::push_constructor(lua_State *L) {
    auto constructor = [](auto L) {
        SDL_FPoint self{
            .x = static_cast<float>(luaL_optnumber(L, 1, 0)),
            .y = static_cast<float>(luaL_optnumber(L, 2, 0)),
        };
        new_tagged<Tag::Point>(L) = std::move(self);
        return 1;
    };
    lua_pushcfunction(L, constructor, "Point");
}
// Rect meta implementation
static auto rect_index(lua_State* L) -> int {
    auto& rect = to_tagged<Tag::Rect>(L, 1);
    const char initial = *luaL_checkstring(L, 2);
    switch (initial) {
        case 'x': lua::push(L, rect.x); return 1;
        case 'y': lua::push(L, rect.y); return 1;
        case 'w': lua::push(L, rect.w); return 1;
        case 'h': lua::push(L, rect.h); return 1;
    }
    lua::arg_error(L, 2, "invalid field");
}
static auto rect_newindex(lua_State* L) -> int {
    auto& rect = to_tagged<Tag::Rect>(L, 1);
    const char initial = *luaL_checkstring(L, 2);
    const float val = static_cast<float>(luaL_checknumber(L, 3));
    switch (initial) {
        case 'x': rect.x = val; return 0;
        case 'y': rect.y = val; return 0;
        case 'w': rect.w = val; return 0;
        case 'h': rect.h = val; return 0;
    }
    lua::arg_error(L, 2, "invalid field");
}
static auto rect_tostring(lua_State* L) -> int {
    auto& self = to_tagged<Tag::Rect>(L, 1);
    lua::push(L, "Rect: {{{}, {}, {}, {}}}", self.x, self.y, self.w, self.h);
    return 1;
}

void Rect::push_metatable(lua_State *L) {
    if (new_metatable<Tag::Rect>(L)) {
        const luaL_Reg meta[] = {
            {"__index", rect_index},
            {"__newindex", rect_newindex},
            {"__tostring", rect_tostring},
            {nullptr, nullptr}
        };
        luaL_register(L, nullptr, meta);
        set_type_metamethod<Tag::Rect>(L);
    }
}
void Rect::push_constructor(lua_State *L) {
    auto constructor = [](auto L) {
        SDL_FRect rect{
            .x = static_cast<float>(luaL_optnumber(L, 1, 0)),
            .y = static_cast<float>(luaL_optnumber(L, 2, 0)),
            .w = static_cast<float>(luaL_optnumber(L, 3, 0)),
            .h = static_cast<float>(luaL_optnumber(L, 4, 0)),
        };
        new_tagged<Tag::Rect>(L) = std::move(rect);
        return 1;
    };
    lua_pushcfunction(L, constructor, "Rect");
}

// Color meta implementation
static auto color_index(lua_State* L) -> int {
    auto& self = to_tagged<Tag::Color>(L, 1);
    const char initial = *luaL_checkstring(L, 2);
    switch (initial) {
        case 'r': lua::push(L, self.r); return 1;
        case 'g': lua::push(L, self.g); return 1;
        case 'b': lua::push(L, self.b); return 1;
        case 'a': lua::push(L, self.a); return 1;
    }
    lua::arg_error(L, 2, "invalid field");
}
static auto color_newindex(lua_State* L) -> int {
    auto& self = to_tagged<Tag::Color>(L, 1);
    const char initial = *luaL_checkstring(L, 2);
    const uint8_t val = static_cast<uint8_t>(luaL_checkinteger(L, 3));
    switch (initial) {
        case 'r': self.r = val; return 0;
        case 'g': self.g = val; return 0;
        case 'b': self.b = val; return 0;
        case 'a': self.a = val; return 0;
    }
    lua::arg_error(L, 2, "invalid field");
}
static auto color_tostring(lua_State* L) -> int {
    auto& self = to_tagged<Tag::Color>(L, 1);
    lua::push(L, "Color: {{{}, {}, {}, {}}}", self.r, self.g, self.b, self.a);
    return 1;
}
void Color::push_metatable(lua_State *L) {
    if (new_metatable<Tag::Color>(L)) {
        const luaL_Reg meta[] = {
            {"__index", color_index},
            {"__newindex", color_newindex},
            {"__tostring", color_tostring},
            {nullptr, nullptr}
        };
        luaL_register(L, nullptr, meta);
        set_type_metamethod<Tag::Color>(L);
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
        new_tagged<Tag::Color>(L) = std::move(self);
        return 1;
    };
    lua_pushcfunction(L, constructor, "Color");
}
// Lou_Window meta implementation

static auto window_namecall(lua_State* L) -> int {
    auto& window = to_tagged<Window>(L, 1);
    int atom;
    lua_namecallatom(L, &atom);
    switch (static_cast<Namecall_Atom>(atom)) {
        case Namecall_Atom::position: {
            int x, y;
            check_sdl(L, SDL_GetWindowPosition(window.get(), &x, &y));
            lua::push(L, x);
            lua::push(L, y);
            return 2;
        }
        case Namecall_Atom::size: {
            int w, h;
            check_sdl(L, SDL_GetWindowSize(window.get(), &w, &h));
            lua::push(L, w);
            lua::push(L, h);
            return 2;
        }
        case Namecall_Atom::resize: {
            const int w = luaL_checkinteger(L, 2);
            const int h = luaL_checkinteger(L, 3);
            check_sdl(L,SDL_SetWindowSize(window.get(), w, h));
            return 0;
        }
        case Namecall_Atom::move: {
            const int x = luaL_checkinteger(L, 2);
            const int y = luaL_checkinteger(L, 3);
            check_sdl(L, SDL_SetWindowPosition(window.get(), x, y));
            return 0;
        }
        default: break;
    }
    err_invalid_method<Window>(L, atom);
}
void Lou_Window::push_metatable(lua_State *L) {
    constexpr luaL_Reg meta[] = {
        {"__namecall", window_namecall},
        {nullptr, nullptr}
    };
    basic_push_metatable<Window>(L, meta);
}

// Lou_Renderer meta implementation
static auto renderer_namecall(lua_State* L) -> int {
    auto& renderer = to_tagged<Renderer>(L, 1);
    auto ptr = renderer.get();
    int atom;
    lua_namecallatom(L, &atom);
    switch (static_cast<Namecall_Atom>(atom)) {
        case Namecall_Atom::draw_rect:
            check_sdl(L, SDL_RenderRect(ptr, &to_tagged<Tag::Rect>(L, 2)));
        return 0;
        case Namecall_Atom::fill_rect:
            check_sdl(L, SDL_RenderFillRect(ptr, &to_tagged<Tag::Rect>(L, 2)));
        return 0;
        case Namecall_Atom::draw_point: {
            const float x = static_cast<float>(luaL_checknumber(L, 2));
            const float y = static_cast<float>(luaL_checknumber(L, 2));
            check_sdl(L, SDL_RenderPoint(ptr, x, y));
            return 0;
        }
        default: break;
    }
    err_invalid_method<Renderer>(L, atom);
}
void Lou_Renderer::push_metatable(lua_State *L) {
    constexpr luaL_Reg meta[] = {
        {"__namecall", renderer_namecall},
        {nullptr, nullptr}
    };
    basic_push_metatable<Renderer>(L, meta);
}
// Lou_Mouse meta implementation
static auto mouse_index(lua_State* L) -> int {
    auto& self = to_tagged<Mouse>(L, 1);
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
    auto& self = to_tagged<Mouse>(L, 1);
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
            err_invalid_method<Mouse>(L, atom);
        break;
    }
}
void Lou_Mouse::push_metatable(lua_State *L) {
    if (new_metatable<Mouse>(L)) {
        const luaL_Reg meta[] = {
            {"__index", mouse_index},
            {"__namecall", mouse_namecall},
            {nullptr, nullptr}
        };
        luaL_register(L, nullptr, meta);
        set_type_metamethod<Mouse>(L);
    }
}
// Lou_Keyboard meta implementation
static auto keyboard_is_pressed(lua_State* L) -> int {
    const auto key_states = SDL_GetKeyboardState(nullptr);
    const auto key = SDL_GetScancodeFromName(luaL_checkstring(L, 2));
    lua::push(L, static_cast<bool>(key_states[key]));
    return 1;
}
static auto keyboard_namecall(lua_State* L) -> int {
    auto& self = to_tagged<Keyboard>(L, 1);
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
            err_invalid_method<Keyboard>(L, atom);
        break;
    }
}
void Lou_Keyboard::push_metatable(lua_State* L) {
    if (new_metatable<Keyboard>(L)) {
        const luaL_Reg meta[] = {
            {"__namecall", keyboard_namecall},
            {nullptr, nullptr}
        };
        luaL_register(L, nullptr, meta);
        set_type_metamethod<Keyboard>(L);
    }
}
// Lou_Console meta implementation
static auto console_namecall(lua_State *L) -> int {
    auto& self = to_tagged<Console>(L, 1);
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
            err_invalid_method<Console>(L, atom);
        break;
    }
    return 0;

}
auto Lou_Console::push_metatable(lua_State* L) -> void {
    if (new_metatable<Console>(L)) {
        const luaL_Reg meta[] = {
            {"__namecall", console_namecall},
            {nullptr, nullptr}
        };
        luaL_register(L, nullptr, meta);
        set_type_metamethod<Console>(L);
    }
}
// Lou_State meta implementation
static auto state_index(lua_State* L) -> int {
    auto& state = to_tagged<State>(L, 1);
    std::string_view index = luaL_checkstring(L, 2);
    if (index == "console") {
        // no need to cache a lua_ref, cause it is a field in the global 'lou'.
        // when sandboxed this automatically resolves to 1 invocation.
        // Pretty cool. Only works with a global __index, however.
        push_tagged(L, state.console);
        return 1;
    } else if (index == "keyboard") {
        push_tagged(L, state.keyboard);
        return 1;
    } else if (index == "mouse") {
        push_tagged(L, state.mouse);
        return 1;
    }
    lua::error(L, "invalid field '{}'", index);
}
static auto state_newindex(lua_State *L) -> int {
    auto& state = to_tagged<State>(L, 1);
    std::string_view index = luaL_checkstring(L, 2);
    lua::error(L, "invalid field '{}'", index);
}
static auto state_namecall(lua_State *L) -> int {
    auto& engine = to_tagged<State>(L, 1);
    int atom{};
    lua_namecallatom(L, &atom);
    logger.log("atom is {}, {}", atom, compile_time::enum_item<Namecall_Atom>(atom).name);
    switch (static_cast<Namecall_Atom>(atom)) {
        case Namecall_Atom::on_update:
            engine.on_update.add(L, 2);
        return 0;
        case Namecall_Atom::on_render:
            engine.on_render.add(L, 2);
        return 0;
        default:
        break;
    }
    err_invalid_method<State>(L, atom);
}
auto Lou_State::push_metatable(lua_State *L) -> void {
    if (new_metatable<State>(L)) {
        const luaL_Reg meta[] = {
            {"__index", state_index},
            {"__newindex", state_newindex},
            {"__namecall", state_namecall},
            {nullptr, nullptr}
        };
        luaL_register(L, nullptr, meta);
        set_type_metamethod<State>(L);
    }
}
