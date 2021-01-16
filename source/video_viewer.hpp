#pragma once

#include "controller.hpp"

struct frame_t {
    uint32_t frame;
    float time;

    uint32_t texture;
    uint32_t width, height;
};

void init_video_viewer();

frame_t *get_current_frame(void);
bool loaded_video();

DECLARE_CMD_PROC(cmd_load_file, const char *file);
