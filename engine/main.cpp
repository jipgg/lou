#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <SDL3/SDL_gpu.h>
#include <functional>
#include <expected>
#include "engine.hpp"
#include <format>
#include <iostream>
#include <print>
#include <fstream>

struct Defer{
    const std::function<void()> fn;
    Defer(const std::function<void()>& fn): fn(fn) {}
    ~Defer() {}
};
constexpr SDL_Color white{0xff, 0xff,0xff,0xff};


auto main(int argc, char** argv) -> int {
    std::ofstream logger{"log.txt"};
    Lua_state = luaL_newstate();
    luaL_openlibs(Lua_state);
    SDL_Init(SDL_INIT_VIDEO);
    TTF_Init();
    SDL_CreateWindowAndRenderer(
        "luwuw",
        1240,
        780,
        SDL_WINDOW_RESIZABLE,
        &Window,
        &Renderer
    );
    Text_engine = TTF_CreateRendererTextEngine(Renderer);
    if (not Text_engine) {
        std::println(logger, "{}", SDL_GetError());
        return -1;
    }
    Defer deferred([&] {
        TTF_DestroyRendererTextEngine(Text_engine);
        Text_engine = nullptr;
        SDL_DestroyRenderer(Renderer);
        Renderer = nullptr;
        SDL_DestroyWindow(Window);
        Window = nullptr;
        SDL_Quit();
    });
    auto font = TTF_OpenFont("resources/main.ttf", 60);
    if (not font) {
        std::println(logger, "{}", SDL_GetError());
        return -1;
    }
    auto surface = TTF_RenderText_Blended(font, "Hello world", 0, white); 
    auto texture = SDL_CreateTextureFromSurface(Renderer, surface);

    bool quit{};
    SDL_Event event{};
    while (not quit) {
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_EVENT_QUIT:
                    quit = true;
                break;
            }
        }
        SDL_SetRenderDrawColor(Renderer, 0x0, 0x0, 0x0, 0x0);
        SDL_RenderClear(Renderer);
        SDL_RenderTexture(Renderer, texture, nullptr, nullptr);
        SDL_RenderPresent(Renderer);

        SDL_Delay(16);
    }
    return 0;
}
