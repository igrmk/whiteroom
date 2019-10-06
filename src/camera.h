#pragma once

#include <thread>
#include <memory>

#include <QObject>
#include <QString>
#include <QQmlEngine>

extern "C" {
#include <libavformat/avformat.h>
}

#include "rtsp.h"
#include "audio.h"
#include "stateenum.h"
#include "shared_amplitudes.h"
#include "log.h"
#include "white_math.h"

class Camera : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString url MEMBER url_)
    Q_PROPERTY(int cameraSensitivity MEMBER cameraSensitivity_)
    Q_PROPERTY(int verticalWeighting MEMBER verticalWeighting_)
    Q_PROPERTY(int audioHeight MEMBER audioHeight_)
    Q_PROPERTY(StateEnumModule::StateEnum state MEMBER state_ NOTIFY signalStateChanged)
    Q_PROPERTY(QString lastError MEMBER lastError_ NOTIFY signalLastErrorChanged)
    Q_PROPERTY(SharedAmplitudes * amplitudes MEMBER amplitudes_)
    Q_PROPERTY(int keysNumber MEMBER keysNumber_)
    Q_PROPERTY(int reverb MEMBER reverb_)

public:
    explicit Camera(QObject* parent = nullptr) : QObject(parent), log_(getlog()) {
        connect(this, &Camera::signalError, this, &Camera::onError);
        connect(this, &Camera::signalStop, this, &Camera::onStop);
    }

    void start() {
        shouldStop_ = false;
        setLastError("");
        std::fill(amplitudes_->get().begin(), amplitudes_->get().end(), 0.);
        std::thread(
            rtsp,
            url_.toStdString(),
            [this](AVFrame* frame) { this->onFrame(frame); },
            [this](const std::string& text) { emit this->signalError(QString::fromStdString(text)); },
            []() {},
            [this]() { emit this->signalStop(); },
            [this]() -> bool { return this->shouldStop_; },
            [this](const std::string& text) { this->log_->info(text); },
            [this]() -> int { return this->keysNumber_.load(); },
            audioHeight_,
            amplitudes_->maxKeysNumber()).detach();
        setState(StateEnumModule::StateEnum::STARTED);
    }

    void stop() {
        if (state_ == StateEnumModule::StateEnum::STARTED) {
            setState(StateEnumModule::StateEnum::STOPPING);
            shouldStop_ = true;
        }
    }

signals:
    void signalStateChanged();
    void signalLastErrorChanged();
    void signalStop();
    void signalError(QString text);

private:
    QString url_;
    std::atomic<int> cameraSensitivity_ = 0;
    std::atomic<int> verticalWeighting_ = 0;
    SharedAmplitudes* amplitudes_ = nullptr;
    int audioHeight_ = -1;
    std::atomic<bool> shouldStop_ = false;
    std::atomic<StateEnumModule::StateEnum> state_ = StateEnumModule::StateEnum::STOPPED;
    QString lastError_;
    logger log_;
    std::atomic<int> keysNumber_ = -1;
    std::atomic<int> reverb_ = -1;

    void onFrame(AVFrame* frame) {
        auto h = frame->height;
        auto k = verticalWeighting_ / 100.;
        auto z = (1. + k * h / 2.) * frame->width * 3.;
        auto r = reverb_ / 100.;
        auto& amplitudes = amplitudes_->get();
        for (int x = 0; x < frame->width; x++) {
            double column = 0.;
            for (int y = 0; y < h; y++) {
                double pix = 0.;
                for (int c = 0; c < 3; c++) {
                    auto cur = static_cast<int>((frame->data[0] + y * frame->linesize[0])[x * 3 + c]);
                    pix += norm(255 - cur, cameraSensitivity_ / 100.);
                }
                pix *= (1. + (h - y) * k);
                column += pix;
            }
            column /= z;
            auto pos = static_cast<size_t>(x);
            amplitudes[pos] = amplitudes[pos] * r + column * (1. - r);
        }
    }

    void onError(QString text) {
        log_->error(text.toStdString());
        setState(StateEnumModule::StateEnum::STOPPED);
        setLastError(text);
    }

    void onStop() {
        setState(StateEnumModule::StateEnum::STOPPED);
    }

    void setState(StateEnumModule::StateEnum state) {
        state_ = state;
        emit signalStateChanged();
    }

    void setLastError(QString error) {
        lastError_ = error;
        emit signalLastErrorChanged();
    }
};
