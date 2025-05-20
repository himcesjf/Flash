#ifndef DEVICEINTERFACE_H
#define DEVICEINTERFACE_H

#include <QObject>
#include <QString>
#include <QByteArray>
#include <QMap>

/**
 * @brief The DeviceInterface class defines the interface for device communication
 * 
 * This abstract class provides a common interface for different types of
 * devices (serial, network, etc.) that can be used for firmware updates.
 */
class DeviceInterface : public QObject
{
    Q_OBJECT

public:
    enum ConnectionStatus {
        Disconnected,
        Connecting,
        Connected,
        Error
    };
    
    enum DeviceState {
        Idle,
        Ready,
        Updating,
        Rebooting
    };

    explicit DeviceInterface(QObject *parent = nullptr);
    virtual ~DeviceInterface();

    /**
     * @brief Get device identifier
     * @return Unique device ID
     */
    virtual QString deviceId() const = 0;

    /**
     * @brief Get device information
     * @return Map of properties
     */
    virtual QMap<QString, QString> deviceInfo() const = 0;

    /**
     * @brief Connect to the device
     * @return true if connection successful
     */
    virtual bool connect() = 0;

    /**
     * @brief Disconnect from the device
     */
    virtual void disconnect() = 0;

    /**
     * @brief Check if device is connected
     * @return true if connected
     */
    virtual bool isConnected() const = 0;

    /**
     * @brief Get connection status
     * @return Current status
     */
    virtual ConnectionStatus connectionStatus() const = 0;

    /**
     * @brief Get device state
     * @return Current state
     */
    virtual DeviceState deviceState() const = 0;

    /**
     * @brief Start firmware update process
     * @return true if update initiated successfully
     */
    virtual bool beginUpdate() = 0;

    /**
     * @brief Send a chunk of firmware data
     * @param data Data chunk
     * @param offset Offset in firmware
     * @return true if sent successfully
     */
    virtual bool sendFirmwareChunk(const QByteArray &data, qint64 offset) = 0;

    /**
     * @brief Finalize the update process
     * @return true if finalized successfully
     */
    virtual bool finalizeUpdate() = 0;

    /**
     * @brief Cancel update in progress
     * @return true if canceled successfully
     */
    virtual bool cancelUpdate() = 0;

    /**
     * @brief Get optimal chunk size for this device
     * @return Chunk size in bytes
     */
    virtual qint64 optimalChunkSize() const = 0;

signals:
    /**
     * @brief Emitted when connection status changes
     * @param status New status
     */
    void connectionStatusChanged(ConnectionStatus status);

    /**
     * @brief Emitted when device state changes
     * @param state New state
     */
    void deviceStateChanged(DeviceState state);

    /**
     * @brief Emitted for log messages
     * @param level Log level (0=debug, 1=info, 2=warning, 3=error)
     * @param message Log message
     */
    void logMessage(int level, const QString &message);
};

#endif // DEVICEINTERFACE_H 