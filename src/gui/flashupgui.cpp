#include "flashupgui.h"
#include "logmodel.h"
#include "core/flashupcore.h"

#include <QDebug>
#include <QFileInfo>
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QDateTime>

// Auto-refresh interval in milliseconds
const int AUTO_REFRESH_INTERVAL = 5000;

FlashUpGUI::FlashUpGUI(FlashUpCore *core, QObject *parent)
    : QObject(parent),
      m_core(core),
      m_updateProgress(0),
      m_updateStatus("Idle"),
      m_updateActive(false),
      m_logModel(new LogModel(this))
{
    // Connect core signals
    connect(m_core, &FlashUpCore::deviceDiscovered,
            this, &FlashUpGUI::onDeviceDiscovered);
    connect(m_core, &FlashUpCore::deviceLost,
            this, &FlashUpGUI::onDeviceLost);
    connect(m_core, &FlashUpCore::updateProgress,
            this, &FlashUpGUI::onUpdateProgress);
    connect(m_core, &FlashUpCore::updateComplete,
            this, &FlashUpGUI::onUpdateComplete);
    connect(m_core, &FlashUpCore::logMessage,
            this, &FlashUpGUI::onLogMessage);
    
    // Setup auto-refresh timer
    connect(&m_autoRefreshTimer, &QTimer::timeout,
            this, &FlashUpGUI::autoRefreshDevices);
    m_autoRefreshTimer.setInterval(AUTO_REFRESH_INTERVAL);
    m_autoRefreshTimer.start();
    
    // Initial device discovery
    QTimer::singleShot(100, this, &FlashUpGUI::refreshDevices);
}

FlashUpGUI::~FlashUpGUI()
{
    m_autoRefreshTimer.stop();
}

QStringList FlashUpGUI::deviceList() const
{
    return m_deviceList;
}

QVariantMap FlashUpGUI::firmwareInfo() const
{
    return m_firmwareInfo;
}

QString FlashUpGUI::selectedDevice() const
{
    return m_selectedDevice;
}

void FlashUpGUI::setSelectedDevice(const QString &deviceId)
{
    if (m_selectedDevice != deviceId) {
        m_selectedDevice = deviceId;
        emit selectedDeviceChanged();
    }
}

int FlashUpGUI::updateProgress() const
{
    return m_updateProgress;
}

QString FlashUpGUI::updateStatus() const
{
    return m_updateStatus;
}

bool FlashUpGUI::updateActive() const
{
    return m_updateActive;
}

LogModel* FlashUpGUI::logModel() const
{
    return m_logModel;
}

void FlashUpGUI::refreshDevices()
{
    m_core->discoverDevices();
}

bool FlashUpGUI::loadFirmware(const QUrl &fileUrl)
{
    QString filePath = fileUrl.toLocalFile();
    
    if (filePath.isEmpty()) {
        emit notification("Error", "Invalid file path", 2);
        return false;
    }
    
    onLogMessage(1, QString("Loading firmware from %1").arg(filePath));
    
    if (m_core->loadFirmware(filePath)) {
        // Update firmware info
        QMap<QString, QString> info = m_core->firmwareInfo();
        m_firmwareInfo.clear();
        
        for (auto it = info.constBegin(); it != info.constEnd(); ++it) {
            m_firmwareInfo[it.key()] = it.value();
        }
        
        emit firmwareInfoChanged();
        
        QString name = info.value("name", "Unknown");
        QString version = info.value("version", "0.0.0");
        emit notification("Firmware Loaded", QString("%1 v%2").arg(name, version), 3);
        
        return true;
    } else {
        emit notification("Error", "Failed to load firmware file", 2);
        return false;
    }
}

bool FlashUpGUI::startUpdate()
{
    if (m_selectedDevice.isEmpty()) {
        emit notification("Error", "No device selected", 2);
        return false;
    }
    
    if (m_firmwareInfo.isEmpty()) {
        emit notification("Error", "No firmware loaded", 2);
        return false;
    }
    
    onLogMessage(1, QString("Starting update for device %1").arg(m_selectedDevice));
    
    if (m_core->updateFirmware(m_selectedDevice)) {
        m_updateActive = true;
        emit updateActiveChanged();
        return true;
    } else {
        emit notification("Error", "Failed to start update", 2);
        return false;
    }
}

