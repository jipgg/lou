#include <imgui_impl_sdl3.h>
#include <imgui_impl_sdlrenderer3.h>
#include <SDL3/SDL_main.h>
#include <SDL3_image/SDL_image.h>
#include "Lou.hpp"
#include <imgui.h>
#include <filesystem>
#include <Luau/Compiler.h>
#include <ranges>
namespace rngs = std::ranges;
namespace fs = std::filesystem;
using Init_Info = Lou_State::Init_Info;

static void init_window_and_renderer(Lou_State* state, const Init_Info& info) {
    SDL_Renderer* renderer{};
    SDL_Window* window{};
    SDL_CreateWindowAndRenderer(
        info.title.c_str(),
        info.width,
        info.height,
        info.flags,
        &window,
        &renderer
    );
    assert(window and renderer);
    state->window.owning.window.reset(window);
    state->renderer.owning.renderer.reset(renderer);
}

static auto run_script_entry_point(lua_State* L, const fs::path& script_entry_point) -> std::expected<void, std::string> {
    fs::path path{script_entry_point};
    std::ifstream file{path};
    if (!file.is_open()) {
        return std::unexpected(std::format("failed to open '{}'", path.string()));
    }
    std::string line, contents;
    while (std::getline(file, line)) contents.append(line + '\n');
    auto bytecode = Luau::compile(contents, {});
    auto chunkname = std::format("@{}:", fs::absolute(path).string());
    rngs::replace(chunkname, '\\', '/');
    auto status = luau_load(L, chunkname.c_str(), bytecode.data(), bytecode.size(), 0);
    if (lua_pcall(L, 0, 0, 0) != LUA_OK) {
        std::string runtime_error{lua_tostring(L, -1)};
        lua_pop(L, 1);
        return std::unexpected(std::move(runtime_error));
    }
    return {};
}

void Lou_State::init(Init_Info info) {
    SDL_Init(SDL_INIT_VIDEO);
    TTF_Init();
    init_window_and_renderer(this, info);
    ImGui::CreateContext();
    ImGui_ImplSDL3_InitForSDLRenderer(window.get(), renderer.get());
    ImGui_ImplSDLRenderer3_Init(renderer.get());
    auto& io = ImGui::GetIO();
    //auto fonti = io.Fonts->AddFontFromFileTTF("resources/main.ttf", 20);
    io.FontGlobalScale = 2;
    //ImGui::GetStyle().ScaleAllSizes(5);
    renderer.owning.text_engine.reset(TTF_CreateRendererTextEngine(renderer.get()));
    texture.renderer = renderer.get();
    //auto font = TTF_OpenFont("resources/main.ttf", 60);
    init_luau();
    auto ok = run_script_entry_point(lua_state(), info.script_entry_point);
    if (not ok) console.error(ok.error());
}


auto main(int argc, char** argv) -> int {
    Lou_State state;
    state.init({
        .title{"test"},
        .width = 1920,
        .height = 1080,
        .flags = SDL_WINDOW_RESIZABLE,
        .script_entry_point = argc > 1 ? argv[1] : "init.luau",
    });
    while(state.running) {
        state.update();
        state.render();
        SDL_Delay(16);
    }
    return 0;
}
