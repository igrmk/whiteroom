#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <vector>
#include <string>
#include <functional>
#include <chrono>
#include <ctime>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavformat/avio.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
#include <libavutil/time.h>
}

#include "rtsp.h"

const auto kEof = AVERROR_EOF; // NOLINT(hicpp-signed-bitwise)
const auto kEagain = AVERROR(EAGAIN);
const auto kTimeoutSeconds = 8;

int decode(AVCodecContext* context, AVFrame* frame, bool* got_frame, AVPacket* pkt);

static int interrupt(void* ctx) {
    auto deadline = reinterpret_cast<std::chrono::system_clock::time_point*>(ctx);
    auto now = std::chrono::system_clock::now();
    return (now > *deadline) ? 1 : 0;
}

static std::chrono::system_clock::time_point get_deadline() {
    return std::chrono::system_clock::now() + std::chrono::seconds(kTimeoutSeconds);
}

SwsContext* get_scaler(
    AVCodecContext* decoder_ctx,
    uint8_t* picture_buffer_yuv,
    uint8_t* picture_buffer_rgb,
    AVFrame* picture_yuv,
    AVFrame* picture_rgb,
    int final_width,
    int final_height) {

    SwsContext* img_convert_ctx = sws_getContext(
        decoder_ctx->width,
        decoder_ctx->height,
        decoder_ctx->pix_fmt,
        final_width,
        final_height,
        AV_PIX_FMT_RGB24,
        SWS_BICUBIC,
        nullptr,
        nullptr,
        nullptr);
    av_image_fill_arrays(
        picture_yuv->data,
        picture_yuv->linesize,
        picture_buffer_yuv,
        AV_PIX_FMT_YUV420P,
        decoder_ctx->width,
        decoder_ctx->height,
        32);
    av_image_fill_arrays(
        picture_rgb->data,
        picture_rgb->linesize,
        picture_buffer_rgb,
        AV_PIX_FMT_RGB24,
        final_width,
        final_height,
        32);
    return img_convert_ctx;
}

void rtsp(
    std::string url,
    std::function<void(AVFrame*)> on_frame,
    std::function<void(const std::string&)> on_fatal_error,
    std::function<void()> on_parse_error,
    std::function<void()> on_stop,
    std::function<bool()> should_stop,
    std::function<void(const std::string&)> on_log,
    std::function<int()> get_final_width,
    int final_height,
    int max_width) {

    av_log_set_level(AV_LOG_QUIET);

    avformat_network_init();

    AVFormatContext* input_ctx = avformat_alloc_context();
    input_ctx->interrupt_callback.callback = interrupt;
    auto deadline = get_deadline();
    input_ctx->interrupt_callback.opaque = &deadline;

    on_log("accessing camera stream...");
    if (int code = avformat_open_input(&input_ctx, url.c_str(), nullptr, nullptr) != 0) {
        on_fatal_error("cannot access camera stream");
        return;
    }

    deadline = get_deadline();
    on_log("reading video stream info...");
    if (avformat_find_stream_info(input_ctx, nullptr) < 0) {
        on_fatal_error("cannot read camera stream info");
        avformat_close_input(&input_ctx);
        return;
    }

    int video_stream_index = -1;
    for (unsigned int i = 0; i < input_ctx->nb_streams; i++) {
        if (input_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            video_stream_index = static_cast<int>(i);
            break;
        }
    }

    if (video_stream_index == -1) {
        on_fatal_error("cannot find video stream");
        avformat_close_input(&input_ctx);
        return;
    }
    on_log("video stream found");

    on_log("issuing play command for camera...");
    av_read_play(input_ctx);

    on_log("creating video decoder");
    AVCodec* decoder = avcodec_find_decoder(input_ctx->streams[video_stream_index]->codecpar->codec_id);
    if (!decoder) {
        on_fatal_error("cannot find video decoder");
        return;
    }

    auto codecpar = input_ctx->streams[video_stream_index]->codecpar;
    auto width = codecpar->width;
    auto height = codecpar->height;

    if (width <= 0 || height <= 0) {
        on_fatal_error("video stream has bad parameters");
        avformat_close_input(&input_ctx);
        return;
    }

    on_log("width " + std::to_string(codecpar->width));
    on_log("height " + std::to_string(codecpar->height));

    on_log("initializing decoder");
    AVCodecContext* decoder_ctx = avcodec_alloc_context3(decoder);
    avcodec_get_context_defaults3(decoder_ctx, decoder);
    avcodec_parameters_to_context(decoder_ctx, codecpar);
    if (avcodec_open2(decoder_ctx, decoder, nullptr) < 0) {
        on_fatal_error("cannot initialize decoder");
        avformat_close_input(&input_ctx);
        avcodec_free_context(&decoder_ctx);
        return;
    }

    auto size_yuv = static_cast<size_t>(av_image_get_buffer_size(AV_PIX_FMT_YUV420P, decoder_ctx->width, decoder_ctx->height, 32));
    auto size_rgb = static_cast<size_t>(av_image_get_buffer_size(AV_PIX_FMT_RGB24, max_width, final_height, 32));
    auto picture_buffer_yuv = static_cast<uint8_t*> (av_malloc(size_yuv));
    auto picture_buffer_rgb = static_cast<uint8_t*> (av_malloc(size_rgb));
    auto picture_yuv = av_frame_alloc();
    auto picture_rgb = av_frame_alloc();
    int prev_final_width = 0;
    SwsContext* img_convert_ctx = nullptr;

    auto capture = url.rfind("rtsp:", 0) == 0;

    on_log("reading video frames");
    auto packet = av_packet_alloc();
    while (!should_stop() && av_read_frame(input_ctx, packet) >= 0) {
        if (packet->stream_index == video_stream_index) {
            bool got_frame = false;
            deadline = get_deadline();
            if (decode(decoder_ctx, picture_yuv, &got_frame, packet) < 0) {
                on_parse_error();
            }
            if (got_frame) {
                auto final_width = get_final_width();
                if (final_width != prev_final_width) {
                    sws_freeContext(img_convert_ctx);
                    img_convert_ctx = get_scaler(decoder_ctx, picture_buffer_yuv, picture_buffer_rgb, picture_yuv, picture_rgb, final_width, final_height);
                    prev_final_width = final_width;
                }
                sws_scale(img_convert_ctx,
                          picture_yuv->data,
                          picture_yuv->linesize,
                          0,
                          decoder_ctx->height,
                          picture_rgb->data,
                          picture_rgb->linesize);
                picture_rgb->height = final_height;
                picture_rgb->width = final_width;
                on_frame(picture_rgb);
                if (!capture) {
                    av_usleep(1000000/30);
                }
            }
        }
        av_packet_unref(packet);
        packet = av_packet_alloc();
    }
    av_frame_free(&picture_yuv);
    av_frame_free(&picture_rgb);
    av_free(picture_buffer_yuv);
    av_free(picture_buffer_rgb);
    sws_freeContext(img_convert_ctx);
    avcodec_free_context(&decoder_ctx);
    avformat_close_input(&input_ctx);

    on_stop();
}

int decode(AVCodecContext* context, AVFrame* frame, bool* got_frame, AVPacket* pkt) {
    auto send_res = avcodec_send_packet(context, pkt);
    if (send_res < 0) {
        *got_frame = false;
        return send_res == kEof ? 0 : send_res;
    }

    auto receive_res = avcodec_receive_frame(context, frame);
    if (receive_res == kEagain || receive_res == kEof) {
        *got_frame = false;
        return 0;
    }
    *got_frame = receive_res == 0;
    return receive_res;
}
