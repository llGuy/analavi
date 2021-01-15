#include <imgui.h>

void tick_panel_output(ImGuiID master) {
    ImGui::SetNextWindowDockID(master, ImGuiCond_FirstUseEver);
    ImGui::Begin("Output");
    ImGui::End();
}
