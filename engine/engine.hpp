#pragma once
#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <memory>
#include <expected>
#include <filesystem>
#include <string>
#include <lualib.h>
#include <fstream>
#include <sstream>
#include <functional>


using Path = std::filesystem::path;
enum class Tag {Rect, Point, Color, Texture, Window};
auto initState(lua_State* L, const Path& init_script = "game/init.luau") -> void;
inline std::ofstream fout{"log.txt"};
inline std::ofstream ferr{"errlog.txt"};
auto renderConsole() -> void;
auto getConsole() -> std::stringstream&;
auto luaopen_rect(lua_State* L) -> void;
auto initRect(lua_State* L) -> void;
auto rectCtor(lua_State* L) -> int;
auto luapush_graphics(lua_State* L) -> int;
struct Defer{
    const std::function<void()> fn;
    Defer(const std::function<void()>& fn): fn(fn) {}
    ~Defer() {}
};
struct EngineData {
    template <class Ty>
    using SDLPtr = std::unique_ptr<Ty, void(*)(Ty*)>; 
    SDLPtr<SDL_Renderer> renderer;
    SDLPtr<SDL_Window> window;
    SDLPtr<TTF_TextEngine> text;
    SDLPtr<TTF_Font> font;
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

