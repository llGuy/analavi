#include <imgui.h>
#include <SDL.h>
#include <malloc.h>
#include "ui.hpp"
#include <imfilebrowser.h>

#include "controller.hpp"
#include "video_viewer.hpp"

#include <TextEditor.h>

static ImFont *clean_font;
static ImFont *code_font;
static ImFont *editor_font;
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
    style.FrameRounding = 0.0f;
    style.TabRounding = 0.0f;
    style.Colors[ImGuiCol_WindowBg] = ImVec4(0.1f, 0.1f, 0.1f, 0.9f);
    style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.07f, 0.07f, 0.07f, 1.0f);
    ImVec4* colors = style.Colors;

    clean_font = io.Fonts->AddFontFromFileTTF("Roboto-Regular.ttf", 15.0f);
    code_font = io.Fonts->AddFontFromFileTTF("SourceCodePro.ttf", 15.0f);
    editor_font = io.Fonts->AddFontFromFileTTF("SourceCodePro.ttf", 17.0f);

    controller_panel.cmd_window = new char[MAX_CHARS_IN_CONTROLLER_OUTPUT];
    memset(controller_panel.cmd_window, 0, sizeof(char) * MAX_CHARS_IN_CONTROLLER_OUTPUT);

    memcpy(controller_panel.cmd_window, controller_panel.prompt, strlen(controller_panel.prompt));
    controller_panel.char_pointer += strlen(controller_panel.prompt);


    editor.SetText("");
    editor.SetShowWhitespaces(0);
    editor.SetColorizerEnable(0);
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

void begin_controller_cmd(const char *cmd, bool print = 1) {
    if (print) {
        print_to_controller_output(cmd);
        print_to_controller_output("\n");
    }

    submit_cmdstr(cmd);
}

void finish_controller_cmd(bool print = 1) {
    if (print)
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

    auto &record = get_record();

    editor.SetReadOnly(record.is_recording);

    if (ImGui::BeginMenuBar()) {
        if (ImGui::BeginMenu("Edit")) {
            if (ImGui::MenuItem("Copy", "Ctrl+C", (bool *)NULL, editor.HasSelection())) {
                editor.Copy();
            }
            if (ImGui::MenuItem("Cut", "Ctrl+X", (bool *)NULL, editor.HasSelection())) {
                editor.Cut();
            }
            if (ImGui::MenuItem("Delete", "Del", (bool *)NULL, editor.HasSelection())) {
                editor.Delete();
            }
            if (ImGui::MenuItem("Paste", "Ctrl+V", (bool *)NULL, editor.HasSelection())) {
                editor.Paste();
            }
        }
        if (ImGui::BeginMenu("View")) {
            if (ImGui::MenuItem("Dark palette"))
                editor.SetPalette(TextEditor::GetDarkPalette());
            if (ImGui::MenuItem("Light palette"))
                editor.SetPalette(TextEditor::GetLightPalette());
            if (ImGui::MenuItem("Retro blue palette"))
                editor.SetPalette(TextEditor::GetRetroBluePalette());
            ImGui::EndMenu();
        }

        ImGui::EndMenuBar();
    }

    ImGui::PushFont(editor_font);

    // printf("%d\n", editor.GetCursorPosition().mLine);

    editor.Render("TextEditor");

    ImGui::PopFont();
    
    ImGui::End();

    ImGui::PopFont();
}

