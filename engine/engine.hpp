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

class Lua_Ref {
    int ref_;
    lua_State* state_;
public:
    Lua_Ref(): ref_(-1), state_(nullptr) {}
    Lua_Ref(lua_State* L, int idx):
        ref_(lua_ref(L, idx)),
        state_(lua_mainthread(L)) {
    }
    Lua_Ref(const Lua_Ref& other) {
        state_ = other.state_;
        if (state_) {
            push(state_);
            ref_ = lua_ref(state_, -1);
            lua_pop(state_, 1);
        }
    }
    Lua_Ref(Lua_Ref&& other) {
        state_ = other.state_;
        ref_ = other.ref_;
        other.state_ = nullptr;
    }
    Lua_Ref& operator=(const Lua_Ref& other) {
        state_ = other.state_;
        if (state_) {
            push(state_);
            ref_ = lua_ref(state_, -1);
            lua_pop(state_, 1);
        }
        return *this;
    }
    Lua_Ref& operator=(Lua_Ref&& other) noexcept {
        state_ = other.state_;
        ref_ = other.ref_;
        other.state_ = nullptr;
        return *this;
    }
    ~Lua_Ref() {
        if (state_) {
            lua_unref(state_, ref_);
        }
    }
    operator bool() const {
        return state_;
    }
    void push(lua_State* L) {
        lua_getref(L, ref_);
    }
    void release() {
        state_ = nullptr;
    }
};
enum class Tag {
    Engine,
    Rect,
    Point,
    Color,
    Texture,
    Window,
    Renderer,
};

struct Engine_Meta {
    static auto index(lua_State* L) -> int;
    static auto newindex(lua_State* L) -> int;
    static auto namecall(lua_State* L) -> int;
    static void push_metatable(lua_State* L);
};

struct Window_Meta {
    static auto index(lua_State* L) -> int;
    static auto newindex(lua_State* L) -> int;
    static auto namecall(lua_State* L) -> int;
    static auto getmetatable(lua_State* L) -> void;
};


struct Engine {
    using Clock_t = std::chrono::steady_clock;
    using Time_Point_t = std::chrono::time_point<Clock_t>;
    struct {
        Time_Point_t last_frame_start{Clock_t::now()};
        SDL_Event event;
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
    struct {
        bool open{true};
        std::stringstream out;
        std::stringstream err;
    } console;
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
    template <class ...Ty_Args>
    void print(const std::format_string<Ty_Args...>& fmt, Ty_Args&&...args) {
        console.out << std::format("[{}] ", std::chrono::system_clock::now());
        console.out <<  std::format(fmt, std::forward<Ty_Args>(args)...);
        console.out << '\n';
    } 
    void print(std::string_view str) {
        console.out << std::format("[{}] {}\n", std::chrono::system_clock::now(), str);
    }
    auto lua_state() -> lua_State* {return raii.luau.get();}
    auto renderer() -> SDL_Renderer* {return raii.renderer.get();}
    auto window() -> SDL_Window* {return raii.window.get();}
    auto text_engine() -> TTF_TextEngine* {return raii.text_engine.get();}
};

auto luaopen_rect(lua_State* L) -> void;
auto initRect(lua_State* L) -> void;
auto rectCtor(lua_State* L) -> int;

template <Tag Tag> struct Mapped_Type;
#define Map_Type_To_Tag(TAG, TYPE)\
template <> struct Mapped_Type<Tag::TAG> {using Type = TYPE;}

Map_Type_To_Tag(Point, SDL_Point);
Map_Type_To_Tag(Rect, SDL_Rect);
Map_Type_To_Tag(Color, SDL_Color);
Map_Type_To_Tag(Engine, Engine);

#undef Map_Type_To_Tag

template <Tag Tag, class Ty = Mapped_Type<Tag>::Type>
constexpr auto new_type(lua_State* L) -> Ty* {
    auto p = static_cast<Ty*>(
        lua_newuserdatataggedwithmetatable(L, sizeof(Ty), static_cast<int>(Tag))
    );
    std::memset(p, 0, sizeof(Ty));
    return p;
}
template <Tag Tag, class Ty = Mapped_Type<Tag>::Type>
constexpr auto push_type(lua_State* L, Ty* ptr) -> void {
    lua_pushlightuserdatatagged(L, ptr, static_cast<int>(Tag));
    lua_getuserdatametatable(L, static_cast<int>(Tag));
    lua_setmetatable(L, -2);
}
template <Tag Tag, class Ty = Mapped_Type<Tag>::Type>
constexpr auto is_type(lua_State* L, int idx) -> Ty* {
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
    constexpr auto info = comp::enum_info<Tag, Tag>();
    if (not is_type<Tag>()) util::type_error(L, info.name);
}
