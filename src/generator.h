#pragma once

#include <memory>
#include <atomic>
#include <future>

#include "audio.h"
#include "camera.h"
#include "audio_config_data.h"
#include "log.h"
#include "shared_amplitudes.h"

class Generator : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int volume MEMBER volume_)
    Q_PROPERTY(int minNote MEMBER minNote_)
    Q_PROPERTY(int maxNote MEMBER maxNote_)
    Q_PROPERTY(SharedAmplitudes * amplitudes MEMBER sharedAmplitudes_)
    Q_PROPERTY(int keysNumber MEMBER keysNumber_)
    Q_PROPERTY(int maxKeysNumber READ maxKeysNumber WRITE setMaxKeysNumber)

public:
    explicit Generator(QObject* parent = nullptr) : QObject(parent), log_(getlog()) {}

    void start() {
        log_->info("starting generator...");
        buffer_.resize(static_cast<size_t>(audioConfig_.samples_ * kBytesPerValue * kNumChannels));
        std::thread([this]() { this->generate(); }).detach();
    }

    void stop() {
        log_->info("stopping generator...");
        process([](char*) -> bool { return true; });
        pos_ = 0;
        log_->info("generator stopped");
    }

    int maxKeysNumber() { return amplitudes_.size(); }
    void setMaxKeysNumber(int x) {
        amplitudes_.resize(x);
        std::fill(amplitudes_.begin(), amplitudes_.end(), 0.);
    }

    size_t readData(void* data, size_t len, bool stop);

    void setAudioConfig(AudioConfigData config) { audioConfig_ = config; }

private:
    std::vector<double> amplitudes_;
    SharedAmplitudes* sharedAmplitudes_ = nullptr;
    long pos_ = 0;
    std::promise<bool> nextPromise_;
    std::promise<bool> outputPromise_;
    std::vector<char> buffer_;
    std::atomic<int> volume_ = -1;
    std::atomic<int> minNote_ = -1;
    std::atomic<int> maxNote_ = -1;
    AudioConfigData audioConfig_;
    std::atomic<int> keysNumber_ = -1;
    logger log_;

    void generate();
    void process(std::function<bool(char*)> onBuffer);
};
