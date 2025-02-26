#include "engine.hpp"

constexpr SDL_Color white{0xff, 0xff ,0xff, 0xff};

static void render_console(game* game) {
    auto& io = ImGui::GetIO();
    if (ImGui::IsKeyPressed(ImGuiKey_F9)) {
        game->console.open = not game->console.open;
    }
    if (game->console.open) {
        ImGui::Begin("console", &game->console.open);
        ImGui::TextWrapped("%s", game->console.out.str().c_str());
        //ImGui::TextUnformatted(console.str().c_str());
        ImGui::End();
    }
}
auto game::draw() -> void {
    auto render = renderer();
    ImGui_ImplSDL3_NewFrame();
    ImGui_ImplSDLRenderer3_NewFrame();
    ImGui::NewFrame();
    SDL_SetRenderDrawColor(render, 0x0, 0x0, 0x0, 0x0);
    SDL_RenderClear(render);
    auto L = lua();
    if (callbacks.draw) {
        callbacks.draw.push(L);
        if (lua_pcall(lua(), 0, 0, 0) != LUA_OK) {
            print("{}", lua_tostring(L, -1));
            lua_pop(L, 1);
        }

    }
    render_console(this);
    ImGui::Render();
    ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), render);
    SDL_RenderPresent(render);
}
