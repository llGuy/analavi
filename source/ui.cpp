#include <imgui.h>
#include "ui.hpp"
#include <imfilebrowser.h>

#include "controller.hpp"
#include "video_viewer.hpp"

#include <TextEditor.h>

static ImFont *clean_font;
static ImFont *code_font;
static ImGui::FileBrowser file_dialog;

static TextEditor editor;

constexpr uint32_t MAX_CHARS_IN_CONTROLLER_OUTPUT = 10000;

static struct {
    char cmdbuf[1000] = {};

    uint32_t char_pointer = 0;
    char *cmd_window = NULL;
    const char *prompt = "> ";
} controller_panel;

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

    clean_font = io.Fonts->AddFontFromFileTTF("Roboto-Regular.ttf", 15.0f);
    code_font = io.Fonts->AddFontFromFileTTF("SourceCodePro.ttf", 15.0f);

    controller_panel.cmd_window = new char[MAX_CHARS_IN_CONTROLLER_OUTPUT];
    memset(controller_panel.cmd_window, 0, sizeof(char) * MAX_CHARS_IN_CONTROLLER_OUTPUT);

    memcpy(controller_panel.cmd_window, controller_panel.prompt, strlen(controller_panel.prompt));
    controller_panel.char_pointer += strlen(controller_panel.prompt);

    // auto lang = TextEditor::LanguageDefinition::
}

static ImGuiID s_tick_panel_master() {
    ImGui::PushFont(clean_font);

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
                file_dialog.Open();
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

    ImGui::PopFont();

    return dock_space_id;
}

void print_to_controller_output(const char *str) {
    uint32_t len = strlen(str);

    if (len + controller_panel.char_pointer < MAX_CHARS_IN_CONTROLLER_OUTPUT) {
        memcpy(controller_panel.cmd_window + controller_panel.char_pointer, str, strlen(str));
        controller_panel.char_pointer += len;
    }
    else {
        controller_panel.char_pointer = 0;
        memset(controller_panel.cmd_window, 0, sizeof(char) * MAX_CHARS_IN_CONTROLLER_OUTPUT);
    }
}

void begin_controller_cmd(const char *cmd) {
    print_to_controller_output(cmd);
    print_to_controller_output("\n");

    submit_cmdstr(cmd);
}

void finish_controller_cmd() {
    print_to_controller_output("\n> ");
}

static void s_tick_panel_controller(ImGuiID master) {
    ImGui::PushFont(code_font);

    ImGui::SetNextWindowDockID(master, ImGuiCond_FirstUseEver);
    ImGui::Begin("Controller");

    if (ImGui::InputText("Command", controller_panel.cmdbuf, 1000, ImGuiInputTextFlags_EnterReturnsTrue)) {
        begin_controller_cmd(controller_panel.cmdbuf);

        memset(controller_panel.cmdbuf, 0, sizeof(controller_panel.cmdbuf));

        finish_controller_cmd();
    }

    ImGui::BeginChild("Controller_OutputWindow", {0, 0}, false, ImGuiWindowFlags_AlwaysVerticalScrollbar | ImGuiWindowFlags_AlwaysHorizontalScrollbar);
    ImGui::TextUnformatted(controller_panel.cmd_window);
    ImGui::EndChild();

    ImGui::End();

    ImGui::PopFont();
}

static void s_tick_panel_file_browser() {
    ImGui::PushFont(clean_font);
    
    file_dialog.Display();

    if (file_dialog.HasSelected()) {
        //printf("%s\n", file_dialog.GetSelected().string().c_str());

        static char cmdbuf[80] = {};
        sprintf(cmdbuf, "load_file(\"%s\")", file_dialog.GetSelected().string().c_str());

        begin_controller_cmd(cmdbuf);
        finish_controller_cmd();

        file_dialog.ClearSelected();
    }

    ImGui::PopFont();
}
static void s_tick_panel_output(ImGuiID master) {
    ImGui::PushFont(clean_font);

    ImGui::SetNextWindowDockID(master, ImGuiCond_FirstUseEver);
    ImGui::Begin("Output");
    ImGui::End();

    ImGui::PopFont();
}

static void s_tick_panel_video_viewer(ImGuiID master) {
    ImGui::PushFont(clean_font);

    ImGui::SetNextWindowDockID(master, ImGuiCond_FirstUseEver);
    ImGui::Begin("Video Viewer");

    if (loaded_video()) {
        frame_t *frame = get_current_frame();
        ImGui::Image((void *)frame->texture, ImVec2(frame->width, frame->height));

        float progress;
        ImGui::SliderFloat("Time", &progress, 0.0f, 5.0f, "%.2fs");
    }
        
    ImGui::End();

    ImGui::PopFont();
}

void tick_gui() {
    ImGui::NewFrame();

    auto master_id = s_tick_panel_master();
    s_tick_panel_controller(master_id);
    s_tick_panel_file_browser();
    s_tick_panel_output(master_id);
    s_tick_panel_video_viewer(master_id);



    ImGui::Render();
}
