#pragma once

#include <functional>
#include <string>

extern "C" {
#include <libavformat/avformat.h>
}

void rtsp(
    std::string url,
    std::function<void(AVFrame*)> on_frame,
    std::function<void(const std::string&)> on_fatal_error,
    std::function<void()> on_parse_error,
    std::function<void()> on_stop,
    std::function<bool()> should_stop,
    std::function<void(const std::string&)> on_log,
    std::function<int()> final_width,
    int final_height,
    int max_width);
