#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQuickStyle>
#include <QLibraryInfo>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_sinks.h>

#include "controller.h"
#include "stateenum.h"
#include "generator.h"
#include "camera.h"
#include "audio_output.h"
#include "log.h"
#include "stateenum.h"
#include "audio_config.h"
#include "shared_amplitudes.h"

void publishTypes() {
    qmlRegisterType<Generator>("Generator", 1, 0, "Generator");
    qmlRegisterType<AudioOutput>("AudioOutput", 1, 0, "AudioOutput");
    qmlRegisterType<StateEnumModule>("StateEnum", 1, 0, "StateEnum");
    qmlRegisterType<Controller>("Controller", 1, 0, "Controller");
    qmlRegisterType<Camera>("Camera", 1, 0, "Camera");
    qmlRegisterType<AudioConfig>("AudioConfig", 1, 0, "AudioConfig");
    qmlRegisterType<SharedAmplitudes>("SharedAmplitudes", 1, 0, "SharedAmplitudes");
}

int main(int argc, char* argv[]) {
    auto log = spdlog::stderr_logger_mt("con");
    log->set_pattern(log_pattern);
    log->set_level(spdlog::level::trace);

    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

    log->info("creating application");
    QGuiApplication app(argc, argv);

    log->info("enabling theme");
    QQuickStyle::setStyle("Material");

    log->info("creating engine");
    QQmlApplicationEngine engine;

    log->info("publishing types");
    publishTypes();

    log->info("plugins path: {}", QLibraryInfo::location(QLibraryInfo::PluginsPath).toStdString());

    log->info("loading UI");
    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));
    if (engine.rootObjects().isEmpty()) {
        return -1;
    }

    return app.exec();
}
