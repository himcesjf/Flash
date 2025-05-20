#ifndef UPDATEJOB_H
#define UPDATEJOB_H

#include <QObject>
#include <QTimer>
#include <memory>

class DeviceInterface;
class FirmwarePackage;

/**
 * @brief The UpdateJob class manages the firmware update process for a device
 */
class UpdateJob : public QObject
{
    Q_OBJECT

public:
    enum State {
        Idle,
        Connecting,
        Preparing,
        Uploading,
        Finalizing,
        Complete,
        Failed,
        Canceled
    };

    /**
     * @brief Construct a new UpdateJob
     * @param device Device to update
     * @param firmware Firmware package to use
     * @param parent Parent object
     */
    UpdateJob(std::shared_ptr<DeviceInterface> device, 
              FirmwarePackage *firmware,
              QObject *parent = nullptr);
    
    ~UpdateJob();

    /**
     * @brief Start the update process
     */
    void start();

    /**
     * @brief Cancel the update process
     */
    void cancel();

    /**
     * @brief Get current state
     * @return Current state
     */
    State state() const;

    /**
     * @brief Get current progress
     * @return Progress value (0-100)
     */
    int progress() const;

signals:
    /**
     * @brief Emitted when update progress changes
     * @param progress Progress percentage (0-100)
     * @param status Status message
     */
    void progressChanged(int progress, const QString &status);

    /**
     * @brief Emitted when update completes
     * @param success Whether update was successful
     * @param message Result message
     */
    void completed(bool success, const QString &message);

    /**
     * @brief Emitted for log messages
     * @param level Log level (0=debug, 1=info, 2=warning, 3=error)
     * @param message Log message
     */
    void logMessage(int level, const QString &message);

private slots:
    void onDeviceConnectionStatusChanged(DeviceInterface::ConnectionStatus status);
    void onDeviceStateChanged(DeviceInterface::DeviceState state);
    void onUploadNextChunk();
    void onRetryTimeout();

private:
    std::shared_ptr<DeviceInterface> m_device;
    FirmwarePackage *m_firmware;
    State m_state;
    int m_progress;
    qint64 m_currentOffset;
    qint64 m_chunkSize;
    int m_retryCount;
    int m_maxRetries;
    QTimer m_retryTimer;
    QTimer m_chunkTimer;
    bool m_paused;

    void setState(State state);
    void setProgress(int progress);
    void startUpload();
    void failUpdate(const QString &reason);
    void completeUpdate();
};

#endif // UPDATEJOB_H 