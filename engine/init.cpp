#include <imgui_impl_sdl3.h>
#include <imgui_impl_sdlrenderer3.h>
#include "engine.hpp"
#include <imgui.h>
#include <filesystem>
#include <Luau/Compiler.h>
namespace fs = std::filesystem;

static void init_window_and_renderer(Game_State* game, Game_State::Init_Info e) {
    SDL_Renderer* renderer{};
    SDL_Window* window{};
    SDL_CreateWindowAndRenderer(
        e.title.c_str(),
        e.width,
        e.height,
        e.flags,
        &window,
        &renderer
    );
    assert(window and renderer);
    game->raii.window.reset(window);
    game->raii.renderer.reset(renderer);
}

void Game_State::init(Game_State::Init_Info e) {
    SDL_Init(SDL_INIT_VIDEO);
    TTF_Init();
    init_window_and_renderer(this, e);
    ImGui::CreateContext();
    ImGui_ImplSDL3_InitForSDLRenderer(window(), renderer());
    ImGui_ImplSDLRenderer3_Init(renderer());
    auto& io = ImGui::GetIO();
    auto fonti = io.Fonts->AddFontFromFileTTF("resources/main.ttf", 20);
    ImGui::GetStyle().ScaleAllSizes(2);
    raii.text_engine.reset(TTF_CreateRendererTextEngine(renderer()));
    auto font = TTF_OpenFont("resources/main.ttf", 60);
    init_luau();
    fs::path path{"game/init.luau"};
    std::ifstream file{path};
    if (!file.is_open()) {
        print("failed to open '{}'", path.string());
    }
    std::string line, contents;
    while (std::getline(file, line)) contents.append(line + '\n');
    auto bytecode = Luau::compile(contents, {});
    auto chunkname = std::format("=script:{}:", fs::relative(path).string());
    auto status = luau_load(lua_state(), chunkname.c_str(), bytecode.data(), bytecode.size(), 0);
    if (lua_pcall(lua_state(), 0, 0, 0) != LUA_OK) {
        print(lua_tostring(lua_state(), -1));
    }
}
