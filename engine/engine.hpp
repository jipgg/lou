#pragma once
#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_sdlrenderer3.h>
#include <imgui.h>
#include <memory>
#include <expected>
#include <filesystem>
#include <string>
#include <lualib.h>
#include <fstream>
#include "comp.hpp"
#include "util.hpp"
#include <chrono>
#include <sstream>
#include <print>
#include <functional>
#include "common.hpp"

enum class Tag {
    Any,
    Engine,
    Console,
    Rect,
    Point,
    Color,
    Texture,
    Window,
    Renderer,
};
template <Tag Tag> struct Mapped_Type;

#define Map_Type_To_Tag(TAG, TYPE)\
template <> struct Mapped_Type<Tag::TAG> {using Type = TYPE;}

#define Declare_Meta_Here \
struct Meta {\
    static auto index(lua_State* L) -> int;\
    static auto newindex(lua_State* L) -> int;\
    static auto namecall(lua_State* L) -> int;\
    static auto init(lua_State* L) -> void;\
}

struct Console {
    Declare_Meta_Here;
    auto render() -> void;
    enum class Severity {
        Comment, Warning, Error
    };
    struct Entry {
        Severity severity;
        std::string message;
    };
    std::vector<Entry> entries;
    bool open{true};
    bool is_dirty{false};
    template <Severity Severity>
    auto basic_print(const std::string& message) -> void {
        entries.emplace_back(Severity, stamp_debug_info() + std::string(message));
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
};
struct Logger {
    std::ofstream file{"lou.log", std::ios::app};
    template <class ...Ts>
    auto log(const std::format_string<Ts...>& fmt, Ts&&...args) -> void {
        file << stamp_debug_info();
        file << std::format(fmt, std::forward<Ts>(args)...) << "\n";
        file.flush();
    }
};
inline Logger logger{};


struct Engine {
    Declare_Meta_Here;
    using Clock_t = std::chrono::steady_clock;
    using Time_Point_t = std::chrono::time_point<Clock_t>;
    struct {
        Time_Point_t last_frame_start{Clock_t::now()};
        SDL_Event event;
        Lua_Ref console;
    } cache;
    struct {
        template <class Ty>
        using C_Owner_t = std::unique_ptr<Ty, void(*)(Ty*)>; 
        C_Owner_t<SDL_Renderer> renderer{nullptr, SDL_DestroyRenderer};
        C_Owner_t<SDL_Window> window{nullptr, SDL_DestroyWindow};
        C_Owner_t<TTF_TextEngine> text_engine{nullptr, TTF_DestroyRendererTextEngine};
        C_Owner_t<lua_State> luau{nullptr, lua_close};
    } raii;
    struct {
        Lua_Ref update{};
        Lua_Ref draw{}; 
        Lua_Ref shutdown{};
        Lua_Ref key_down{};
        Lua_Ref key_up{};
        Lua_Ref mouse_button_down{};
        Lua_Ref mouse_button_up{};
        Lua_Ref mouse_motion{};
        Lua_Ref mouse_scroll{};
    } callbacks;
    Console console;
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
    void draw();
    auto lua_state() -> lua_State* {return raii.luau.get();}
    auto renderer() -> SDL_Renderer* {return raii.renderer.get();}
    auto window() -> SDL_Window* {return raii.window.get();}
    auto text_engine() -> TTF_TextEngine* {return raii.text_engine.get();}
};

auto luaopen_rect(lua_State* L) -> void;
auto initRect(lua_State* L) -> void;
auto rectCtor(lua_State* L) -> int;

Map_Type_To_Tag(Point, SDL_Point);
Map_Type_To_Tag(Rect, SDL_Rect);
Map_Type_To_Tag(Color, SDL_Color);
Map_Type_To_Tag(Engine, Engine);
Map_Type_To_Tag(Console, Console);


template <Tag Tag, class Ty = Mapped_Type<Tag>::Type>
constexpr auto new_type(lua_State* L) -> Ty* {
    auto p = static_cast<Ty*>(
        lua_newuserdatatagged(L, sizeof(Ty), static_cast<int>(Tag))
    );
    new (p) Ty{};
    //std::memset(p, 0, sizeof(Ty));
    luaL_getmetatable(L, typeid(Ty).name());
    lua_setmetatable(L, -2);
    return p;
}
template <Tag Tag, class Ty = Mapped_Type<Tag>::Type>
constexpr auto push_type(lua_State* L, Ty* ptr) -> void {
    logger.log("pushed {}, ptr {}", typeid(Ty).name(), (uintptr_t)ptr);
    lua_pushlightuserdatatagged(L, ptr, static_cast<int>(Tag));
    luaL_getmetatable(L, typeid(Ty).name());
    lua_setmetatable(L, -2);
}
template <Tag Tag, class Ty = Mapped_Type<Tag>::Type>
constexpr auto is_type(lua_State* L, int idx) -> bool {
    if (lua_islightuserdata(L, idx)) {
        return lua_lightuserdatatag(L, idx) == int(Tag);
    } else {
        return lua_userdatatag(L, idx) == int(Tag);
    }
}

template <Tag Tag, class Ty = Mapped_Type<Tag>::Type>
constexpr auto to_type(lua_State* L, int idx) -> Ty* {
    if (lua_islightuserdata(L, idx)) {
        return static_cast<Ty*>(lua_tolightuserdatatagged(L, idx, int(Tag)));
    } else {
        return static_cast<Ty*>(lua_touserdatatagged(L, idx, int(Tag)));
    }
}

template <Tag Tag, class Ty = Mapped_Type<Tag>::Type>
constexpr auto check_type(lua_State* L, int idx) -> Ty* {
    if (not is_type<Tag>(L, idx)) return nullptr;
    return to_type<Tag>(L, idx);
}
#undef Declare_Meta_Here
#undef Map_Type_To_Tag
