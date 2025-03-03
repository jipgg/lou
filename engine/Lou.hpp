#pragma once
#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <SDL3_image/SDL_image.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_sdlrenderer3.h>
#include <imgui.h>
#include <memory>
#include <expected>
#include <string>
#include <lualib.h>
#include <chrono>
#include <print>
#include "common.hpp"

template <class Ty>
using C_Owner_t = std::unique_ptr<Ty, void(*)(Ty*)>; 

struct Rect {
    static void push_constructor(lua_State* L);
    static void push_metatable(lua_State* L);
};
struct Color {
    static void push_constructor(lua_State* L);
    static void push_metatable(lua_State* L);
};
struct Point {
    static void push_constructor(lua_State* L);
    static void push_metatable(lua_State* L);
};

struct Texture {
    C_Owner_t<SDL_Texture> ptr{nullptr, SDL_DestroyTexture};
    lua::Ref cached_color_ref;
    static void push_metatable(lua_State* L);
    auto source_rect() -> std::expected<SDL_FRect, std::string> const {
        float w, h;
        if (!SDL_GetTextureSize(ptr.get(), &w, &h)) {
            return std::unexpected(SDL_GetError());
        } 
        return SDL_FRect{0, 0, w, h};
    }
    constexpr auto get() -> SDL_Texture* const {
        return ptr.get();
    }
};

struct Font {
    C_Owner_t<TTF_Font> ptr{nullptr, TTF_CloseFont};
    auto open(const std::string& file, float font_size) -> std::expected<void, std::string> {
        ptr.reset(TTF_OpenFont(file.c_str(), font_size));
        if (not ptr) return std::unexpected(SDL_GetError());

        return {};
    };
    static void push_metatable(lua_State* L);
    static void push_constructor(lua_State* L);
};


struct Lou_Console {
    auto render() -> void;
    enum class Severity {
        Comment, Warning, Error
    };
    struct Entry {
        Severity severity;
        std::string message;
    };
    std::vector<Entry> entries{};
    bool open{true};
    bool is_dirty{false};
    template <Severity Severity>
    auto basic_print(const std::string& message) -> void {
        entries.emplace_back(Severity, stamp_time() + std::string(message));
        is_dirty = true;
    }
    auto comment(const std::string& message) -> void {
        return basic_print<Severity::Comment>(message);
    }
    auto warn(const std::string& message) -> void {
        return basic_print<Severity::Warning>(message);
    }
    auto error(const std::string& message) -> void {
        return basic_print<Severity::Error>(message);
    }
    static auto push_metatable(lua_State* L) -> void;
};

struct Lou_Keyboard {
    lua::Callback_List<std::string_view> pressed;
    lua::Callback_List<std::string_view> released;
    static void push_metatable(lua_State* L);
};
struct Lou_Mouse {
    lua::Callback_List<const std::string&, float, float> pressed;
    lua::Callback_List<const std::string&, float, float> released;
    lua::Callback_List<float, float> moved;
    static void push_metatable(lua_State* L);
};

struct Lou_Window {
    struct {
        C_Owner_t<SDL_Window> window{nullptr, SDL_DestroyWindow};
    } owning;
    constexpr auto get() -> SDL_Window* const {return owning.window.get();}
    static void push_metatable(lua_State* L);
};

struct Lou_Renderer {
    struct {
        C_Owner_t<SDL_Renderer> renderer{nullptr, SDL_DestroyRenderer};
        C_Owner_t<TTF_TextEngine> text_engine{nullptr, TTF_DestroyRendererTextEngine};
    } owning;
    constexpr auto get() -> SDL_Renderer* const {return owning.renderer.get();}
    constexpr auto get_text_engine() -> TTF_TextEngine* const {return owning.text_engine.get();}
    static void push_metatable(lua_State* L);
};
struct Lou_Texture {
    SDL_Renderer* renderer;
    auto render_text_blended(const Font& font, std::string_view text, SDL_Color color) -> std::expected<Texture, std::string> {
        auto surface = C_Owner_t<SDL_Surface>(TTF_RenderText_Blended(
            font.ptr.get(),
            text.data(),
            text.length(),
            color
        ), SDL_DestroySurface);
        auto texture = SDL_CreateTextureFromSurface(renderer, surface.get());
        if (not texture) return std::unexpected{SDL_GetError()};
        return Texture{
            .ptr{texture, SDL_DestroyTexture},
        };
    }
    auto load_image(const char* file) -> std::expected<Texture, std::string> {
        auto texture = IMG_LoadTexture(renderer, file);
        if (not texture) return std::unexpected(SDL_GetError());
        return Texture{
            .ptr{texture, SDL_DestroyTexture},
        };
    }
    static void push_metatable(lua_State* L);
};

