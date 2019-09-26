#pragma once

#include <QObject>

#include "audio_config_data.h"

class AudioConfig : public QObject {
    Q_OBJECT

    Q_PROPERTY(int samples READ samples WRITE setSamples)
    Q_PROPERTY(int device READ device WRITE setDevice)

public:
    explicit AudioConfig(QObject* parent = nullptr) : QObject(parent) {}

    int samples() { return data_.samples_; }
    void setSamples(int value) { data_.samples_ = value; }

    int device() { return data_.device_; }
    void setDevice(int value) { data_.device_ = value; }

    AudioConfigData data() { return data_; }

private:
    AudioConfigData data_;
};
