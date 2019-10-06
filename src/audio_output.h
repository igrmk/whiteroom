#pragma once

#include <QObject>
#include <QStringListModel>
#include <memory>
#include <vector>
#include <string>
#include <algorithm>
#include <functional>
#include <memory>

#ifdef __linux__
    #include <alsa/asoundlib.h>
    #include <jack/jack.h>
#endif

#include <portaudiocpp/PortAudioCpp.hxx>

#include "audio.h"
#include "generator.h"
#include "log.h"
#include "audio_config_data.h"

class AudioOutput : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QStringListModel * devices READ devices NOTIFY signalDevicesChanged)
    Q_PROPERTY(Generator * generator MEMBER generator_)
    Q_PROPERTY(int defaultDevice READ defaultDevice)

public:
    explicit AudioOutput(QObject* parent = nullptr) : QObject(parent), log_(getlog()) {
        initializeAudio();
    }

    ~AudioOutput() override { stop(); }

    QStringListModel* devices() {
        auto names = device_names();
        QStringList result;
        std::transform(
            std::begin(names),
            std::end(names),
            std::back_inserter(result),
            [] (std::string from) { return QString::fromUtf8(from.data()); });

        return new QStringListModel(result);
    }

    void setAudioConfig(AudioConfigData config) { audioConfig_ = config; }

    int defaultDevice() { return audioSystem_->defaultOutputDevice().index(); }

    void start() {
        if (audioConfig_.device_ == -1) {
            return;
        }

        working_ = true;

        try {
            auto& device = audioSystem_->deviceByIndex(audioConfig_.device_);
            auto params = getParams(device, audioConfig_.samples_);
            stream_ = std::make_unique<portaudio::MemFunCallbackStream<AudioOutput>>(params, *this, &AudioOutput::internalCallback);
            log_->info("starting audio stream...");
            stream_->start();
        }
        catch(const std::exception& error) {
            log_->error("error on starting audio stream: {}", error.what());
        }
    }

    void stop() {
        if (stream_) {
            log_->info("stopping and closing audio stream...");
            working_ = false;
            stream_->stop();
            stream_->close();
            stream_.reset();
            log_->info("audio stream stopped");
        }
    }

signals:
    void signalDevicesChanged();

private:
    void initializeAudio();

    int internalCallback(
        [[maybe_unused]] const void* inputBuffer,
        void* outputBuffer,
        unsigned long nBufferFrames,
        [[maybe_unused]] const PaStreamCallbackTimeInfo* timeInfo,
        [[maybe_unused]] PaStreamCallbackFlags flags) {

        if (generator_) {
            generator_->readData(outputBuffer, nBufferFrames, false);
        }

        return 0;
    }

    std::unique_ptr<portaudio::AutoSystem> raiiSystem_;
    portaudio::System* audioSystem_ = nullptr;
    std::unique_ptr<portaudio::MemFunCallbackStream<AudioOutput>> stream_;
    Generator* generator_ = nullptr;
    logger log_;
    AudioConfigData audioConfig_;
    std::atomic<bool> working_ = false;
};
