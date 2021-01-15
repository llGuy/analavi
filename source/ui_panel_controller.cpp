#include <imgui.h>

static char cmdbuf[1000] = {};

void tick_panel_controller(ImGuiID master) {
    ImGui::SetNextWindowDockID(master, ImGuiCond_FirstUseEver);
    ImGui::Begin("Controller");

    ImGui::InputText("Command", cmdbuf, 1000);

    ImGui::BeginChild("Controller_OutputWindow", {0, 0}, false, ImGuiWindowFlags_AlwaysVerticalScrollbar);
    ImGui::TextUnformatted("> ");
    // ImGui::GetWindowDrawList()->AddLine({0, 0}, {500, 500}, 0xFFFFFFFF);
    // ImGui::SetCursorPos({1500, 1500});
    // ImGui::TextUnformatted("hello");
    ImGui::EndChild();

    ImGui::End();
}
