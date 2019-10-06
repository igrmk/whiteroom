#pragma once

#include <QObject>
#include <QQmlEngine>

class StateEnumModule : public QObject
{
    Q_OBJECT

public:
    enum class StateEnum
    {
        STOPPED,
        STARTED,
        STOPPING
    };

    Q_ENUMS(StateEnum)
};
