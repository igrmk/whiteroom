#ifndef STATEENUM_H
#define STATEENUM_H

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

#endif // STATEENUM_H
