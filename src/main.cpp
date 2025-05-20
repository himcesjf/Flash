#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QIcon>
#include <QFontDatabase>
#include <QCommandLineParser>
#include <QDir>

#include "gui/flashupgui.h"
#include "core/flashupcore.h"

int main(int argc, char *argv[])
{
    // Set application attributes
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QCoreApplication::setOrganizationName("FlashUp");
    QCoreApplication::setOrganizationDomain("flashup.io");
    QCoreApplication::setApplicationName("FlashUp");
    QCoreApplication::setApplicationVersion("0.1.0");

    // Create the application
    QGuiApplication app(argc, argv);
    app.setWindowIcon(QIcon(":/images/logo.png"));

    // Command line parser
    QCommandLineParser parser;
    parser.setApplicationDescription("FlashUp - Firmware/OTA updater & diagnostics tool");
    parser.addHelpOption();
    parser.addVersionOption();
    
    QCommandLineOption headlessOption({"s", "script"}, "Run in headless script mode");
    parser.addOption(headlessOption);
    
    QCommandLineOption firmwareOption({"f", "firmware"}, "Firmware file path", "filepath");
    parser.addOption(firmwareOption);
    
    QCommandLineOption deviceOption({"d", "device"}, "Target device identifier", "device");
    parser.addOption(deviceOption);
    
    parser.process(app);
    
    bool headless = parser.isSet(headlessOption);
    QString firmwarePath = parser.value(firmwareOption);
    QString deviceId = parser.value(deviceOption);

    // Initialize the core
    FlashUpCore core;
    
    // Check for headless mode
    if (headless) {
        if (firmwarePath.isEmpty() || deviceId.isEmpty()) {
            qCritical() << "Firmware path and device ID are required in headless mode.";
            return 1;
        }
        
        bool success = core.updateFirmware(deviceId, firmwarePath);
        return success ? 0 : 1;
    }
    
    // GUI mode
    QQmlApplicationEngine engine;
    
    // Register FlashUpGUI as QML type
    FlashUpGUI gui(&core);
    engine.rootContext()->setContextProperty("flashupGui", &gui);
    
    // Load main QML file
    const QUrl url(QStringLiteral("qrc:/qml/main.qml"));
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [url](QObject *obj, const QUrl &objUrl) {
        if (!obj && url == objUrl)
            QCoreApplication::exit(-1);
    }, Qt::QueuedConnection);
    
    engine.load(url);
    
    return app.exec();
} 