bool FlashUpGUI::cancelUpdate()
{
    if (!m_updateActive) {
        return false;
    }
    
    onLogMessage(1, "Canceling update");
    
    if (m_core->cancelUpdate(m_selectedDevice)) {
        m_updateActive = false;
        emit updateActiveChanged();
        emit notification("Update Canceled", "Firmware update was canceled", 1);
        return true;
    } else {
        emit notification("Error", "Failed to cancel update", 2);
        return false;
    }
}

QVariantMap FlashUpGUI::getDeviceInfo(const QString &deviceId) const
{
    QMap<QString, QString> info = m_core->deviceInfo(deviceId);
    QVariantMap result;
    
    for (auto it = info.constBegin(); it != info.constEnd(); ++it) {
        result[it.key()] = it.value();
    }
    
    return result;
}

void FlashUpGUI::clearLogs()
{
    m_logModel->clear();
}

bool FlashUpGUI::saveLogs(const QUrl &fileUrl) const
{
    QString filePath = fileUrl.toLocalFile();
    
    if (filePath.isEmpty()) {
        qWarning() << "Invalid file path for log saving";
        return false;
    }
    
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qWarning() << "Failed to open file for writing:" << file.errorString();
        return false;
    }
    
    QTextStream out(&file);
    out << "FlashUp Log - " << QDateTime::currentDateTime().toString() << "\n\n";
    
    for (int i = 0; i < m_logModel->rowCount(); ++i) {
        QDateTime timestamp = m_logModel->data(m_logModel->index(i, 0), LogModel::TimestampRole).toDateTime();
        QString level = m_logModel->data(m_logModel->index(i, 0), LogModel::LevelStrRole).toString();
        QString message = m_logModel->data(m_logModel->index(i, 0), LogModel::MessageRole).toString();
        
        out << timestamp.toString("yyyy-MM-dd HH:mm:ss.zzz") << " [" << level << "] " << message << "\n";
    }
    
    file.close();
    return true;
}

void FlashUpGUI::onDeviceDiscovered(const QString &deviceId, const QMap<QString, QString> &info)
{
    if (!m_deviceList.contains(deviceId)) {
        m_deviceList.append(deviceId);
        emit deviceListChanged();
        
        QString type = info.value("type", "Unknown");
        QString description = info.value("description", deviceId);
        
        onLogMessage(1, QString("Discovered %1 device: %2").arg(type, description));
        
        if (m_deviceList.size() == 1) {
            // Auto-select first device
            setSelectedDevice(deviceId);
        }
    }
}

void FlashUpGUI::onDeviceLost(const QString &deviceId)
{
    if (m_deviceList.contains(deviceId)) {
        m_deviceList.removeOne(deviceId);
        emit deviceListChanged();
        
        onLogMessage(1, QString("Device lost: %1").arg(deviceId));
        
        if (m_selectedDevice == deviceId) {
            if (m_deviceList.isEmpty()) {
                setSelectedDevice(QString());
            } else {
                setSelectedDevice(m_deviceList.first());
            }
        }
    }
}

void FlashUpGUI::onUpdateProgress(const QString &deviceId, int progress, const QString &status)
{
    if (deviceId == m_selectedDevice) {
        m_updateProgress = progress;
        m_updateStatus = status;
        emit updateProgressChanged();
        emit updateStatusChanged();
    }
}

void FlashUpGUI::onUpdateComplete(const QString &deviceId, bool success, const QString &message)
{
    if (deviceId == m_selectedDevice) {
        m_updateActive = false;
        emit updateActiveChanged();
        
        if (success) {
            emit notification("Update Complete", message, 3);
        } else {
            emit notification("Update Failed", message, 2);
        }
    }
}

void FlashUpGUI::onLogMessage(int level, const QString &message)
{
    m_logModel->addMessage(level, message);
}

void FlashUpGUI::autoRefreshDevices()
{
    // Only auto-refresh if no update is in progress
    if (!m_updateActive) {
        refreshDevices();
    }
} 