struct Lou_State {
    struct {
        C_Owner_t<lua_State> luau{nullptr, lua_close};
    } owning;
    Lou_Window window;
    Lou_Renderer renderer;
    Lou_Texture texture;
    Lou_Console console;
    Lou_Keyboard keyboard;
    Lou_Mouse mouse;
    using Clock_t = std::chrono::steady_clock;
    using Time_Point_t = std::chrono::time_point<Clock_t>;
    struct {
        Time_Point_t last_frame_start{Clock_t::now()};
        SDL_Event event;
    } cache;
    lua::Callback_List<double> on_update;
    lua::Callback_List<void> on_render;
    bool running{true};
    struct Init_Info {
        std::string title{"engine"};
        int width{800};
        int height{600};
        SDL_WindowFlags flags{SDL_WINDOW_RESIZABLE};
        std::string script_entry_point{"game/init.luau"};
    };
    void init(Init_Info data);
    void init_luau();
    void update();
    void render();
    constexpr auto lua_state() -> lua_State* {return owning.luau.get();}
    static auto push_metatable(lua_State* L) -> void;
};

enum class Namecall_Atom: int16_t {
    load,
    render,
    render_rotated,
    load_image,
    render_texture,
    from_text,
    draw,
    print,
    comment,
    error,
    warn,
    get_console,
    get_window,
    get_renderer,
    get_callbacks,
    position,
    on_update,
    on_render,
    size_in_pixels,
    size_changed,
    resized,
    resize,
    reposition,
    restore,
    opacity,
    set_opacity,
    title,
    set_title,
    maximize,
    enable_fullscreen,
    minimize,
    size,
    resizable,
    set_resizable,
    aspect_ratio,
    as_tuple,
    clear,
    draw_rect,
    draw_point,
    set_draw_color,
    get_draw_color,
    set_blend_mode,
    get_blend_mode,
    draw_line,
    draw_triangle,
    draw_ellipse,
    fill_rect,
    fill_point,
    fill_line,
    fill_triangle,
    fill_ellipse,
    move,
    pressed,
    released,
    is_pressed,
    is_down,
    is_key_down,
    moved,
    COMPILE_TIME_ENUM_SENTINEL
};

#define Tag_Enum_Item_Generator(X) \
    X(Any)\
    X(Lou_State)\
    X(Lou_Console)\
    X(Rect)\
    X(Point)\
    X(Color)\
    X(Texture)\
    X(Font)\
    X(Lou_Window)\
    X(Lou_Renderer)\
    X(Lou_Keyboard)\
    X(Lou_Mouse)\
    X(Lou_Texture)\
    X(COMPILE_TIME_ENUM_SENTINEL)

enum class Tag {
    #define Generate_Tag_Enum(item) item,
    Tag_Enum_Item_Generator(Generate_Tag_Enum)
    #undef Generate_Tag_Enum
};

namespace detail {
enum class Tag_Reference {
    #define Generate_Tag_Reference_Enum(item) item = LUA_UTAG_LIMIT - static_cast<int>(Tag::item),
    Tag_Enum_Item_Generator(Generate_Tag_Reference_Enum)
    #undef Generate_Tag_Reference_Enum
};
#undef Tag_Enum_Item_Generator

template <Tag Tag> struct Mapped_Type;
template <class Ty> struct Mapped_Tag;

#define Map_Type_To_Tag(TAG, TYPE)\
template <> struct Mapped_Type<Tag::TAG> {using type = TYPE;};\
template <> struct Mapped_Tag<TYPE> {static constexpr Tag value = Tag::TAG;}

Map_Type_To_Tag(Point, SDL_FPoint);
Map_Type_To_Tag(Rect, SDL_FRect);
Map_Type_To_Tag(Color, SDL_Color);
Map_Type_To_Tag(Lou_State, Lou_State);
Map_Type_To_Tag(Lou_Console, Lou_Console);
Map_Type_To_Tag(Lou_Keyboard, Lou_Keyboard);
Map_Type_To_Tag(Lou_Mouse, Lou_Mouse);
Map_Type_To_Tag(Lou_Window, Lou_Window);
Map_Type_To_Tag(Lou_Renderer, Lou_Renderer);
Map_Type_To_Tag(Texture, Texture);
Map_Type_To_Tag(Font, Font);
Map_Type_To_Tag(Lou_Texture, Lou_Texture);

#undef Map_Type_To_Tag

template <Tag_Reference Val, typename As = Tag>
requires std::is_integral_v<As>
consteval auto tag_cast() -> As {
    return static_cast<As>(static_cast<int>(Val) - LUA_UTAG_LIMIT);
}
template <Tag Val, typename As = Tag_Reference>
requires std::is_integral_v<As>
consteval auto tag_reference_cast() -> As {
    return static_cast<As>(LUA_UTAG_LIMIT - static_cast<int>(Val));
}
}//detail
template <Tag Val>
using Type_For = typename detail::Mapped_Type<Val>::type;