static void s_tick_panel_video_viewer(ImGuiID master) {
    ImGui::PushFont(clean_font);

    ImGui::SetNextWindowDockID(master, ImGuiCond_FirstUseEver);
    ImGui::Begin("Video Viewer");

    ImVec2 window_pos = ImGui::GetWindowPos();
    ImVec2 window_size = ImGui::GetWindowSize();

    if (loaded_video()) {
        frame_t *frame = get_current_frame();
        float aspect = (float)(frame->width) / (float)(frame->height);
        float window_aspect = window_size.x / window_size.y;

        float img_width = 0.0f;
        float img_height = 0.0f;

        if (window_aspect > aspect) {
            img_height = window_size.y;
            img_width = img_height * aspect;
        }
        else {
            img_width = window_size.x;
            img_height = img_width / aspect;
        }

        ImGui::Image((void *)frame->texture, ImVec2(img_width, img_height));

        ImVec2 min = ImGui::GetItemRectMin();
        ImVec2 max = ImGui::GetItemRectMax();
        ImVec2 size = ImGui::GetItemRectSize();

        auto *dl = ImGui::GetWindowDrawList();

        auto &record = get_record();

        if (record.is_recording) {
            if (ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {
                ImVec2 mouse_pos = ImGui::GetMousePos();
                if (mouse_pos.x > min.x && mouse_pos.x < max.x &&
                    mouse_pos.y > min.y && mouse_pos.y < max.y) {
                    int x = mouse_pos.x - min.x;
                    int y = mouse_pos.y - min.y;

                    static char cmdbuf[80] = {};
                    sprintf(cmdbuf, "add_record_point(%d, %d, %d, %d)", x, y, (int)size.x, (int)size.y);

                    begin_controller_cmd(cmdbuf);
                    finish_controller_cmd();

                    const char *point_str = get_return_value<const char *>();

                    // Push this to the text editor
                    auto lines = editor.GetTextLines();
                    auto cursor_pos = editor.GetCursorPosition();

                    lines[cursor_pos.mLine].append(point_str);

                    if (cursor_pos.mLine == lines.size() - 1) {
                        lines.push_back({});
                    }

                    editor.SetCursorPosition(TextEditor::Coordinates(cursor_pos.mLine + 1, 0));
                    
                    editor.SetTextLines(lines);
                }
            }
        }

        auto &axes = get_axes();

        if (axes.is_being_made) {
            if (!axes.x1_is_set) {
                if (ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {
                    ImVec2 mouse_pos = ImGui::GetMousePos();
                    if (mouse_pos.x > min.x && mouse_pos.x < max.x &&
                        mouse_pos.y > min.y && mouse_pos.y < max.y) {
                        int x = mouse_pos.x - min.x;
                        int y = mouse_pos.y - min.y;

                        axes.x1 = glm::vec2(x / size.x, y / size.y);
                        axes.x1_is_set = 1;
                    }
                }
            }
            else if (!axes.x2_is_set) {
                ImVec2 mouse_pos = ImGui::GetMousePos();
                ImVec2 clamped = mouse_pos;
                clamped.x = glm::clamp(mouse_pos.x, min.x, max.x);
                clamped.y = glm::clamp(mouse_pos.y, min.y, max.y);

                ImVec2 d = ImVec2();
                dl->AddLine(ImVec2(axes.x1.x * size.x + min.x, axes.x1.y * size.y + min.y), clamped, ImGui::ColorConvertFloat4ToU32(ImVec4(1.0f, 1.0f, 1.0f, 1.0f)));

                if (ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {
                    if (mouse_pos.x > min.x && mouse_pos.x < max.x &&
                        mouse_pos.y > min.y && mouse_pos.y < max.y) {
                        int x = mouse_pos.x - min.x;
                        int y = mouse_pos.y - min.y;

                        axes.x2 = glm::vec2(x / size.x, y / size.y);
                        axes.x2_is_set = 1;
                    }
                }
            }
            else {
                ImVec2 d = ImVec2();
                dl->AddLine(
                    ImVec2(axes.x1.x * size.x + min.x, axes.x1.y * size.y + min.y),
                    ImVec2(axes.x2.x * size.x + min.x, axes.x2.y * size.y + min.y),
                    ImGui::ColorConvertFloat4ToU32(ImVec4(1.0f, 1.0f, 1.0f, 1.0f)));

                axes.is_being_made = 0;
            }
        }
        else if (axes.x1_is_set &&  axes.x2_is_set) {
            glm::vec2 size_glm = glm::vec2(size.x, size.y);

            glm::vec2 x1_pixel = axes.x1 * size_glm;
            glm::vec2 x2_pixel = (axes.x2 * size_glm - x1_pixel) / (float)axes.dist + x1_pixel;

            ImVec2 d = ImVec2();
            dl->AddLine(
                ImVec2(x1_pixel.x + min.x, x1_pixel.y + min.y),
                ImVec2(x2_pixel.x + min.x, x2_pixel.y + min.y),
                ImGui::ColorConvertFloat4ToU32(ImVec4(1.0f, 0.0f, 0.0f, 1.0f)), 3.0f);

            glm::vec2 perp_vec = axes.x2 * size_glm - x1_pixel;
            perp_vec = glm::vec2(perp_vec.y, -perp_vec.x);
            x2_pixel = perp_vec / (float)axes.dist + x1_pixel;

            d = ImVec2();
            dl->AddLine(
                ImVec2(x1_pixel.x + min.x, x1_pixel.y + min.y),
                ImVec2(x2_pixel.x + min.x, x2_pixel.y + min.y),
                ImGui::ColorConvertFloat4ToU32(ImVec4(0.0f, 1.0f, 0.0f, 1.0f)), 3.0f);
        }

        if (ImGui::IsKeyDown(SDL_SCANCODE_LCTRL) && ImGui::IsKeyReleased(SDL_SCANCODE_W)) {
            cmd_goto_video_frame(frame->frame + 1);
        }

        if (ImGui::IsKeyDown(SDL_SCANCODE_LCTRL) && ImGui::IsKeyDown(SDL_SCANCODE_LSHIFT)) {
            auto &axes = get_axes();
            if (ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
                // Start moving the axes
                auto mouse_pos = ImGui::GetMousePos();
                axes.current_mouse_pos = glm::vec2(mouse_pos.x, mouse_pos.y);
            }
            else if (ImGui::IsMouseDown(ImGuiMouseButton_Right)) {
                auto mouse_pos = ImGui::GetMousePos();
                glm::vec2 size_glm = glm::vec2(size.x, size.y);
                glm::vec2 current_pos = glm::vec2(mouse_pos.x, mouse_pos.y);
                glm::vec2 diff = current_pos - axes.current_mouse_pos;

                glm::vec2 x2_x1_diff = axes.x2 - axes.x1;

                glm::vec2 new_x1 = axes.x1 * size_glm + diff;
                axes.x1 = new_x1 / size_glm;
                axes.x2 = axes.x1 + x2_x1_diff;

                axes.current_mouse_pos = current_pos;
            }

            ImGuiIO &io = ImGui::GetIO();
            // ImGui::Text("%f", io.MouseWheel);
            if (io.MouseWheel != 0.0f) {
                float diff = io.MouseWheel;
                glm::vec2 size_glm = glm::vec2(size.x, size.y);

                diff = io.DeltaTime * 100.0f * diff;
                float angle = glm::radians(diff);

                glm::mat2 rot = glm::mat2(
                    glm::cos(angle),
                    glm::sin(angle),
                    -glm::sin(angle),
                    glm::cos(angle));

                glm::vec2 x2_x1_diff = (axes.x2 - axes.x1) * size_glm;
                x2_x1_diff = rot * x2_x1_diff;

                axes.x2 = (axes.x1 * size_glm + x2_x1_diff) / size_glm;
            }
        }

        for (auto p : record.points) {
            ImVec2 relative_pos = size;
            relative_pos.x *= p.pos.x;
            relative_pos.y *= p.pos.y;
            ImVec2 position = min;
            position.x += relative_pos.x;
            position.y += relative_pos.y;
            dl->AddCircleFilled(position, 4.0f, ImGui::ColorConvertFloat4ToU32(ImVec4(1.0f, 1.0f, 1.0f, 1.0f)));
        }

        static float progress = frame->time;
        if (ImGui::SliderFloat("Time", &progress, 0.0f, frame->video_length, "%.2fs")) {
            int time_milli = (int)(progress * 1000.0f);

            static char cmdbuf[80] = {};
            sprintf(cmdbuf, "goto_video_time(%d)", time_milli);

            begin_controller_cmd(cmdbuf, 0);
            finish_controller_cmd(0);
        }
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
