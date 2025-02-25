#include "engine.hpp"
#include <imgui.h>
static bool console_open{true};
static std::stringstream console{};

auto getConsole() -> std::stringstream& {
    return console;
}

auto renderConsole() -> void {
    auto& io = ImGui::GetIO();
    if (ImGui::IsKeyPressed(ImGuiKey_F9)) {
        console_open = not console_open;
    }
    if (console_open) {
        ImGui::Begin("console", &console_open);
        ImGui::TextUnformatted(console.str().c_str());
        ImGui::End();
    }
}
