#pragma once

#include <vector>
#include <glm/glm.hpp>

#include "controller.hpp"

struct frame_t {
    uint32_t frame;
    float time;

    uint32_t texture;
    uint32_t width, height;

    float video_length;
};

void init_video_viewer();

frame_t *get_current_frame(void);
bool loaded_video();

struct record_point_t {
    glm::vec2 pos; // From 0->1
};

struct record_t{
    std::vector<record_point_t> points;
    bool is_recording = 0;
};

record_t &get_record();

void begin_playing_video();
void stop_playing_video();

void tick_video_player();

struct axes_t {
    int dist;

    glm::vec2 x_axis;
    glm::vec2 y_axis;

    glm::vec2 x1, x2;

    union {
        struct {
            uint8_t is_being_made: 1;
            uint8_t x1_is_set: 1;
            uint8_t x2_is_set: 1;
        };
        uint8_t flags;
    };
};

axes_t &get_axes();

DECLARE_CMD_PROC(int32_t, cmd_load_file, const char *file);
DECLARE_CMD_PROC(int32_t, cmd_goto_video_frame, int frame_id);
DECLARE_CMD_PROC(int32_t, cmd_goto_video_time, int time); // Milliseconds
DECLARE_CMD_PROC(int32_t, cmd_begin_record);
DECLARE_CMD_PROC(const char *, cmd_add_record_point, int x, int y, int max_x, int max_y);
DECLARE_CMD_PROC(int32_t, cmd_end_record);
DECLARE_CMD_PROC(int32_t, cmd_make_axes, int dist);

// Snake game
// 
