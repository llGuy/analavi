#include <imgui.h>
#include <stdint.h>

ImGuiID tick_panel_master() {
    ImGuiViewport *viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->Pos);
    ImGui::SetNextWindowSize(viewport->Size);

    uint32_t flags = 
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoBringToFrontOnFocus |
        ImGuiWindowFlags_MenuBar;

    ImGuiID dock_space_id = 0;

    ImGui::Begin("Master", NULL, flags);

    if (ImGui::BeginMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Open..", "Ctrl+O")) {
                // file_dialog.Open();
            }
            if (ImGui::MenuItem("Save", "Ctrl+S"))   { /* Do stuff */ }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Window")) {
            if (ImGui::MenuItem("Video Viewer", "Ctrl+Alt+v")) { /* Do stuff */ }
            if (ImGui::MenuItem("Controller", "Ctrl+Alt+c"))   { /* Do stuff */ }
            if (ImGui::MenuItem("Output", "Ctrl+Alt+o"))   { /* Do stuff */ }
            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
    }

    dock_space_id = ImGui::GetID("HUD_DockSpace");
    ImGui::DockSpace(dock_space_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None | ImGuiDockNodeFlags_PassthruCentralNode);

    ImGui::End();

    return dock_space_id;
}