template <class Ty>
concept Not_A_Pointer = not std::is_pointer_v<Ty>;
template <class Ty>
constexpr Tag Tag_For = detail::Mapped_Tag<Ty>::value;

template <Tag Val>
auto get_metatable_name() -> const char* {
    constexpr auto v = compile_time::enum_info<Val>().name;
    static const std::string name{v};
    return name.c_str();
}
template <detail::Tag_Reference Val>
auto get_metatable_name() -> decltype(auto) {
    return get_metatable_name<tag_cast<Val>()>();
}
template <Tag Val>
auto push_metatable(lua_State* L) -> void {
    luaL_getmetatable(L, get_metatable_name<Val>());
}
template <detail::Tag_Reference Val>
auto push_metatable(lua_State *L) -> decltype(auto) {
    return push_metatable<tag_cast<Val>()>();
}
template <Tag Val, class Ty = Type_For<Val>, class ...Ty_Args>
requires std::constructible_from<Ty, Ty_Args...>
auto make_tagged(lua_State* L, Ty_Args&&...args) -> Ty& {
    auto* p = static_cast<Ty*>(
        lua_newuserdatatagged(L, sizeof(Ty), static_cast<int>(Val))
    );
    std::construct_at(p, std::forward<Ty_Args>(args)...);
    push_metatable<Val>(L);
    lua_setmetatable(L, -2);
    return *p;
}
template <class Ty, Tag Val = Tag_For<Ty>, class ...Ty_Args>
auto make_tagged(lua_State* L, Ty_Args&&...args) -> decltype(auto) {
    return make_tagged<Val, Ty>(L, std::forward<Ty_Args>(args)...);
}
template <Tag Val, class Ty = Type_For<Val>>
auto new_tagged(lua_State* L) -> Ty& {
    return make_tagged<Val, Ty>(L);
}
template <class Ty, Tag Val = Tag_For<Ty>>
auto new_tagged(lua_State *L) -> decltype(auto) {
    return make_tagged<Val, Ty>(L);
}
template <Tag Val, class Ty = Type_For<Val>>
auto push_tagged(lua_State *L, Ty& ref) -> void {
    auto& p = *static_cast<Ty**>(
        lua_newuserdatatagged(L, sizeof(Ty**), detail::tag_reference_cast<Val, int>())
    );
    p = &ref;
    push_metatable<Val>(L);
    lua_setmetatable(L, -2);
}
template <class Ty, Tag Val = Tag_For<Ty>>
void push_tagged(lua_State* L, Ty& ref) {
    push_tagged<Val>(L, ref);
}
template <Tag Val, class Ty = Type_For<Val>>
constexpr auto is_tagged(lua_State* L, int idx) -> bool {
    const int tag = lua_userdatatag(L, idx);
    return tag == static_cast<int>(Val)
        or tag == detail::tag_reference_cast<Val, int>();
}

template <Tag Val, class Ty = Type_For<Val>>
constexpr auto to_tagged(lua_State* L, int idx) -> Ty& {
    const int tag = lua_userdatatag(L, idx);
    if (tag == static_cast<int>(Val)) {
        return *static_cast<Ty*>(lua_touserdatatagged(L, idx, static_cast<int>(Val)));
    } else if (tag == detail::tag_reference_cast<Val, int>()) {
        return **static_cast<Ty**>(lua_touserdatatagged(L, idx, detail::tag_reference_cast<Val, int>()));
    }
    lua::type_error(L, idx, compile_time::enum_info<Val>().name);
}
template <class Ty, Tag Val = Tag_For<Ty>>
constexpr auto to_tagged(lua_State* L, int idx) -> decltype(auto) {
    return to_tagged<Val>(L, idx);
}

template <Tag Val, class Ty = Type_For<Val>>
constexpr auto to_tagged_if(lua_State* L, int idx) -> Ty* {
    const int tag = lua_userdatatag(L, idx);
    if (tag == static_cast<int>(Val)) {
        return static_cast<Ty*>(lua_touserdatatagged(L, idx, static_cast<int>(Val)));
    } else if (tag == detail::tag_reference_cast<Val, int>()) {
        return *static_cast<Ty**>(lua_touserdatatagged(L, idx, detail::tag_reference_cast<Val, int>()));
    }
    return nullptr;
}
template <class Ty, Tag Val = Tag_For<Ty>>
constexpr auto to_tagged_if(lua_State* L, int idx) -> decltype(auto) {
    return to_tagged_if<Val>(L, idx);
}
