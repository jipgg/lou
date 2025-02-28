#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <SDL3/SDL_gpu.h>
#include <functional>
#include <expected>
#include "engine.hpp"
#include <format>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_sdlrenderer3.h>
#include <iostream>
#include <print>
#include <fstream>
#include <imgui.h>
#include <Luau/Require.h>
#include <Luau/Compiler.h>
namespace fs = std::filesystem;

constexpr SDL_Color white{0xff, 0xff ,0xff, 0xff};

auto main(int argc, char** argv) -> int {
    logger.log("");
    Engine state;
    state.init({
        .title{"test"},
        .width = 1920,
        .height = 1080,
        .flags = SDL_WINDOW_RESIZABLE,
    });
    while(state.running) {
        state.update();
        state.draw();
        SDL_Delay(16);
    }
    return 0;
}
