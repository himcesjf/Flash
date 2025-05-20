#include "flashupcore.h"
#include "deviceinterface.h"
#include "firmwarepackage.h"
#include "updatejob.h"

#include <QDir>
#include <QPluginLoader>
#include <QDebug>
#include <QCoreApplication>

FlashUpCore::FlashUpCore(QObject *parent)
    : QObject(parent)
{
    registerPlugins();
    emit logMessage(1, "FlashUp Core initialized");
}

FlashUpCore::~FlashUpCore()
{
    // Cancel any active jobs before shutting down
    for (const auto &deviceId : m_activeJobs.keys()) {
        cancelUpdate(deviceId);
    }
}

void FlashUpCore::discoverDevices()
{
    emit logMessage(1, "Starting device discovery...");
    
    // Clear out stale devices
    m_devices.clear();
    
    // TODO: Implement plugin-based device discovery
    // 1. Load and initialize device plugins
    // 2. Scan for USB/Serial devices
    // 3. Scan for network devices
    // 4. Handle device connection/disconnection
    // 5. Implement device caching
    
    // Current implementation uses simulated devices for testing
    // Simulate finding a USB CDC device
    QMap<QString, QString> usbDevInfo;
    usbDevInfo["type"] = "USB-CDC";
    usbDevInfo["port"] = "/dev/ttyUSB0";
    usbDevInfo["description"] = "ESP32 Development Board";
    usbDevInfo["protocol"] = "ESP-IDF";
    emit deviceDiscovered("usb:ttyUSB0", usbDevInfo);
    
    // Simulate finding a network device
    QMap<QString, QString> wifiDevInfo;
    wifiDevInfo["type"] = "WiFi";
    wifiDevInfo["ip"] = "192.168.1.100";
    wifiDevInfo["hostname"] = "esp32-ota";
    wifiDevInfo["protocol"] = "ESP-OTA";
    emit deviceDiscovered("net:192.168.1.100", wifiDevInfo);
    
    emit logMessage(1, QString("Found %1 devices").arg(m_devices.size()));
}

QStringList FlashUpCore::availableDevices() const
{
    return m_devices.keys();
}

QMap<QString, QString> FlashUpCore::deviceInfo(const QString &deviceId) const
{
    if (m_devices.contains(deviceId) && m_devices[deviceId]) {
        return m_devices[deviceId]->deviceInfo();
    }
    return QMap<QString, QString>();
}

bool FlashUpCore::loadFirmware(const QString &filePath)
{
    emit logMessage(1, QString("Loading firmware from %1").arg(filePath));
    
    try {
        m_currentFirmware = std::make_unique<FirmwarePackage>(filePath);
        
        QMap<QString, QString> info = m_currentFirmware->metadata();
        emit logMessage(1, QString("Loaded firmware: %1 v%2").arg(
                           info.value("name", "Unknown"),
                           info.value("version", "0.0.0")));
        return true;
    } catch (const std::exception &e) {
        emit logMessage(3, QString("Failed to load firmware: %1").arg(e.what()));
        m_currentFirmware.reset();
        return false;
    }
}

QMap<QString, QString> FlashUpCore::firmwareInfo() const
{
    if (m_currentFirmware) {
        return m_currentFirmware->metadata();
    }
    return QMap<QString, QString>();
}

bool FlashUpCore::updateFirmware(const QString &deviceId, const QString &firmwarePath)
{
    // If a job is already active for this device, cancel it first
    if (m_activeJobs.contains(deviceId)) {
        cancelUpdate(deviceId);
    }
    
    // If a firmware path was provided, load it
    if (!firmwarePath.isEmpty() && !loadFirmware(firmwarePath)) {
        emit logMessage(3, "Failed to load firmware file");
        return false;
    }
    
    // Check if firmware is loaded
    if (!m_currentFirmware) {
        emit logMessage(3, "No firmware loaded");
        return false;
    }
    
    // Check if device exists
    if (!m_devices.contains(deviceId) || !m_devices[deviceId]) {
        emit logMessage(3, QString("Unknown device: %1").arg(deviceId));
        return false;
    }
    
    // Start the update job
    try {
        auto device = m_devices[deviceId];
        auto firmware = m_currentFirmware.get();
        
        // Create update job
        auto job = std::make_shared<UpdateJob>(device, firmware, this);
        
        // Connect signals
        connect(job.get(), &UpdateJob::progressChanged, this, 
                [this, deviceId](int progress, const QString &status) {
                    emit updateProgress(deviceId, progress, status);
                });
        
        connect(job.get(), &UpdateJob::completed, this,
                [this, deviceId](bool success, const QString &message) {
                    emit updateComplete(deviceId, success, message);
                    m_activeJobs.remove(deviceId);
                });
        
        connect(job.get(), &UpdateJob::logMessage, this,
                [this](int level, const QString &message) {
                    emit logMessage(level, message);
                });
        
        // Store the job
        m_activeJobs[deviceId] = job;
        
        // Start the job
        job->start();
        
        emit logMessage(1, QString("Started firmware update for device %1").arg(deviceId));
        
        return true;
    } catch (const std::exception &e) {
        emit logMessage(3, QString("Failed to start update: %1").arg(e.what()));
        return false;
    }
}

bool FlashUpCore::cancelUpdate(const QString &deviceId)
{
    if (!m_activeJobs.contains(deviceId) || !m_activeJobs[deviceId]) {
        emit logMessage(2, QString("No active update job for device %1").arg(deviceId));
        return false;
    }
    
    emit logMessage(1, QString("Canceling update for device %1").arg(deviceId));
    
    try {
        m_activeJobs[deviceId]->cancel();
        m_activeJobs.remove(deviceId);
        emit updateComplete(deviceId, false, "Update canceled by user");
        return true;
    } catch (const std::exception &e) {
        emit logMessage(3, QString("Failed to cancel update: %1").arg(e.what()));
        return false;
    }
}

void FlashUpCore::registerPlugins()
{
    // TODO: Implement dynamic plugin system
    // 1. Scan plugin directories
    // 2. Load plugin metadata and dependencies
    // 3. Validate plugin compatibility
    // 4. Initialize plugin instances
    // 5. Handle plugin versioning
    
    // Current implementation uses hard-coded plugins for testing
    emit logMessage(1, "Registering device plugins...");
    
    // TODO: Implement actual plugin registration
    // This would search for plugins in the application's plugin directory
    
    emit logMessage(1, "Device plugins registered");
} 