#pragma once
#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
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

class Ref {
    int ref_;
    lua_State* state_;
public:
    Ref(lua_State* L, int idx):
        ref_(lua_ref(L, idx)),
        state_(lua_mainthread(L)) {
    }
    Ref(const Ref& other) {
        state_ = other.state_;
        push(state_);
        ref_ = lua_ref(state_, -1);
        lua_pop(state_, 1);
    }
    Ref(Ref&& other) {
        state_ = other.state_;
        ref_ = other.ref_;
        other.state_ = nullptr;
    }
    Ref& operator=(const Ref& other) {
        *this = Ref(other);
        return *this;
    }
    Ref& operator=(Ref&& other) noexcept {
        *this = Ref(std::move(other));
        return *this;
    }
    ~Ref() {
        if (state_) {
            lua_unref(state_, ref_);
        }
    }
    void push(lua_State* L) {
        lua_getref(L, ref_);
    }
};

enum class Tag {
    Rect,
    Point,
    Color,
    Texture,
    Window
};

struct Engine {
    void init();
    void step();
    bool running{true};
    struct {
        template <class Ty>
        using Ptr = std::unique_ptr<Ty, void(*)(Ty*)>; 
        Ptr<SDL_Renderer> renderer{nullptr, SDL_DestroyRenderer};
        Ptr<SDL_Window> window{nullptr, SDL_DestroyWindow};
        Ptr<TTF_TextEngine> text{nullptr, TTF_DestroyRendererTextEngine};
        Ptr<lua_State> lua{nullptr, lua_close};
    } data;
    auto lstate() -> lua_State* {return data.lua.get();}
    struct {
        std::vector<Ref> update;
        std::vector<Ref> draw;
        std::vector<Ref> quit;
    } callbacks;
    struct {
        bool open{true};
        std::stringstream stream;
    } console;
    template <class ...TyArgs>
    void print(const std::format_string<TyArgs...>& fmt, TyArgs&&...args) {
        console.stream << std::format("[{}] ", std::chrono::system_clock::now());
        console.stream <<  std::format(fmt, std::forward<TyArgs>(args)...);
        console.stream << '\n';
    } 
    void print(std::string_view str) {
        console.stream << std::format("[{}] {}\n", std::chrono::system_clock::now(), str);
    }
};
inline Engine engine{};

using Path = std::filesystem::path;
auto setupState(lua_State* L) -> void;
inline std::ofstream fout{"log.txt"};
inline std::ofstream ferr{"errlog.txt"};
auto renderConsole() -> void;
auto getConsole() -> std::stringstream&;
auto writeConsole(std::string_view msg) -> void;
auto luaopen_rect(lua_State* L) -> void;
auto initRect(lua_State* L) -> void;
auto rectCtor(lua_State* L) -> int;
auto luapush_graphics(lua_State* L) -> int;
struct Defer{
    const std::function<void()> fn;
    Defer(const std::function<void()>& fn): fn(fn) {}
    ~Defer() {}
};

template <Tag Tg, class Ty>
constexpr auto newObject(lua_State* L) -> Ty& {
    auto p = static_cast<Ty*>(
        lua_newuserdatataggedwithmetatable(L, sizeof(Ty), static_cast<int>(Tg))
    );
    std::memset(p, 0, sizeof(Ty));
    return *p;
}
constexpr auto newRect(auto L) -> decltype(auto) {
    return newObject<Tag::Rect, SDL_Rect>(L);
}
constexpr auto newColor(auto L) -> decltype(auto) {
    return newObject<Tag::Color, SDL_Color>(L);
}
constexpr auto newPoint(auto L) -> decltype(auto) {
    return newObject<Tag::Point, SDL_Point>(L);
}

