#ifndef FLASHUPGUI_H
#define FLASHUPGUI_H

#include <QObject>
#include <QStringList>
#include <QMap>
#include <QVariant>
#include <QUrl>
#include <QTimer>

class FlashUpCore;
class LogModel;

/**
 * @brief The FlashUpGUI class serves as the bridge between QML UI and the core functionality
 */
class FlashUpGUI : public QObject
{
    Q_OBJECT
    
    // Properties exposed to QML
    Q_PROPERTY(QStringList deviceList READ deviceList NOTIFY deviceListChanged)
    Q_PROPERTY(QVariantMap firmwareInfo READ firmwareInfo NOTIFY firmwareInfoChanged)
    Q_PROPERTY(QString selectedDevice READ selectedDevice WRITE setSelectedDevice NOTIFY selectedDeviceChanged)
    Q_PROPERTY(int updateProgress READ updateProgress NOTIFY updateProgressChanged)
    Q_PROPERTY(QString updateStatus READ updateStatus NOTIFY updateStatusChanged)
    Q_PROPERTY(bool updateActive READ updateActive NOTIFY updateActiveChanged)
    Q_PROPERTY(LogModel* logModel READ logModel CONSTANT)

public:
    explicit FlashUpGUI(FlashUpCore *core, QObject *parent = nullptr);
    ~FlashUpGUI();
    
    // Property getters/setters
    QStringList deviceList() const;
    QVariantMap firmwareInfo() const;
    QString selectedDevice() const;
    void setSelectedDevice(const QString &deviceId);
    int updateProgress() const;
    QString updateStatus() const;
    bool updateActive() const;
    LogModel* logModel() const;

public slots:
    /**
     * @brief Refresh the list of available devices
     */
    void refreshDevices();
    
    /**
     * @brief Load firmware from file
     * @param fileUrl URL to firmware file
     * @return true if successful
     */
    bool loadFirmware(const QUrl &fileUrl);
    
    /**
     * @brief Start firmware update process
     * @return true if update started successfully
     */
    bool startUpdate();
    
    /**
     * @brief Cancel an ongoing update
     * @return true if canceled successfully
     */
    bool cancelUpdate();
    
    /**
     * @brief Get information about a device
     * @param deviceId The device ID
     * @return Map of device properties
     */
    QVariantMap getDeviceInfo(const QString &deviceId) const;
    
    /**
     * @brief Clear logs
     */
    void clearLogs();
    
    /**
     * @brief Save logs to file
     * @param fileUrl URL to save logs to
     * @return true if successful
     */
    bool saveLogs(const QUrl &fileUrl) const;

signals:
    // Property change signals
    void deviceListChanged();
    void firmwareInfoChanged();
    void selectedDeviceChanged();
    void updateProgressChanged();
    void updateStatusChanged();
    void updateActiveChanged();
    
    /**
     * @brief Emitted when a notification should be shown to the user
     * @param title Notification title
     * @param message Notification message
     * @param type Notification type (0=info, 1=warning, 2=error, 3=success)
     */
    void notification(const QString &title, const QString &message, int type);

private slots:
    void onDeviceDiscovered(const QString &deviceId, const QMap<QString, QString> &info);
    void onDeviceLost(const QString &deviceId);
    void onUpdateProgress(const QString &deviceId, int progress, const QString &status);
    void onUpdateComplete(const QString &deviceId, bool success, const QString &message);
    void onLogMessage(int level, const QString &message);
    void autoRefreshDevices();

private:
    FlashUpCore *m_core;
    QStringList m_deviceList;
    QString m_selectedDevice;
    QVariantMap m_firmwareInfo;
    int m_updateProgress;
    QString m_updateStatus;
    bool m_updateActive;
    LogModel *m_logModel;
    QTimer m_autoRefreshTimer;
};

#endif // FLASHUPGUI_H 