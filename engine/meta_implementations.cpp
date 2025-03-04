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
constexpr SDL_BlendMode string_to_blend_mode(const std::string_view str) {
    if (str == "none") return SDL_BLENDMODE_NONE;
    else if (str == "add") return SDL_BLENDMODE_ADD;
    else if (str == "add premultiplied") return SDL_BLENDMODE_ADD_PREMULTIPLIED;
    else if (str == "blend") return SDL_BLENDMODE_BLEND;
    else if (str == "blend premultiplied") return SDL_BLENDMODE_BLEND_PREMULTIPLIED;
    else if (str == "mod") return SDL_BLENDMODE_MOD;
    else if (str == "mul") return SDL_BLENDMODE_MUL;
    else return SDL_BLENDMODE_INVALID;
}
constexpr const char* blend_mode_to_string(SDL_BlendMode bm) {
    switch (bm) {
        case SDL_BLENDMODE_NONE: return "none";
        case SDL_BLENDMODE_ADD: return "add";
        case SDL_BLENDMODE_ADD_PREMULTIPLIED: return "add premultiplied";
        case SDL_BLENDMODE_BLEND: return "blend pemultiplied";
        case SDL_BLENDMODE_BLEND_PREMULTIPLIED: return "blend premultiplied";
        case SDL_BLENDMODE_MOD: return "mod";
        case SDL_BLENDMODE_MUL: return "mul";
        default: return "invalid";
    }
}
// V2 meta implementation
static auto v2_call(lua_State* L) -> int {
    const auto& self = to_tagged<Tag::V2>(L, 1);
    return lua::values(L, self.x, self.y);
}
static auto v2_index(lua_State* L) -> int {
    auto& self = to_tagged<Tag::V2>(L, 1);
    const char initial = *lua::check<const char*>(L, 2);
    switch (initial) {
        case 'x': return lua::values(L, self.x);
        case 'y': return lua::values(L, self.y);
    }
    lua::arg_error(L, 2, "invalid field");
}
static auto v2_newindex(lua_State* L) -> int {
    auto& self = to_tagged<Tag::V2>(L, 1);
    const auto initial = *lua::check<const char*>(L, 2);
    const auto val = lua::check<float>(L, 3);
    switch (initial) {
        case 'x': self.x = val; return None;
        case 'y': self.y = val; return None;
    }
    lua::arg_error(L, 2, "invalid field");
}
static auto v2_tostring(lua_State* L) -> int {
    auto& self = to_tagged<Tag::V2>(L, 1);
    return lua::values(L, std::format("V2{{{}, {}}}", self.x, self.y));
}
static auto v2_add(lua_State* L) -> int {
    auto& self = to_tagged<Tag::V2>(L, 1);
    auto& other = to_tagged<Tag::V2>(L, 2);
    make_tagged<Tag::V2>(L, self.x + other.x, self.y + other.y);
    return Value;
}
static auto v2_sub(lua_State* L) -> int {
    auto& self = to_tagged<Tag::V2>(L, 1);
    auto& other = to_tagged<Tag::V2>(L, 2);
    make_tagged<Tag::V2>(L, self.x - other.x, self.y - other.y);
    return Value;
}
static auto v2_mul(lua_State* L) -> int {
    auto a_opt = to_tagged_if<Tag::V2>(L, 1);
    auto b_opt = to_tagged_if<Tag::V2>(L, 2);
    if (a_opt and b_opt) {
        make_tagged<Tag::V2>(L,a_opt->x * b_opt->x, a_opt->y * b_opt->y);
        return Value;
    }
    const int num_idx = a_opt ? 2 : 1;
    const auto& v = a_opt ? *a_opt : *b_opt;
    auto scalar = lua::check<float>(L, num_idx);
    make_tagged<Tag::V2>(L, v.x * scalar, v.y * scalar);
    return Value;
}
static auto v2_namecall(lua_State* L) -> int {
    auto& self = to_tagged<Tag::V2>(L, 1);
    auto [atom, name] = lua::namecall_atom<Namecall_Atom>(L);
    auto vec = [](const auto& pt) {
        return blaze::StaticVector{pt.x, pt.y};
    };
    switch (atom) {
        case Namecall_Atom::as_tuple:
            return lua::values(L, self.x, self.y);
        case Namecall_Atom::dot: {
            const auto& other = to_tagged<Tag::V2>(L, 2);
            return lua::values(L, blaze::dot(vec(self), vec(other)));
        }
        case Namecall_Atom::length: {
            auto length = blaze::length(vec(self));
            return lua::values(L, length);
        }
        case Namecall_Atom::squared_length: {
            auto sl = blaze::sqrLength(vec(self));
            return lua::values(L, sl);
        }
        case Namecall_Atom::normalized: {
            auto unit = blaze::normalize(vec(self));
            make_tagged<Tag::V2>(L, unit[0], unit[1]);
            return Value;
        }
        default: break;
    }
    err_invalid_method<Tag::V2>(L, atom);
}
void V2::push_metatable(lua_State *L) {
    constexpr luaL_Reg meta[] = {
        {"__index", v2_index},
        {"__newindex", v2_newindex},
        {"__add", v2_add},
        {"__sub", v2_sub},
        {"__mul", v2_mul},
        {"__tostring", v2_tostring},
        {"__namecall", v2_namecall},
        {"__call", v2_call},
        {nullptr, nullptr}
    };
    basic_push_metatable<Tag::V2>(L, meta);
}
void V2::push_constructor(lua_State *L) {
    auto constructor = [](auto L) {
        SDL_FPoint self{
            .x = lua::opt<float>(L, 1),
            .y = lua::opt<float>(L, 2),
        };
        new_tagged<Tag::V2>(L) = std::move(self);
        return Value;
    };
    lua_pushcfunction(L, constructor, "Point");
}
// Rect meta implementation
static auto rect_call(lua_State* L) -> int {
    auto& self = to_tagged<Tag::Rect>(L, 1);
    return lua::values(L, self.x, self.y, self.w, self.h);
}
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
            {"__call", rect_call},
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
static auto color_call(lua_State* L) -> int {
    auto& c = to_tagged<Tag::Color>(L, 1);
    return lua::values(L, c.r, c.g, c.b, c.a);
}
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
            {"__call", color_call},
            {nullptr, nullptr}
        };
        luaL_register(L, nullptr, meta);
        set_type_metamethod<Tag::Color>(L);
    }
}
void Color::push_constructor(lua_State *L) {
    auto constructor = [](auto L) -> int {
        SDL_Color self{
            .r = lua::opt<Uint8>(L, 1),
            .g = lua::opt<Uint8>(L, 2),
            .b = lua::opt<Uint8>(L, 3),
            .a = lua::opt<Uint8, 0xff>(L, 4),
        };
        make_tagged<Tag::Color>(L, std::move(self));
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
        auto [file, size] = lua::check_args<const char*, float>(L);
        Font font;
        font.ptr.reset(TTF_OpenFont(file, size));
        check_sdl(L, font.ptr != nullptr);
        make_tagged<Font>(L, std::move(font));
        return 1;
    };
    lua_pushcfunction(L, constructor, "Font");
}
// Texture meta implementation
static auto texture_index(lua_State* L) -> int {
    auto& self = to_tagged<Texture>(L, 1);
    auto initial = lua::check<char>(L, 2);
    switch (initial) {
        case 'c': {
        }
    }
    lua::arg_error(L, 2, "invalid index");
}
static auto texture_namecall(lua_State* L) -> int {
    auto& self = to_tagged<Texture>(L, 1);
    auto renderer = SDL_GetRendererFromTexture(self.get());
    if (!renderer) lua::error(L, SDL_GetError());
    auto [atom, name] = lua::namecall_atom<Namecall_Atom>(L);
    switch (atom) {
        case Namecall_Atom::render: {
            if (lua_isnumber(L, 2)) {
                auto rect = self.source_rect();
                if (!rect) lua::error(L, rect.error());
                auto [x, y] = lua::check_args<float, float>(L, 2);
                rect->x = x;
                rect->y = y;
                rect->w = static_cast<float>(luaL_optnumber(L, 4, rect->w));
                rect->h = static_cast<float>(luaL_optnumber(L, 5, rect->h));
                check_sdl(L, SDL_RenderTexture(renderer, self.get(), nullptr, &rect.value()));
                return None;
            } else if (is_tagged<Tag::Rect>(L, 2)) {
                auto dst_rect = to_tagged<Tag::Rect>(L, 2);
                if (lua_isnumber(L, 3)) {
                    auto angle = lua::check<double>(L, 3);
                    auto center = to_tagged_if<Tag::V2>(L, 4);
                    auto src_rect = to_tagged_if<Tag::Rect>(L, 5);
                    check_sdl(L, SDL_RenderTextureRotated(renderer, self.get(), src_rect, &dst_rect, angle, center, SDL_FLIP_NONE));
                    return None;
                }
                auto src_rect = to_tagged_if<Tag::Rect>(L, 3);
                check_sdl(L, SDL_RenderTexture(renderer, self.get(), src_rect, &dst_rect));
                return None;
            }
        }
        case Namecall_Atom::render_rotated: {
            return None;
        }
        default: break;
    }
    err_invalid_method<Tag::Texture>(L, atom);
}
void Texture::push_metatable(lua_State *L) {
    constexpr auto tag = Tag::Texture;
    if (new_metatable<tag>(L)) {
        set_destructor<tag>(L);
        const luaL_Reg meta[] = {
            {"__index", texture_index},
            {"__namecall", texture_namecall},
            {nullptr, nullptr}
        };
        luaL_register(L, nullptr, meta);
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
        case Namecall_Atom::load_image: {
            auto texture = self.load_image(lua::check<const char*>(L, 2));
            if (!texture) lua::error(L, texture.error());
            make_tagged<Texture>(L, std::move(texture.value()));
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
                float w, h;
                check_sdl(L, SDL_GetTextureSize(texture.ptr.get(), &w, &h));
                SDL_FRect rect{x, y, w, h};
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
