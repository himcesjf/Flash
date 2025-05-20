#ifndef FLASHUPCORE_H
#define FLASHUPCORE_H

#include <QObject>
#include <QMap>
#include <QVector>
#include <QStringList>
#include <QString>
#include <QUrl>
#include <QFile>
#include <memory>

class DeviceInterface;
class FirmwarePackage;
class UpdateJob;

/**
 * @brief The FlashUpCore class manages firmware updates and device interactions
 */
class FlashUpCore : public QObject
{
    Q_OBJECT

public:
    explicit FlashUpCore(QObject *parent = nullptr);
    ~FlashUpCore();

    /**
     * @brief Discover available devices using all registered plugins
     */
    void discoverDevices();

    /**
     * @brief Get list of discovered devices
     * @return List of device IDs
     */
    QStringList availableDevices() const;

    /**
     * @brief Get detailed information about a device
     * @param deviceId The device identifier
     * @return Map of properties
     */
    QMap<QString, QString> deviceInfo(const QString &deviceId) const;

    /**
     * @brief Load a firmware package from file
     * @param filePath Path to firmware file
     * @return true if successful, false otherwise
     */
    bool loadFirmware(const QString &filePath);

    /**
     * @brief Get information about currently loaded firmware
     * @return Map of firmware properties
     */
    QMap<QString, QString> firmwareInfo() const;

    /**
     * @brief Start firmware update process
     * @param deviceId The device to update
     * @param firmwarePath Path to firmware file (optional if already loaded)
     * @return true if update started successfully, false otherwise
     */
    bool updateFirmware(const QString &deviceId, const QString &firmwarePath = QString());

    /**
     * @brief Cancel an ongoing update
     * @param deviceId The device being updated
     * @return true if canceled successfully, false otherwise
     */
    bool cancelUpdate(const QString &deviceId);

signals:
    /**
     * @brief Emitted when a new device is discovered
     * @param deviceId The device identifier
     * @param info Device information
     */
    void deviceDiscovered(const QString &deviceId, const QMap<QString, QString> &info);

    /**
     * @brief Emitted when a device is no longer available
     * @param deviceId The device identifier
     */
    void deviceLost(const QString &deviceId);

    /**
     * @brief Emitted when update progress changes
     * @param deviceId The device being updated
     * @param progress Progress percentage (0-100)
     * @param status Status message
     */
    void updateProgress(const QString &deviceId, int progress, const QString &status);

    /**
     * @brief Emitted when an update completes
     * @param deviceId The device that was updated
     * @param success Whether the update was successful
     * @param message Result message
     */
    void updateComplete(const QString &deviceId, bool success, const QString &message);

    /**
     * @brief Emitted for log messages
     * @param level Log level (0=debug, 1=info, 2=warning, 3=error)
     * @param message The log message
     */
    void logMessage(int level, const QString &message);

private:
    QMap<QString, std::shared_ptr<DeviceInterface>> m_devices;
    std::unique_ptr<FirmwarePackage> m_currentFirmware;
    QMap<QString, std::shared_ptr<UpdateJob>> m_activeJobs;
    
    // Register plugins
    void registerPlugins();
};

#endif // FLASHUPCORE_H 