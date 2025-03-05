#include "Lou.hpp"


auto Lou_Console::render() -> void {
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
        ImGui::End();
    }
}
void Lou_State::render() {
    auto r = renderer.get();
    ImGui_ImplSDL3_NewFrame();
    ImGui_ImplSDLRenderer3_NewFrame();
    ImGui::NewFrame();
    SDL_SetRenderDrawColor(r, 0x0, 0x0, 0x0, 0x0);
    SDL_RenderClear(r);
    on_render.call(lua_state(), console);
    console.render();
    ImGui::Render();
    ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), r);
    SDL_RenderPresent(r);
}
