#include <imgui.h>
#include <imfilebrowser.h>

void tick_panel_file_browser(ImGui::FileBrowser &file_dialog) {
    file_dialog.Display();

    if (file_dialog.HasSelected()) {
        printf("%s\n", file_dialog.GetSelected().string().c_str());
        file_dialog.ClearSelected();
    }
}
