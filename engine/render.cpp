#include "engine.hpp"

constexpr SDL_Color white{0xff, 0xff ,0xff, 0xff};

auto Console::render() -> void {
    auto& io = ImGui::GetIO();
    if (ImGui::IsKeyPressed(ImGuiKey_F9)) {
        open = not open;
    }
    if (open) {
        ImGui::Begin("console", &open);
        for (const auto& entry : entries) {
            switch (entry.severity) {
                case Severity::Error:
                    ImGui::PushStyleColor(ImGuiCol_Text, 0xff0000ff);
                break;
                case Severity::Warning:
                    ImGui::PushStyleColor(ImGuiCol_Text, 0xffff00ff);
                break;
                case Severity::Comment:
                break;
            }
            ImGui::TextWrapped("%s", entry.message.c_str());

            switch (entry.severity) {
                case Severity::Warning:
                case Severity::Error:
                    ImGui::PopStyleColor();
                break;
                default:
                break;
            }
        }
        if (is_dirty) {
            ImGui::SetScrollHereY(1);
            is_dirty = false;
        }
        //ImGui::TextUnformatted(console.str().c_str());
        ImGui::End();
    }
}
auto Engine::draw() -> void {
    auto render = renderer();
    ImGui_ImplSDL3_NewFrame();
    ImGui_ImplSDLRenderer3_NewFrame();
    ImGui::NewFrame();
    SDL_SetRenderDrawColor(render, 0x0, 0x0, 0x0, 0x0);
    SDL_RenderClear(render);
    if (callbacks.draw) {
        auto L = lua_state();
        callbacks.draw.push(L);
        if (lua_pcall(L, 0, 0, 0) != LUA_OK) {
            console.error(lua_tostring(L, -1));
            lua_pop(L, 1);
        }

    }
    console.render();
    ImGui::Render();
    ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), render);
    SDL_RenderPresent(render);
}
