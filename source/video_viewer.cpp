#include "ui.hpp"
#include <glm/glm.hpp>
#include "video_viewer.hpp"

#include <opencv2/core.hpp>     // Basic OpenCV structures (cv::Mat, Scalar)
#include <opencv2/videoio.hpp>

#include <GL/glew.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <vector>

static struct {
    cv::VideoCapture current_capture;
    cv::Size resolution;
    uint32_t frame_count;
    uint32_t fps;
    float length;

    frame_t current_frame;

    bool loaded;
    bool is_playing;

    axes_t axes;
} video;

static record_t record;

// void load_avi(const char *path) {
//     cv::VideoCapture capture;
//     capture.open(path);

//     if (!capture.isOpened()) {
//         printf("Failed to open %s\n", path);
//         exit(-1);
//     }

//     cv::Size size = cv::Size((int)capture.get(cv::CAP_PROP_FRAME_WIDTH), (int)capture.get(cv::CAP_PROP_FRAME_HEIGHT));
//     printf("Resolution is: %d %d\n", size.width, size.height);

//     int frame_count = capture.get(cv::CAP_PROP_FRAME_COUNT);
//     printf("Frame count: %d\n", frame_count);

//     // Next time we read, we will be at 1.2 seconds into the video
//     // capture.set(cv::CAP_PROP_POS_MSEC, 1.2f);

//     // Sets reader to 10th frame.
//     capture.set(cv::CAP_PROP_POS_FRAMES, 10);

//     cv::Mat frame;
//     capture.read(frame);

//     if (frame.empty()) {
//         printf("Got to the end of the video file\n");
//     }

//     capture.release();
// }

void init_video_viewer() {
    video.loaded = 0;

    record.points.reserve(100);
}

void destroy_texture() {
    glDeleteTextures(1, &video.current_frame.texture);
}

void fill_texture(uint32_t frame_id) {
    video.current_capture.set(cv::CAP_PROP_POS_FRAMES, frame_id);

    cv::Mat frame;
    video.current_capture.read(frame);

    if (!frame.empty()) {
        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_RGBA,
            video.resolution.width,
            video.resolution.height,
            0,
            GL_BGR,
            GL_UNSIGNED_BYTE,
            frame.data);
    }
}

