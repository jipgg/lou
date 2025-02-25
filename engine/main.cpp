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

constexpr SDL_Color white{0xff, 0xff ,0xff, 0xff};
static auto init_data() ->std::expected<EngineData, std::string> {
    SDL_Renderer* renderer;
    SDL_Window* window;
    TTF_TextEngine* text;
    SDL_Init(SDL_INIT_VIDEO);
    TTF_Init();
    SDL_CreateWindowAndRenderer(
        "luwuw",
        1240,
        780,
        SDL_WINDOW_RESIZABLE,
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
    getConsole() << "initializing\n";
    if (not text) {
        return std::unexpected(SDL_GetError());
    }
    return EngineData{
        .renderer{renderer, SDL_DestroyRenderer},
        .window{window, SDL_DestroyWindow},
        .text{text, TTF_DestroyRendererTextEngine},
        .font{font, TTF_CloseFont},
    };
}

auto main(int argc, char** argv) -> int {
    auto data = init_data();
    auto r = data->renderer.get();
    auto surface = TTF_RenderText_Blended(data->font.get(), "Hello world", 0, white); 
    auto texture = SDL_CreateTextureFromSurface(data->renderer.get(), surface);

    bool quit{};
    SDL_Event event{};
    while (not quit) {
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_EVENT_QUIT:
                    quit = true;
                break;
            }
            ImGui_ImplSDL3_ProcessEvent(&event);
        }
        ImGui_ImplSDL3_NewFrame();
        ImGui_ImplSDLRenderer3_NewFrame();
        ImGui::NewFrame();
        renderConsole();
        ImGui::Render();
        SDL_SetRenderDrawColor(r, 0x0, 0x0, 0x0, 0x0);
        SDL_RenderClear(r);
        SDL_RenderTexture(r, texture, nullptr, nullptr);
        ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), r);
        SDL_RenderPresent(r);

        SDL_Delay(16);
    }
    return 0;
}
