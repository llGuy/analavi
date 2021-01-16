#include <imgui.h>
#include "controller.hpp"
#include <opencv2/highgui.hpp>
#include <imfilebrowser.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <opencv2/core.hpp>     // Basic OpenCV structures (cv::Mat, Scalar)
#include <opencv2/videoio.hpp>

#include <stdlib.h>

#include "ui.hpp"
#include "context.hpp"

void load_avi(const char *path) {
    cv::VideoCapture capture;
    capture.open(path);

    if (!capture.isOpened()) {
        printf("Failed to open %s\n", path);
        exit(-1);
    }

    cv::Size size = cv::Size((int)capture.get(cv::CAP_PROP_FRAME_WIDTH), (int)capture.get(cv::CAP_PROP_FRAME_HEIGHT));
    printf("Resolution is: %d %d\n", size.width, size.height);

    int frame_count = capture.get(cv::CAP_PROP_FRAME_COUNT);
    printf("Frame count: %d\n", frame_count);

    // Next time we read, we will be at 1.2 seconds into the video
    // capture.set(cv::CAP_PROP_POS_MSEC, 1.2f);

    // Sets reader to 10th frame.
    capture.set(cv::CAP_PROP_POS_FRAMES, 10);

    cv::Mat frame;
    capture.read(frame);

    if (frame.empty()) {
        printf("Got to the end of the video file\n");
    }

    capture.release();
}

int main() {
    // load_avi("basket.avi");

    prepare_imgui();
    init_context();
    init_controller();

    { // Main loop
        while (is_running()) {
            begin_frame();
            tick_gui();
            end_frame();
        }
    }

    return 0;
}
