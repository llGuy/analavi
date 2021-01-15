#include <imgui.h>
#include <imfilebrowser.h>

static ImFont *font;
static ImGui::FileBrowser file_dialog;

void prepare_imgui() {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    ImGui::StyleColorsDark();

    ImGuiStyle &style =ImGui::GetStyle();
    style.WindowRounding = 0.0f;
    style.FrameRounding = 2.3f;
    style.Colors[ImGuiCol_WindowBg] = ImVec4(0.1f, 0.1f, 0.1f, 0.9f);
    style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.07f, 0.07f, 0.07f, 1.0f);
    ImVec4* colors = style.Colors;

    font = io.Fonts->AddFontFromFileTTF("Roboto-Regular.ttf", 15.0f);
}

/*
  Defined in respective files ui_panel_*panel_name*.cpp
 */
ImGuiID tick_panel_master();
void tick_panel_controller(ImGuiID master);
void tick_panel_file_browser(ImGui::FileBrowser &);
void tick_panel_output(ImGuiID master);
void tick_panel_video_viewer(ImGuiID master);

void tick_gui() {
    ImGui::NewFrame();
    ImGui::PushFont(font);

    auto master_id = tick_panel_master();
    tick_panel_controller(master_id);
    tick_panel_file_browser(file_dialog);
    tick_panel_output(master_id);
    tick_panel_video_viewer(master_id);

    ImGui::PopFont();

    ImGui::Render();
}
