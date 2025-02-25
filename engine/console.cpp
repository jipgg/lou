#include "engine.hpp"
#include <imgui.h>
#include <chrono>
static bool console_open{true};
static std::stringstream console{};
namespace chr = std::chrono;

auto getConsole() -> std::stringstream& {
    return console;
}
auto writeConsole(std::string_view msg) -> void {
    using namespace std::chrono;
    console << std::format("[{}] {}\n", system_clock::now(), msg);
}

auto renderConsole() -> void {
    auto& io = ImGui::GetIO();
    if (ImGui::IsKeyPressed(ImGuiKey_F9)) {
        console_open = not console_open;
    }
    if (console_open) {
        ImGui::Begin("console", &console_open);
        ImGui::TextWrapped("%s", console.str().c_str());
        //ImGui::TextUnformatted(console.str().c_str());
        ImGui::End();
    }
}
