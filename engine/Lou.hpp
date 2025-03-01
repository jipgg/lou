#pragma once
#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
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
    lua::Callback_List<const std::string&, int, int> pressed;
    lua::Callback_List<const std::string&, int, int> released;
    lua::Callback_List<int, int> moved;
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

struct Lou_State {
    struct {
        C_Owner_t<lua_State> luau{nullptr, lua_close};
    } owning;
    Lou_Window window;
    Lou_Renderer renderer;
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
    };
    void init(Init_Info data);
    void init_luau();
    void update();
    void render();
    constexpr auto lua_state() -> lua_State* {return owning.luau.get();}
    static auto push_metatable(lua_State* L) -> void;
};

enum class Namecall_Atom: int16_t {
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
    size_changed,
    resized,
    resize,
    size,
    clear,
    draw_rect,
    draw_point,
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
    X(Lou_Window)\
    X(Lou_Renderer)\
    X(Lou_Keyboard)\
    X(Lou_Mouse)\
    X(COMPILE_TIME_ENUM_SENTINEL)

enum class Tag {
    #define Generate_Tag_Enum(item) item,
    Tag_Enum_Item_Generator(Generate_Tag_Enum)
    #undef Generate_Tag_Enum
};

enum class Tag_Reference {
    #define Generate_Tag_Reference_Enum(item) item = LUA_UTAG_LIMIT - static_cast<int>(Tag::item),
    Tag_Enum_Item_Generator(Generate_Tag_Reference_Enum)
    #undef Generate_Tag_Reference_Enum
};
#undef Tag_Enum_Item_Generator

template <Tag Tag> struct Mapped_Type;

#define Map_Type_To_Tag(TAG, TYPE)\
template <> struct Mapped_Type<Tag::TAG> {using Type = TYPE;}

Map_Type_To_Tag(Point, SDL_FPoint);
Map_Type_To_Tag(Rect, SDL_FRect);
Map_Type_To_Tag(Color, SDL_Color);
Map_Type_To_Tag(Lou_State, Lou_State);
Map_Type_To_Tag(Lou_Console, Lou_Console);
Map_Type_To_Tag(Lou_Keyboard, Lou_Keyboard);
Map_Type_To_Tag(Lou_Mouse, Lou_Mouse);
Map_Type_To_Tag(Lou_Window, Lou_Window);
Map_Type_To_Tag(Lou_Renderer, Lou_Renderer);

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
template <Tag Val>
auto get_metatable_name() -> const char* {
    constexpr auto v = compile_time::enum_info<Val>().name;
    static const std::string name{v};
    return name.c_str();
}
template <Tag_Reference Val>
auto get_metatable_name() -> decltype(auto) {
    return get_metatable_name<tag_cast<Val>()>();
}
template <Tag Val>
auto push_metatable(lua_State* L) -> void {
    luaL_getmetatable(L, get_metatable_name<Val>());
}
template <Tag_Reference Val>
auto push_metatable(lua_State *L) -> decltype(auto) {
    return push_metatable<tag_cast<Val>()>();
}
template <Tag Val, class Ty = Mapped_Type<Val>::Type>
auto new_object(lua_State* L) -> Ty& {
    auto* p = static_cast<Ty*>(
        lua_newuserdatatagged(L, sizeof(Ty), static_cast<int>(Val))
    );
    new (p) Ty{};
    push_metatable<Val>(L);
    lua_setmetatable(L, -2);
    return *p;
}
template <Tag Val, class Ty = Mapped_Type<Val>::Type>
auto push_reference(lua_State* L, Ty* ref) -> void {
    auto& p = *static_cast<Ty**>(
        lua_newuserdatatagged(L, sizeof(Ty**), tag_reference_cast<Val, int>())
    );
    p = ref;
    push_metatable<Val>(L);
    lua_setmetatable(L, -2);
}
template <Tag Val, class Ty = Mapped_Type<Val>::Type>
auto push_reference(lua_State *L, Ty& ref) -> void {
    push_reference<Val>(L, &ref);
}
template <Tag Val, class Ty = Mapped_Type<Val>::Type>
constexpr auto is_tagged(lua_State* L, int idx) -> bool {
    const int tag = lua_userdatatag(L, idx);
    return tag == static_cast<int>(Val)
        or tag == tag_reference_cast<Val, int>();
}

template <Tag Val, class Ty = Mapped_Type<Val>::Type>
constexpr auto to_object(lua_State* L, int idx) -> Ty& {
    const int tag = lua_userdatatag(L, idx);
    if (tag == static_cast<int>(Val)) {
        return *static_cast<Ty*>(lua_touserdatatagged(L, idx, static_cast<int>(Val)));
    } else if (tag == tag_reference_cast<Val, int>()) {
        return **static_cast<Ty**>(lua_touserdatatagged(L, idx, tag_reference_cast<Val, int>()));
    }
    lua::type_error(L, idx, compile_time::enum_info<Val>().name);
}

