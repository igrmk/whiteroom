#include <functional>
#include <iostream>

extern "C" {
#include <libavformat/avformat.h>
}

#include "../src/rtsp.h"

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cout << "usage: rtsp-connect rtsp://address:port/other_url_parts" << std::endl;
        return EXIT_FAILURE;
    }

    long frames = 0;
    long errors = 0;
    auto show_stats = [&]() { std::cout << "\33[2K\r" << "frames: " << frames << ", errors: " << errors << std::flush; };

    auto url = argv[1];
    auto on_frame = [&](AVFrame*) { ++frames; show_stats(); };
    auto on_error = [](const std::string& text) { std::cout << "ERROR " << text << std::endl; };
    auto on_parse_error = [&]() { ++errors; show_stats(); };
    auto on_stop = []() {};
    auto should_stop = []() -> auto { return false; };
    auto on_log = [](const std::string& text) { std::cout << "DEBUG " << text << std::endl; };
    auto get_width = []() -> auto { return 20; };

    rtsp(url, on_frame, on_error, on_parse_error, on_stop, should_stop, on_log, get_width, 20, 20);

    return 0;
}
