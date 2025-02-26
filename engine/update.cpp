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
namespace fs = std::filesystem;

void game::update() {
    while (SDL_PollEvent(&cache.event)) {
        switch (cache.event.type) {
            case SDL_EVENT_QUIT:
                running = false;
            break;
        }
        ImGui_ImplSDL3_ProcessEvent(&cache.event);
    }
    auto L = lua();
    if (callbacks.update) {
        callbacks.update.push(L);
        if (lua_pcall(L, 0, 0, 0) != LUA_OK) {
            print("{}", lua_tostring(L, -1));
            lua_pop(L, 1);
        }
    }
}

