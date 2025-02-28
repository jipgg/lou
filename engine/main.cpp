#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <SDL3/SDL_gpu.h>
#include <expected>
#include <format>
#include "Lou.hpp"
#include <imgui_impl_sdl3.h>
#include <imgui_impl_sdlrenderer3.h>
#include <print>
#include <imgui.h>
#include <Luau/Require.h>
#include <Luau/Compiler.h>

constexpr SDL_Color white{0xff, 0xff ,0xff, 0xff};

auto main(int argc, char** argv) -> int {
    logger.log("");
    Lou_State state;
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
