#pragma once

#include <memory>

#include <QObject>
#include <QDebug>
#include <QQmlProperty>
#include <QQmlEngine>

#include "camera.h"
#include "stateenum.h"
#include "generator.h"
#include "audio_output.h"
#include "audio_config.h"
#include "log.h"

class Controller : public QObject
{
    Q_OBJECT
    Q_PROPERTY(Camera * camera1 MEMBER camera1_ NOTIFY signalCamera1Changed)
    Q_PROPERTY(Generator * generator MEMBER generator_ NOTIFY signalGeneratorChanged)
    Q_PROPERTY(AudioOutput * output MEMBER output_ NOTIFY signalOutputChanged)
    Q_PROPERTY(AudioConfig * config MEMBER audioConfig_ NOTIFY signalAudioConfigChanged)
    Q_PROPERTY(StateEnumModule::StateEnum state MEMBER state_ NOTIFY signalStateChanged)

public:
    explicit Controller(QObject* parent = nullptr) : QObject(parent), log_(getlog()) {}

signals:
    void signalCamera1Changed();
    void signalGeneratorChanged();
    void signalStateChanged();
    void signalOutputChanged();
    void signalAudioConfigChanged();

public slots:
    void onStartClicked() {
        setState(StateEnumModule::StateEnum::STARTED);
        auto config = audioConfig_->data();
        log_->info("starting, config: {}", config);
        generator_->setAudioConfig(audioConfig_->data());
        output_->setAudioConfig(audioConfig_->data());

        camera1_->start();
        generator_->start();
        output_->start();
    }

    void onStopClicked() {
        setState(StateEnumModule::StateEnum::STOPPING);
        log_->info("stopping...");
        output_->stop();
        generator_->stop();
        camera1_->stop();
        setState(StateEnumModule::StateEnum::STOPPED);
    }

private:
    Camera* camera1_ = nullptr;
    Generator* generator_ = nullptr;
    AudioOutput* output_ = nullptr;
    AudioConfig* audioConfig_ = nullptr;
    StateEnumModule::StateEnum state_ = StateEnumModule::StateEnum::STOPPED;
    logger log_;

    void setState(StateEnumModule::StateEnum state) {
        state_ = state;
        emit signalStateChanged();
    }
};
