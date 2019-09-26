#pragma once

#include <vector>
#include <atomic>

#include <QObject>

class SharedAmplitudes : public QObject {
    Q_OBJECT

    Q_PROPERTY(int maxKeysNumber READ maxKeysNumber WRITE setMaxKeysNumber NOTIFY maxKeysNumberChanged)

public:
    explicit SharedAmplitudes(QObject* parent = nullptr) : QObject(parent) {}

    int maxKeysNumber() { return amplitudes_.size(); }
    void setMaxKeysNumber(int x) {
        amplitudes_ = std::vector<std::atomic<double>>(x);
        std::fill(amplitudes_.begin(), amplitudes_.end(), 0.);
        emit maxKeysNumberChanged();
    }

    std::vector<std::atomic<double>>& get() { return amplitudes_; }

signals:
    void maxKeysNumberChanged();

private:
    std::vector<std::atomic<double>> amplitudes_;
};