void make_texture_out_of_video(uint32_t width, uint32_t height) {
    glGenTextures(1, &video.current_frame.texture);
    glBindTexture(GL_TEXTURE_2D, video.current_frame.texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    fill_texture(0);
}

int32_t cmd_load_file(const char *file) {
    char msg[100] = {};

    if (video.current_capture.isOpened()) {
        // Need to make sure that nothing has been opened
        video.current_capture.release();
        destroy_texture();
    }

    video.current_capture.open(file);

    if (!video.current_capture.isOpened()) {
        sprintf(msg, "Failed to load from %s\n", file);
        print_to_controller_output(msg);
        video.loaded = 0;

        return 0;
    }
    else {
        sprintf(msg, "Loaded from %s\n", file);
        print_to_controller_output(msg);

        video.resolution = cv::Size(
            (int)video.current_capture.get(cv::CAP_PROP_FRAME_WIDTH),
            (int)video.current_capture.get(cv::CAP_PROP_FRAME_HEIGHT));

        video.current_frame.width = video.resolution.width;
        video.current_frame.height = video.resolution.height;
        video.current_frame.time = 0.0f;
        video.current_frame.frame = 0;

        video.frame_count = video.current_capture.get(cv::CAP_PROP_FRAME_COUNT);

        video.fps = video.current_capture.get(cv::CAP_PROP_FPS);
        video.length = (float)video.frame_count / (float)video.fps;

        sprintf(
            msg,
            "Resolution: %dx%d\nFrame count: %d\nLength: %.1f\nFPS: %d\n",
            video.resolution.width,
            video.resolution.height,
            video.frame_count,
            video.length,
            video.fps);

        print_to_controller_output(msg);

        make_texture_out_of_video(video.resolution.width, video.resolution.height);

        video.loaded = 1;

        video.current_frame.video_length = video.length;

        return 0;
    }
}

int32_t cmd_goto_video_frame(int frame_id) {
    if (frame_id < video.frame_count) {
        video.current_capture.set(cv::CAP_PROP_POS_FRAMES, frame_id);
        fill_texture((uint32_t)frame_id);

        video.current_frame.frame = frame_id;
        video.current_frame.time = (float)frame_id / (float)video.fps;
    }

    return 0;
}

int32_t cmd_goto_video_time(int time) {
    float t_seconds = (float)time / 1000.0f;

    if (t_seconds < video.length) {
        float frames = t_seconds * (float)video.fps;

        fill_texture((uint32_t)frames);

        video.current_frame.frame = frames;
        video.current_frame.time = (float)frames / (float)video.fps;
    }

    return 0;
}

frame_t *get_current_frame() {
    return &video.current_frame;
}

bool loaded_video() {
    return video.loaded;
}

record_t &get_record() {
    return record;
}

int32_t cmd_begin_record() {
    record.is_recording = 1;

    print_to_controller_output("Began recording...");

    return 0;
}

static glm::vec2 s_start_from_0(const glm::vec2 &v) {
    return glm::vec2(v.x, 1.0f - v.y);
}

const char *cmd_add_record_point(int x, int y, int max_x, int max_y) {
    record_point_t p = {};
    p.pos = p.axis_space_pos = glm::vec2(x, y) / glm::vec2(max_x, max_y);
    p.axis_space_pos = s_start_from_0(p.pos);
    // Calculate the coordinates of the point int he space we defined
    if (video.axes.x2_is_set && video.axes.x1_is_set) {
        float aspect = float(max_x) / float(max_y);

        // We first create a vector space with the pixel distance of max_x
        // Corresponding to 1 in all directions.
        glm::vec2 pos = p.axis_space_pos;
        pos.y /= aspect;

        glm::vec2 axis_origin = s_start_from_0(video.axes.x1);
        glm::vec2 axis_x = (s_start_from_0(video.axes.x2) - axis_origin) / float(video.axes.dist);
        glm::vec2 axis_y = glm::vec2(-axis_x.y, axis_x.x);

        axis_origin.y /= aspect;
        axis_x.y /= aspect;
        axis_y.y /= aspect;

        glm::mat3 t_inv = glm::mat3(1.0f);
        t_inv[2] = glm::vec3(-axis_origin.x, -axis_origin.y, 1.0f);

        glm::mat3 r_inv = glm::mat3(1.0f);
        glm::vec2 norm_x = glm::normalize(axis_x);
        glm::vec2 norm_y = glm::vec2(-norm_x.y, norm_x.x);

        r_inv[0] = glm::vec3(norm_x.x, norm_y.x, 0.0f);
        r_inv[1] = glm::vec3(norm_x.y, norm_y.y, 0.0f);
        r_inv[2] = glm::vec3(0.0f, 0.0f, 1.0f);

        glm::mat3 s_inv = glm::mat3(1.0f);
        float len = glm::length(axis_x);
        s_inv[0][0] = 1.0f / len;
        s_inv[1][1] = 1.0f / len;

        glm::vec2 translated = t_inv * glm::vec3(pos, 1.0f);
        glm::vec2 rotated = r_inv * glm::vec3(translated, 1.0f);
        glm::vec2 scaled = s_inv * glm::vec3(rotated, 1.0f);

        p.axis_space_pos = scaled;
    }

    char *info_str = new char[30];
    sprintf(info_str, "%.2f %.2f", p.axis_space_pos.x, p.axis_space_pos.y);

    record.points.push_back(p);

    return info_str;
}

int32_t cmd_end_record() {
    record.is_recording = 0;

    print_to_controller_output("Finished recording");

    return 0;
}

void begin_playing_video() {
    video.is_playing = 1;
}

void stop_playing_video() {
    video.is_playing = 0;
}

void tick_video_player() {
    
}

int32_t cmd_make_axes(int dist) {
    video.axes.flags = 0;
    video.axes.is_being_made = 1;
    video.axes.dist = dist;

    return 0;
}

axes_t &get_axes() {
    return video.axes;
}
