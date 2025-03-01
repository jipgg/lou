#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <SDL3/SDL_gpu.h>
#include <functional>
#include <expected>
#include "Lou.hpp"
#include <format>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_sdlrenderer3.h>
#include <iostream>
#include <print>
#include <fstream>
#include <imgui.h>
#include <Luau/Require.h>
#include <Luau/Compiler.h>
#include "common.hpp"
namespace fs = std::filesystem;

static auto button_name(lua_State* L, uint8_t button_index) -> std::string {
    if (button_index == SDL_BUTTON_LEFT) return "left";
    if (button_index == SDL_BUTTON_MIDDLE) return "middle";
    if (button_index == SDL_BUTTON_RIGHT) return "right";
    return "unknown";
}

void Lou_State::update() {
    auto L = lua_state();
    auto& e = cache.event;
    while (SDL_PollEvent(&e)) {
        switch (cache.event.type) {
            case SDL_EVENT_QUIT:
                running = false;
            case SDL_EVENT_KEY_DOWN:
            case SDL_EVENT_KEY_UP:
                if (e.key.down) {
                    keyboard.pressed.call(L, console, SDL_GetKeyName(e.key.key));
                } else {
                    keyboard.released.call(L, console, SDL_GetKeyName(e.key.key));
                }
            break;
            case SDL_EVENT_MOUSE_BUTTON_DOWN:
            case SDL_EVENT_MOUSE_BUTTON_UP:
                if (e.button.down) {
                    mouse.pressed.call(
                        L,
                        console,
                        button_name(L, e.button.button),
                        e.button.x,
                        e.button.y
                    );
                } else {
                    mouse.released.call(
                        L,
                        console,
                        button_name(L, e.button.button),
                        e.button.x,
                        e.button.y
                    );
                }
            break;
            case SDL_EVENT_MOUSE_MOTION:
                mouse.moved.call(L, console, e.motion.x, e.motion.y);
            break;
        }
        ImGui_ImplSDL3_ProcessEvent(&cache.event);
    }
    using namespace std::chrono;
    auto current_frame_start = Clock_t::now();
    auto delta_seconds = duration<double>(
        current_frame_start - cache.last_frame_start
    ).count();
    cache.last_frame_start = current_frame_start;
    on_update.call(L, console, delta_seconds);
}

