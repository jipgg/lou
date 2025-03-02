#include "Lou.hpp"
#include "common.hpp"
constexpr int None = 0;
constexpr int Value = 1;

// quality-of-life
constexpr auto State = Tag::Lou_State;
constexpr auto Mouse = Tag::Lou_Mouse;
constexpr auto Keyboard = Tag::Lou_Keyboard;
constexpr auto Console = Tag::Lou_Console;
constexpr auto Window = Tag::Lou_Window;
constexpr auto Renderer = Tag::Lou_Renderer;
template <Tag Val>
[[noreturn]] static auto err_invalid_method(lua_State* L, auto atom) {
    lua::error(L,
        "invalid method for {} -> {}",
        compile_time::enum_item<Val>().name,
        compile_time::enum_item<Namecall_Atom>(static_cast<int>(atom)).name
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
template <class Ty>
constexpr void generic_destructor(lua_State* L, void* userdata) {
    static_cast<Ty*>(userdata)->~Ty();
}
template <Tag Val, lua_Destructor Destructor = generic_destructor<Type_For<Val>>>
constexpr void set_destructor(lua_State* L) {
    lua_setuserdatadtor(L, static_cast<int>(Val), Destructor);
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
    const char initial = *lua::check<const char*>(L, 2);
    switch (initial) {
        case 'x': return lua::values(L, self.x);
        case 'y': return lua::values(L, self.y);
    }
    lua::arg_error(L, 2, "invalid field");
}
static auto point_newindex(lua_State* L) -> int {
    auto& self = to_tagged<Tag::Point>(L, 1);
    const auto initial = *lua::check<const char*>(L, 2);
    const auto val = lua::check<float>(L, 3);
    switch (initial) {
        case 'x': self.x = val; return None;
        case 'y': self.y = val; return None;
    }
    lua::arg_error(L, 2, "invalid field");
}
static auto point_tostring(lua_State* L) -> int {
    auto& self = to_tagged<Tag::Point>(L, 1);
    return lua::values(L, std::format("Point: {{{}, {}}}", self.x, self.y));
}
static auto point_namecall(lua_State* L) -> int {
    auto& self = to_tagged<Tag::Point>(L, 1);
    auto [atom, name] = lua::namecall_atom<Namecall_Atom>(L);
    switch (atom) {
        case Namecall_Atom::as_tuple:
            return lua::values(L, self.x, self.y);
        default: break;
    }
    err_invalid_method<Tag::Point>(L, atom);
}
void Point::push_metatable(lua_State *L) {
    constexpr luaL_Reg meta[] = {
        {"__index", point_index},
        {"__newindex", point_newindex},
        {"__tostring", point_tostring},
        {"__namecall", point_namecall},
        {nullptr, nullptr}
    };
    basic_push_metatable<Tag::Point>(L, meta);
}
void Point::push_constructor(lua_State *L) {
    auto constructor = [](auto L) {
        SDL_FPoint self{
            .x = lua::opt<float>(L, 1),
            .y = lua::opt<float>(L, 2),
        };
        new_tagged<Tag::Point>(L) = std::move(self);
        return Value;
    };
    lua_pushcfunction(L, constructor, "Point");
}
// Rect meta implementation
static auto rect_index(lua_State* L) -> int {
    auto& rect = to_tagged<Tag::Rect>(L, 1);
    const char initial = *luaL_checkstring(L, 2);
    switch (initial) {
        case 'x': return lua::values(L, rect.x);
        case 'y': return lua::values(L, rect.y);
        case 'w': return lua::values(L, rect.w);
        case 'h': return lua::values(L, rect.h);
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
static auto rect_namecall(lua_State* L) -> int {
    auto& self = to_tagged<Tag::Rect>(L, 1);
    auto [atom, name] = lua::namecall_atom<Namecall_Atom>(L);
    switch (atom) {
        case Namecall_Atom::as_tuple:
            return lua::values(L, self.x, self.y, self.w, self.h);
        default: break;
    }
    err_invalid_method<Tag::Rect>(L, atom);
}

void Rect::push_metatable(lua_State *L) {
    if (new_metatable<Tag::Rect>(L)) {
        const luaL_Reg meta[] = {
            {"__index", rect_index},
            {"__newindex", rect_newindex},
            {"__tostring", rect_tostring},
            {"__namecall", rect_namecall},
            {nullptr, nullptr}
        };
        luaL_register(L, nullptr, meta);
        set_type_metamethod<Tag::Rect>(L);
    }
}
void Rect::push_constructor(lua_State *L) {
    auto constructor = [](auto L) {
        SDL_FRect rect{
            .x = lua::opt<float>(L, 1),
            .y = lua::opt<float>(L, 2),
            .w = lua::opt<float>(L, 3),
            .h = lua::opt<float>(L, 4),
        };
        new_tagged<Tag::Rect>(L) = std::move(rect);
        return 1;
    };
    lua_pushcfunction(L, constructor, "Rect");
}

// Color meta implementation
static auto color_index(lua_State* L) -> int {
    auto& self = to_tagged<Tag::Color>(L, 1);
    const auto initial = lua::check<char>(L, 2);
    switch (initial) {
        case 'r': return lua::values(L, self.r);
        case 'g': return lua::values(L, self.g);
        case 'b': return lua::values(L, self.b);
        case 'a': return lua::values(L, self.a);
    }
    lua::arg_error(L, 2, "invalid field");
}
static auto color_newindex(lua_State* L) -> int {
    auto& self = to_tagged<Tag::Color>(L, 1);
    const char initial = lua::check<char>(L, 2);
    const auto val = lua::check<uint8_t>(L, 3);
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
    return Value;
}
static auto color_namecall(lua_State* L) -> int {
    auto& self = to_tagged<Tag::Color>(L, 1);
    auto [atom, name] = lua::namecall_atom<Namecall_Atom>(L);
    switch (atom) {
        case Namecall_Atom::as_tuple:
            return lua::values(L, self.r, self.g, self.b, self.a);
        default: break;
    }
    err_invalid_method<Tag::Color>(L, atom);
}
void Color::push_metatable(lua_State *L) {
    if (new_metatable<Tag::Color>(L)) {
        const luaL_Reg meta[] = {
            {"__index", color_index},
            {"__newindex", color_newindex},
            {"__tostring", color_tostring},
            {"__namecall", color_namecall},
            {nullptr, nullptr}
        };
        luaL_register(L, nullptr, meta);
        set_type_metamethod<Tag::Color>(L);
    }
}
void Color::push_constructor(lua_State *L) {
    auto constructor = [](auto L) {
        SDL_Color self{
            .r = lua::opt<uint8_t>(L, 1),
            .g = lua::opt<uint8_t>(L, 2),
            .b = lua::opt<uint8_t>(L, 3),
            .a = lua::opt<uint8_t, 0xff>(L, 4),
        };
        new_tagged<Tag::Color>(L) = std::move(self);
        return Value;
    };
    lua_pushcfunction(L, constructor, "Color");
}
void Font::push_metatable(lua_State *L) {
    constexpr auto tag = Tag::Font;
    if (new_metatable<tag>(L)) {
        set_destructor<tag>(L);
        set_type_metamethod<tag>(L);
    }
}
void Font::push_constructor(lua_State* L) {
    auto constructor = [](lua_State* L) -> int {
        auto [file, size] = lua::check_args<const char*, int>(L);
        Font font;
        font.ptr.reset(TTF_OpenFont(file, size));
        check_sdl(L, font.ptr != nullptr);
        make_tagged<Font>(L, std::move(font));
        return 1;
    };
    lua_pushcfunction(L, constructor, "Font");
}
static auto texture_index(lua_State* L) -> int {
    auto& self = to_tagged<Texture>(L, 1);
    auto initial = lua::check<char>(L, 2);
    switch (initial) {
        case 'w': return lua::values(L, self.width);
        case 'h': return lua::values(L, self.height);
    }
    lua::arg_error(L, 2, "invalid index");
}
void Texture::push_metatable(lua_State *L) {
    constexpr auto tag = Tag::Texture;
    if (new_metatable<tag>(L)) {
        set_destructor<tag>(L);
        const luaL_Reg meta[] = {
            {"__index", texture_index},
            {nullptr, nullptr}
        };
        set_type_metamethod<tag>(L);
    }
}
// Lou_Texture meta implementation
static auto lou_texture_namecall(lua_State* L) -> int {
    auto& self = to_tagged<Lou_Texture>(L, 1);
    auto [atom, name] = lua::namecall_atom<Namecall_Atom>(L);
    switch (atom) {
        case Namecall_Atom::from_text: {
            auto& font = to_tagged<Font>(L, 2);
            auto text = lua::check<std::string_view>(L, 3);
            auto color = to_tagged<Tag::Color>(L, 4);
            auto r = self.render_text_blended(font, text, color);
            if (!r) lua::error(L, r.error());
            make_tagged<Texture>(L, std::move(r.value()));
            return Value;
        }
        default: break;
    }
    err_invalid_method<Tag::Lou_Texture>(L, atom);
}
void Lou_Texture::push_metatable(lua_State *L) {
    constexpr luaL_Reg meta[] = {
        {"__namecall", lou_texture_namecall},
        {nullptr, nullptr}
    };
    basic_push_metatable<Tag::Lou_Texture>(L, meta);
}
// Lou_Window meta implementation
static auto window_namecall(lua_State* L) -> int {
    auto& window = to_tagged<Window>(L, 1);
    auto ptr = window.get();
    int atom;
    lua_namecallatom(L, &atom);
    switch (static_cast<Namecall_Atom>(atom)) {
        case Namecall_Atom::position: {
            int x, y;
            check_sdl(L, SDL_GetWindowPosition(window.get(), &x, &y));
            return lua::values(L, x, y);
        }
        case Namecall_Atom::size: {
            int w, h;
            check_sdl(L, SDL_GetWindowSize(window.get(), &w, &h));
            lua::push(L, w);
            lua::push(L, h);
            return lua::values(L, w, h);
        }
        case Namecall_Atom::resize: {
            const auto w = lua::check<int>(L, 2);
            const auto h = lua::check<int>(L, 3);
            check_sdl(L,SDL_SetWindowSize(window.get(), w, h));
            return None;
        }
        case Namecall_Atom::reposition: {
            const int x = lua::check<int>(L, 2);
            const int y = lua::check<int>(L, 3);
            check_sdl(L, SDL_SetWindowPosition(window.get(), x, y));
            return None;
        }
        case Namecall_Atom::opacity:
            return lua::values(L, SDL_GetWindowOpacity(ptr));
        case Namecall_Atom::set_opacity: {
            check_sdl(L, SDL_SetWindowOpacity(ptr, lua::check<float>(L, 2)));
            return None;
        }
        case Namecall_Atom::title: {
            return lua::values(L, SDL_GetWindowTitle(ptr));
        }
        case Namecall_Atom::set_title: {
            check_sdl(L, SDL_SetWindowTitle(ptr, lua::check<const char*>(L, 2)));
            return None;
        }
        case Namecall_Atom::maximize: {
            check_sdl(L, SDL_MaximizeWindow(ptr));
            return None;
        }
        case Namecall_Atom::minimize: {
            check_sdl(L, SDL_MinimizeWindow(ptr));
            return None;
        }
        case Namecall_Atom::restore: {
            check_sdl(L, SDL_RestoreWindow(ptr));
            return None;
        }
        case Namecall_Atom::enable_fullscreen: {
            check_sdl(L, SDL_SetWindowFullscreen(ptr, lua::check<bool>(L, 2)));
            return None;
        }
        case Namecall_Atom::size_in_pixels: {
            int w, h;
            check_sdl(L, SDL_GetWindowSizeInPixels(ptr, &w, &h));
            return lua::values(L, w, h);
        }
        case Namecall_Atom::aspect_ratio: {
            float min_aspect, max_aspect;
            check_sdl(L, SDL_GetWindowAspectRatio(ptr, &min_aspect, &max_aspect));
            return lua::values(L, min_aspect, max_aspect);
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
        return None;
        case Namecall_Atom::fill_rect:
            check_sdl(L, SDL_RenderFillRect(ptr, &to_tagged<Tag::Rect>(L, 2)));
        return None;
        case Namecall_Atom::draw_point: {
            auto [x, y] = lua::check_args<float, float>(L, 2);
            check_sdl(L, SDL_RenderPoint(ptr, x, y));
            return None;
        }
        case Namecall_Atom::clear: {
            check_sdl(L, SDL_RenderClear(ptr));
            return None;
        }
        case Namecall_Atom::draw_line: {
            auto [x1, y1, x2, y2] = lua::check_args<float, float, float, float>(L, 2);
            return None;
        }
        case Namecall_Atom::set_draw_color: {
            auto [r, g, b] = lua::check_args<Uint8, Uint8, Uint8>(L, 2);
            auto a = lua::opt<Uint8, SDL_ALPHA_OPAQUE>(L, 5);
            check_sdl(L, SDL_SetRenderDrawColor(ptr, r, g, b, a));
            return None;
        }
        case Namecall_Atom::get_draw_color: {
            Uint8 r, g, b, a;
            check_sdl(L, SDL_GetRenderDrawColor(ptr, &r, &g, &b, &a));
            return lua::values(L, r, g, b, a);
        }
        case Namecall_Atom::render_texture: {
            auto& texture = to_tagged<Texture>(L, 2);
            if (lua_isnumber(L, 3)) {
                auto [x, y] = lua::check_args<float, float>(L, 3);
                SDL_FRect rect{x, y, float(texture.width), float(texture.height)};
                check_sdl(L, SDL_RenderTexture(ptr, texture.ptr.get(), nullptr, &rect));
                return None;
            } else {
                auto& rect = to_tagged<Tag::Rect>(L, 3);
                check_sdl(L, SDL_RenderTexture(ptr, texture.ptr.get(), nullptr, &rect));
                return None;
            }
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
        case 'x': return lua::values(L, x);
        case 'y': return lua::values(L, y);
    }
    lua::arg_error(L, 2, "invalid field '{}'", key);
}
static auto mouse_namecall(lua_State* L) -> int {
    auto& self = to_tagged<Mouse>(L, 1);
    auto [atom, name] = lua::namecall_atom<Namecall_Atom>(L);
    switch (atom) {
        case Namecall_Atom::pressed:
            self.pressed.add(L, 2);
        return None;
        case Namecall_Atom::released:
            self.released.add(L, 2);
        return None;
        case Namecall_Atom::moved:
            self.moved.add(L, 2);
        return None;
        case Namecall_Atom::position: {
            float x, y;
            SDL_GetMouseState(&x, &y);
            return lua::values(L, x, y);
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
    return lua::values<bool>(L, key_states[key]);
}
static auto keyboard_namecall(lua_State* L) -> int {
    auto& self = to_tagged<Keyboard>(L, 1);
    auto [atom, nam] = lua::namecall_atom<Namecall_Atom>(L);
    switch (atom) {
        case Namecall_Atom::pressed:
            self.pressed.add(L, 2);
        return None;
        case Namecall_Atom::released:
            self.released.add(L, 2);
        return None;
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
    auto [atom, name] = lua::namecall_atom<Namecall_Atom>(L);
    switch (atom) {
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
        return Value;
    } else if (index == "keyboard") {
        push_tagged(L, state.keyboard);
        return Value;
    } else if (index == "mouse") {
        push_tagged(L, state.mouse);
        return Value;
    } else if (index == "window") {
        push_tagged(L, state.window);
        return Value;
    } else if (index == "renderer") {
        push_tagged(L, state.renderer);
        return Value;
    } else if (index == "texture") {
        push_tagged(L, state.texture);
        return Value;
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
