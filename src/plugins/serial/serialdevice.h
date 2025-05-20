#ifndef SERIALDEVICE_H
#define SERIALDEVICE_H

#include "core/deviceinterface.h"

#include <QSerialPort>
#include <QTimer>
#include <QByteArray>
#include <QQueue>

/**
 * @brief The SerialDevice class implements DeviceInterface for serial devices
 */
class SerialDevice : public DeviceInterface
{
    Q_OBJECT

public:
    explicit SerialDevice(const QString &portName, QObject *parent = nullptr);
    ~SerialDevice();

    // DeviceInterface interface
    QString deviceId() const override;
    QMap<QString, QString> deviceInfo() const override;
    bool connect() override;
    void disconnect() override;
    bool isConnected() const override;
    ConnectionStatus connectionStatus() const override;
    DeviceState deviceState() const override;
    bool beginUpdate() override;
    bool sendFirmwareChunk(const QByteArray &data, qint64 offset) override;
    bool finalizeUpdate() override;
    bool cancelUpdate() override;
    qint64 optimalChunkSize() const override;

private slots:
    void onReadyRead();
    void onError(QSerialPort::SerialPortError error);
    void onTimeout();
    void processResponse();

private:
    QString m_portName;
    QSerialPort m_serialPort;
    ConnectionStatus m_status;
    DeviceState m_state;
    QByteArray m_buffer;
    QTimer m_timeoutTimer;
    QQueue<QByteArray> m_pendingCommands;
    bool m_waitingForAck;

    // Serial protocol commands
    QByteArray createCommand(const QString &cmd, const QByteArray &data = QByteArray());
    bool sendCommand(const QByteArray &cmd);
    void sendNextCommand();
};

#endif // SERIALDEVICE_H 