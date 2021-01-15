#include <imgui.h>

void tick_panel_video_viewer(ImGuiID master) {
    ImGui::SetNextWindowDockID(master, ImGuiCond_FirstUseEver);
    ImGui::Begin("Video Viewer");
    ImGui::End();
}
