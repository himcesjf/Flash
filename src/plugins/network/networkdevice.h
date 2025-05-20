#ifndef NETWORKDEVICE_H
#define NETWORKDEVICE_H

#include "core/deviceinterface.h"

#include <QTcpSocket>
#include <QTimer>
#include <QByteArray>
#include <QQueue>
#include <QHostAddress>

/**
 * @brief The NetworkDevice class implements DeviceInterface for network-connected devices
 */
class NetworkDevice : public DeviceInterface
{
    Q_OBJECT

public:
    explicit NetworkDevice(const QString &address, quint16 port = 8266, QObject *parent = nullptr);
    ~NetworkDevice();

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
    void onConnected();
    void onDisconnected();
    void onError(QAbstractSocket::SocketError error);
    void onReadyRead();
    void onTimeout();
    void processResponse();

private:
    QString m_address;
    quint16 m_port;
    QTcpSocket m_socket;
    ConnectionStatus m_status;
    DeviceState m_state;
    QByteArray m_buffer;
    QTimer m_timeoutTimer;
    QQueue<QByteArray> m_pendingCommands;
    bool m_waitingForResponse;

    // Network protocol commands
    QByteArray createRequest(const QString &cmd, const QByteArray &data = QByteArray());
    bool sendRequest(const QByteArray &req);
    void sendNextRequest();
};

#endif // NETWORKDEVICE_H 