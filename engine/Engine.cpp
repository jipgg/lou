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

void Engine::init() {
    SDL_Renderer* renderer;
    SDL_Window* window;
    TTF_TextEngine* text;
    SDL_Init(SDL_INIT_VIDEO);
    TTF_Init();
    SDL_CreateWindowAndRenderer("luwuw", 1240, 780, SDL_WINDOW_RESIZABLE,
        &window,
        &renderer
    );
    ImGui::CreateContext();
    ImGui_ImplSDL3_InitForSDLRenderer(window, renderer);
    ImGui_ImplSDLRenderer3_Init(renderer);
    auto& io = ImGui::GetIO();
    auto fonti = io.Fonts->AddFontFromFileTTF("resources/main.ttf", 20);
    ImGui::GetStyle().ScaleAllSizes(2);
    text = TTF_CreateRendererTextEngine(renderer);
    auto font = TTF_OpenFont("resources/main.ttf", 60);
    writeConsole("initializing");
    data = {
        .renderer{renderer, SDL_DestroyRenderer},
        .window{window, SDL_DestroyWindow},
        .text{text, TTF_DestroyRendererTextEngine},
        .lua{luaL_newstate(), lua_close},
    };
    setupState(data.lua.get());
    fs::path path{"game/init.luau"};
    std::ifstream file{path};
    if (!file.is_open()) {
        print("failed to open '{}'", path.string());
    }
    std::string line, contents;
    while (std::getline(file, line)) contents.append(line + '\n');
    auto bytecode = Luau::compile(contents, {});
    auto chunkname = std::format("=script:{}:", fs::relative(path).string());
    auto status = luau_load(lstate(), chunkname.c_str(), bytecode.data(), bytecode.size(), 0);
    if (lua_pcall(lstate(), 0, 0, 0) != LUA_OK) {
        print(lua_tostring(lstate(), -1));
    }
}

constexpr SDL_Color white{0xff, 0xff ,0xff, 0xff};

static void consoleRender(Engine& e) {
    auto& io = ImGui::GetIO();
    if (ImGui::IsKeyPressed(ImGuiKey_F9)) {
        e.console.open = not e.console.open;
    }
    if (e.console.open) {
        ImGui::Begin("console", &e.console.open);
        ImGui::TextWrapped("%s", e.console.stream.str().c_str());
        //ImGui::TextUnformatted(console.str().c_str());
        ImGui::End();
    }
}

auto Engine::step() -> void {
    static SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_EVENT_QUIT:
                engine.running = false;
            break;
        }
        ImGui_ImplSDL3_ProcessEvent(&event);
    }
    auto render = data.renderer.get();
    ImGui_ImplSDL3_NewFrame();
    ImGui_ImplSDLRenderer3_NewFrame();
    ImGui::NewFrame();
    SDL_SetRenderDrawColor(render, 0x0, 0x0, 0x0, 0x0);
    SDL_RenderClear(render);
    for (auto& ref : callbacks.draw) {
        ref.push(lstate());
        if (lua_pcall(lstate(), 0, 0, 0) != LUA_OK) {
            print("{}", lua_tostring(lstate(), -1));
            lua_pop(lstate(), 1);
        }
    }
    consoleRender(*this);
    ImGui::Render();
    ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), render);
    SDL_RenderPresent(render);
}

