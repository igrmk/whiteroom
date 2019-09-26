#pragma once

#include <vector>
#include <string>
#include <portaudiocpp/PortAudioCpp.hxx>

const int kSampleRate = 44100;
const int kNumChannels = 2;
const int kBytesPerValue = 4;
const bool kInterleaved = true;
const double kVolume = 2.;
const PaStreamFlags kClipOff = paClipOff;

inline std::vector<std::string> device_names() {
    auto result = std::vector<std::string>();
    auto system = &portaudio::System::instance();
    for (auto i = system->devicesBegin(); i != system->devicesEnd(); ++i) {
        auto name = std::string(i->hostApi().name()) + ": " + i->name();
        result.push_back(name);
    }
    return result;
}

inline portaudio::StreamParameters getParams(const portaudio::Device& device, int samples) {
    portaudio::DirectionSpecificStreamParameters outParams(
        device,
        kNumChannels,
        portaudio::FLOAT32,
        kInterleaved,
        device.defaultLowOutputLatency(),
        nullptr);

    return portaudio::StreamParameters(
        portaudio::DirectionSpecificStreamParameters::null(),
        outParams,
        kSampleRate,
        static_cast<unsigned long>(samples),
        kClipOff);
}
