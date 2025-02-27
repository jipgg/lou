#include <SDL3/SDL.h>
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
#include "util.hpp"
namespace fs = std::filesystem;

static auto button_name(lua_State* L, uint8_t button_index) -> std::string {
    if (button_index == SDL_BUTTON_LEFT) return "left";
    if (button_index == SDL_BUTTON_MIDDLE) return "middle";
    if (button_index == SDL_BUTTON_RIGHT) return "right";
    return "unknown";
}

void Engine::update() {
    auto L = lua_state();
    auto& e = cache.event;
    while (SDL_PollEvent(&e)) {
        switch (cache.event.type) {
            case SDL_EVENT_QUIT:
                running = false;
            case SDL_EVENT_KEY_DOWN:
            case SDL_EVENT_KEY_UP:
                auto& cb = e.key.down ? callbacks.key_down : callbacks.key_up;
                if (cb) {
                    cb.push(L);
                    lua_pushstring(L, SDL_GetKeyName(e.key.key));
                    if (lua_pcall(L, 1, 0, 0) != LUA_OK) {
                        console.error(lua_tostring(L, -1));
                        lua_pop(L, 1);
                    }
                }
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

    if (callbacks.update) {
        auto L = lua_state();
        callbacks.update.push(L);
        util::push(L, delta_seconds);
        if (lua_pcall(L, 1, 0, 0) != LUA_OK) {
            console.error(lua_tostring(L, -1));
            lua_pop(L, 1);
        }
    }
}

