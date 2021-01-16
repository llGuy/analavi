#include "ui.hpp"
#include "video_viewer.hpp"

int32_t cmd_load_file(const char *file) {
    char msg[50] = {};
    sprintf(msg, "Loading from %s\n", file);
    print_to_controller_output(msg);

    return 0;
}
