#include <qmath.h>
#include <qendian.h>
#include <atomic>
#include <vector>
#include <cmath>
#include <cstring>

#include "generator.h"
#include "white_math.h"

float buffer_rms(const std::vector<char>& buffer) {
    const float* ptr = reinterpret_cast<const float*>(buffer.data());
    const size_t n = buffer.size() / kBytesPerValue;
    return rms(ptr, ptr + n);
}

long generateData(
    long pos,
    void* buffer,
    size_t length,
    std::vector<double>& amplitudes,
    const std::vector<std::atomic<double>>& sharedAmplitudes,
    int keysNumber,
    double minFreq,
    double maxFreq,
    double volume) {

    float* ptr = reinterpret_cast<float*>(buffer);
    const size_t n = length / (kNumChannels * kBytesPerValue);
    double logMin = std::log(minFreq);
    double logMax = std::log(maxFreq);
    double kTrans = 0.002;

    for (size_t i = 0; i < n; ++i) {
        double a = 0;
        for (int k = 0; k < keysNumber; ++k) {
            const double logFreq = logMin + (logMax - logMin) * k / (keysNumber - 1);
            const double period = kSampleRate / std::exp(logFreq);
            amplitudes[k] = sharedAmplitudes[k] * kTrans + amplitudes[k] * (1. - kTrans);
            a += qSin(2 * M_PI * pos / period) * amplitudes[k];
        }
        float value = clamp(static_cast<float>(a * volume * kVolume), -1., 1.);
        for (int c = 0; c < kNumChannels; ++c) {
            *ptr = value;
            ++ptr;
        }
        ++pos;
    }
    return pos;
}

size_t Generator::readData(void* data, size_t nBufferFrames, bool stop) {
    auto len = nBufferFrames * kBytesPerValue * kNumChannels;
    process([=, &len](char* buffer) {
        len = std::min(len, buffer_.size());
        std::memcpy(data, buffer_.data(), len);
        return stop;
    });
    return len;
}

void Generator::generate() {
    bool stop = false;
    while(!stop) {
        auto minFreq = std::exp2((minNote_ - 69) / 12.0) * 440.0;
        auto maxFreq = std::exp2((maxNote_ - 69) / 12.0) * 440.0;
        pos_ = generateData(pos_, buffer_.data(), buffer_.size(), amplitudes_, sharedAmplitudes_->get(), keysNumber_, minFreq, maxFreq, volume_ / 100.);
        outputPromise_.set_value(true);
        stop = nextPromise_.get_future().get();
        nextPromise_ = std::promise<bool>();
    }
}

void Generator::process(std::function<bool(char*)> onBuffer) {
    outputPromise_.get_future().get();
    outputPromise_ = std::promise<bool>();
    auto stop = onBuffer(buffer_.data());
    nextPromise_.set_value(stop);
}
