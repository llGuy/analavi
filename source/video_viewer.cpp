#include "ui.hpp"
#include "video_viewer.hpp"

#include <opencv2/core.hpp>     // Basic OpenCV structures (cv::Mat, Scalar)
#include <opencv2/videoio.hpp>

#include <GL/glew.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

static struct {
    cv::VideoCapture current_capture;
    cv::Size resolution;
    uint32_t frame_count;
    uint32_t fps;
    float length;

    frame_t current_frame;

    bool loaded;
} video;

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

    fill_texture(3);
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

        return 0;
    }
}

frame_t *get_current_frame() {
    return &video.current_frame;
}

bool loaded_video() {
    return video.loaded;
}
