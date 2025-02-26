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
#include <chrono>
#include <sstream>
#include <print>
#include <functional>

class ref_t {
    int ref_;
    lua_State* state_;
public:
    ref_t(): ref_(-1), state_(nullptr) {}
    ref_t(lua_State* L, int idx):
        ref_(lua_ref(L, idx)),
        state_(lua_mainthread(L)) {
    }
    ref_t(const ref_t& other) {
        state_ = other.state_;
        if (state_) {
            push(state_);
            ref_ = lua_ref(state_, -1);
            lua_pop(state_, 1);
        }
    }
    ref_t(ref_t&& other) {
        state_ = other.state_;
        ref_ = other.ref_;
        other.state_ = nullptr;
    }
    ref_t& operator=(const ref_t& other) {
        *this = ref_t(other);
        return *this;
    }
    ref_t& operator=(ref_t&& other) noexcept {
        *this = ref_t(std::move(other));
        return *this;
    }
    ~ref_t() {
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
enum class light_tag {
    game
};
enum class tag {
    rect,
    point,
    color,
    texture,
    window
};

struct game {
    struct {
        using clock = std::chrono::steady_clock;
        using time_point = std::chrono::time_point<clock>;
        time_point last_frame_start{clock::now()};
        SDL_Event event;
    } cache;
    struct {
        template <class Ty>
        using ptr = std::unique_ptr<Ty, void(*)(Ty*)>; 
        ptr<SDL_Renderer> renderer{nullptr, SDL_DestroyRenderer};
        ptr<SDL_Window> window{nullptr, SDL_DestroyWindow};
        ptr<TTF_TextEngine> text_engine{nullptr, TTF_DestroyRendererTextEngine};
        ptr<lua_State> luau{nullptr, lua_close};
    } raii;
    struct {
        ref_t update{};
        ref_t draw{}; 
        ref_t shutdown{};
    } callbacks;
    struct {
        bool open{true};
        std::stringstream out;
        std::stringstream err;
    } console;
    bool running{true};
    struct init_t {
        std::string title{"engine"};
        int width{800};
        int height{600};
        SDL_WindowFlags flags{SDL_WINDOW_RESIZABLE};
    };
    void init(init_t data);
    void init_luau();
    void update();
    void draw();
    template <class ...TyArgs>
    void print(const std::format_string<TyArgs...>& fmt, TyArgs&&...args) {
        console.out << std::format("[{}] ", std::chrono::system_clock::now());
        console.out <<  std::format(fmt, std::forward<TyArgs>(args)...);
        console.out << '\n';
    } 
    void print(std::string_view str) {
        console.out << std::format("[{}] {}\n", std::chrono::system_clock::now(), str);
    }
    auto lua() -> lua_State* {return raii.luau.get();}
    auto renderer() -> SDL_Renderer* {return raii.renderer.get();}
    auto window() -> SDL_Window* {return raii.window.get();}
    auto text_engine() -> TTF_TextEngine* {return raii.text_engine.get();}
};

auto luaopen_rect(lua_State* L) -> void;
auto initRect(lua_State* L) -> void;
auto rectCtor(lua_State* L) -> int;

template <tag Tg, class Ty>
constexpr auto newObject(lua_State* L) -> Ty& {
    auto p = static_cast<Ty*>(
        lua_newuserdatataggedwithmetatable(L, sizeof(Ty), static_cast<int>(Tg))
    );
    std::memset(p, 0, sizeof(Ty));
    return *p;
}
constexpr auto newRect(auto L) -> decltype(auto) {
    return newObject<tag::rect, SDL_Rect>(L);
}
constexpr auto newColor(auto L) -> decltype(auto) {
    return newObject<tag::color, SDL_Color>(L);
}
constexpr auto newPoint(auto L) -> decltype(auto) {
    return newObject<tag::point, SDL_Point>(L);
}

