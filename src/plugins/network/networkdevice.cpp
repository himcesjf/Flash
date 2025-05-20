#include "networkdevice.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QDataStream>
#include <QDebug>

// Constants
const int TIMEOUT_MS = 5000;
const qint64 DEFAULT_CHUNK_SIZE = 4096;

NetworkDevice::NetworkDevice(const QString &address, quint16 port, QObject *parent)
    : DeviceInterface(parent),
      m_address(address),
      m_port(port),
      m_status(Disconnected),
      m_state(Idle),
      m_waitingForResponse(false)
{
    // Connect socket signals
    QObject::connect(&m_socket, &QTcpSocket::connected,
                     this, &NetworkDevice::onConnected);
    QObject::connect(&m_socket, &QTcpSocket::disconnected,
                     this, &NetworkDevice::onDisconnected);
    QObject::connect(&m_socket, QOverload<QAbstractSocket::SocketError>::of(&QTcpSocket::error),
                     this, &NetworkDevice::onError);
    QObject::connect(&m_socket, &QTcpSocket::readyRead,
                     this, &NetworkDevice::onReadyRead);
    
    // Setup timeout timer
    m_timeoutTimer.setSingleShot(true);
    QObject::connect(&m_timeoutTimer, &QTimer::timeout,
                     this, &NetworkDevice::onTimeout);
}

NetworkDevice::~NetworkDevice()
{
    disconnect();
}

QString NetworkDevice::deviceId() const
{
    return QString("net:%1:%2").arg(m_address).arg(m_port);
}

QMap<QString, QString> NetworkDevice::deviceInfo() const
{
    QMap<QString, QString> info;
    info["type"] = "Network";
    info["address"] = m_address;
    info["port"] = QString::number(m_port);
    info["status"] = (m_socket.state() == QAbstractSocket::ConnectedState) ? "Connected" : "Disconnected";
    return info;
}

bool NetworkDevice::connect()
{
    if (m_socket.state() == QAbstractSocket::ConnectedState) {
        // Already connected
        return true;
    }
    
    emit logMessage(1, QString("Connecting to device at %1:%2...").arg(m_address).arg(m_port));
    
    m_status = Connecting;
    emit connectionStatusChanged(m_status);
    
    m_socket.connectToHost(m_address, m_port);
    
    // Start timeout timer
    m_timeoutTimer.start(TIMEOUT_MS);
    
    return true;
}

void NetworkDevice::disconnect()
{
    if (m_socket.state() != QAbstractSocket::UnconnectedState) {
        m_socket.disconnectFromHost();
        m_socket.waitForDisconnected(1000);
    }
    
    m_buffer.clear();
    m_pendingCommands.clear();
    m_timeoutTimer.stop();
    m_waitingForResponse = false;
    
    m_status = Disconnected;
    emit connectionStatusChanged(m_status);
    
    emit logMessage(1, "Disconnected from network device");
}

bool NetworkDevice::isConnected() const
{
    return m_socket.state() == QAbstractSocket::ConnectedState;
}

DeviceInterface::ConnectionStatus NetworkDevice::connectionStatus() const
{
    return m_status;
}

DeviceInterface::DeviceState NetworkDevice::deviceState() const
{
    return m_state;
}

bool NetworkDevice::beginUpdate()
{
    if (!isConnected()) {
        emit logMessage(3, "Cannot begin update: device not connected");
        return false;
    }
    
    emit logMessage(1, "Beginning firmware update...");
    
    // Send update begin request
    QJsonObject data;
    data["action"] = "begin_update";
    
    QByteArray jsonData = QJsonDocument(data).toJson(QJsonDocument::Compact);
    
    if (!sendRequest(createRequest("update", jsonData))) {
        emit logMessage(3, "Failed to send update begin request");
        return false;
    }
    
    return true;
}

bool NetworkDevice::sendFirmwareChunk(const QByteArray &data, qint64 offset)
{
    if (!isConnected() || m_state != Updating) {
        emit logMessage(3, "Cannot send firmware: device not in update mode");
        return false;
    }
    
    // Create chunk request
    QJsonObject reqData;
    reqData["action"] = "write_chunk";
    reqData["offset"] = static_cast<qint64>(offset);
    reqData["size"] = data.size();
    
    QByteArray jsonData = QJsonDocument(reqData).toJson(QJsonDocument::Compact);
    QByteArray request = createRequest("update", jsonData + "\n" + data);
    
    if (!sendRequest(request)) {
        emit logMessage(3, QString("Failed to send firmware chunk at offset %1").arg(offset));
        return false;
    }
    
    return true;
}

bool NetworkDevice::finalizeUpdate()
{
    if (!isConnected() || m_state != Updating) {
        emit logMessage(3, "Cannot finalize update: device not in update mode");
        return false;
    }
    
    emit logMessage(1, "Finalizing firmware update...");
    
    // Send finalize request
    QJsonObject data;
    data["action"] = "end_update";
    
    QByteArray jsonData = QJsonDocument(data).toJson(QJsonDocument::Compact);
    
    if (!sendRequest(createRequest("update", jsonData))) {
        emit logMessage(3, "Failed to send update finalize request");
        return false;
    }
    
    return true;
}

bool NetworkDevice::cancelUpdate()
{
    if (!isConnected()) {
        return false;
    }
    
    emit logMessage(1, "Canceling firmware update...");
    
    // Send cancel request
    QJsonObject data;
    data["action"] = "cancel_update";
    
    QByteArray jsonData = QJsonDocument(data).toJson(QJsonDocument::Compact);
    
    if (!sendRequest(createRequest("update", jsonData))) {
        emit logMessage(3, "Failed to send update cancel request");
        return false;
    }
    
    m_state = Idle;
    emit deviceStateChanged(m_state);
    
    return true;
}

qint64 NetworkDevice::optimalChunkSize() const
{
    return DEFAULT_CHUNK_SIZE;
}

void NetworkDevice::onConnected()
{
    m_timeoutTimer.stop();
    
    emit logMessage(1, QString("Connected to device at %1:%2").arg(m_address).arg(m_port));
    
    m_status = Connected;
    emit connectionStatusChanged(m_status);
    
    // Send info request to get device information
    sendRequest(createRequest("info"));
}

void NetworkDevice::onDisconnected()
{
    emit logMessage(1, "Device disconnected");
    
    m_status = Disconnected;
    emit connectionStatusChanged(m_status);
    
    m_buffer.clear();
    m_pendingCommands.clear();
    m_timeoutTimer.stop();
    m_waitingForResponse = false;
}

void NetworkDevice::onError(QAbstractSocket::SocketError error)
{
    QString errorString = m_socket.errorString();
    emit logMessage(3, QString("Socket error: %1").arg(errorString));
    
    m_status = Error;
    emit connectionStatusChanged(m_status);
}

void NetworkDevice::onReadyRead()
{
    // Read available data
    QByteArray data = m_socket.readAll();
    m_buffer.append(data);
    
    // Process complete responses
    processResponse();
}

void NetworkDevice::onTimeout()
{
    emit logMessage(2, "Request timeout");
    
    if (m_waitingForResponse) {
        m_waitingForResponse = false;
        
        // Send next request if any
        if (!m_pendingCommands.isEmpty()) {
            sendNextRequest();
        }
    }
}

void NetworkDevice::processResponse()
{
    // Check if we have a complete response
    // Format: [SIZE:4][JSON_DATA]
    
    while (m_buffer.size() >= 4) {
        // Read message size (first 4 bytes)
        quint32 messageSize = 0;
        QDataStream sizeStream(m_buffer);
        sizeStream.setByteOrder(QDataStream::LittleEndian);
        sizeStream >> messageSize;
        
        // Check if we have enough data to read the complete message
        if (m_buffer.size() < static_cast<int>(4 + messageSize)) {
            // Not enough data yet, wait for more
            return;
        }
        
        // Read the JSON response
        QByteArray jsonData = m_buffer.mid(4, messageSize);
        
        // Remove processed data from buffer
        m_buffer.remove(0, 4 + messageSize);
        
        // Parse the JSON response
        QJsonDocument doc = QJsonDocument::fromJson(jsonData);
        if (doc.isNull() || !doc.isObject()) {
            emit logMessage(3, "Received invalid JSON response");
            continue;
        }
        
        QJsonObject response = doc.object();
        
        // Process the response
        QString status = response["status"].toString();
        
        if (status == "ok") {
            // Request succeeded
            m_timeoutTimer.stop();
            m_waitingForResponse = false;
            
            // Process specific response data
            if (response.contains("info")) {
                QJsonObject info = response["info"].toObject();
                
                // Update device state based on info
                QString state = info["state"].toString();
                if (state == "idle") {
                    m_state = Idle;
                } else if (state == "ready") {
                    m_state = Ready;
                } else if (state == "updating") {
                    m_state = Updating;
                } else if (state == "rebooting") {
                    m_state = Rebooting;
                }
                
                emit deviceStateChanged(m_state);
                
                emit logMessage(1, QString("Device info: %1").arg(QString::fromUtf8(QJsonDocument(info).toJson())));
            } else if (response.contains("update_status")) {
                QJsonObject updateStatus = response["update_status"].toObject();
                
                QString action = updateStatus["action"].toString();
                bool success = updateStatus["success"].toBool();
                
                if (action == "begin_update" && success) {
                    m_state = Updating;
                    emit deviceStateChanged(m_state);
                } else if (action == "end_update" && success) {
                    m_state = Rebooting;
                    emit deviceStateChanged(m_state);
                }
                
                emit logMessage(1, QString("Update status: %1").arg(QString::fromUtf8(QJsonDocument(updateStatus).toJson())));
            }
            
            // Send next request if any
            if (!m_pendingCommands.isEmpty()) {
                sendNextRequest();
            }
        } else {
            // Request failed
            QString error = response["error"].toString();
            emit logMessage(3, QString("Request failed: %1").arg(error));
            
            m_timeoutTimer.stop();
            m_waitingForResponse = false;
            
            // Send next request if any
            if (!m_pendingCommands.isEmpty()) {
                sendNextRequest();
            }
        }
    }
}

QByteArray NetworkDevice::createRequest(const QString &cmd, const QByteArray &data)
{
    // Request format: [SIZE:4][JSON_HEADER][DATA]
    QJsonObject header;
    header["command"] = cmd;
    
    if (!data.isEmpty()) {
        header["data_size"] = data.size();
    }
    
    QByteArray headerJson = QJsonDocument(header).toJson(QJsonDocument::Compact);
    
    // Calculate total message size (header + data)
    quint32 messageSize = headerJson.size() + data.size();
    
    // Create result buffer
    QByteArray result;
    QDataStream stream(&result, QIODevice::WriteOnly);
    stream.setByteOrder(QDataStream::LittleEndian);
    
    // Write size and header
    stream << messageSize;
    result.append(headerJson);
    
    // Append data if any
    if (!data.isEmpty()) {
        result.append(data);
    }
    
    return result;
}

bool NetworkDevice::sendRequest(const QByteArray &req)
{
    if (!isConnected()) {
        return false;
    }
    
    // If already waiting for a response, queue the request
    if (m_waitingForResponse) {
        m_pendingCommands.enqueue(req);
        return true;
    }
    
    // Send request
    qint64 bytesWritten = m_socket.write(req);
    if (bytesWritten != req.size()) {
        emit logMessage(3, "Failed to write data to socket");
        return false;
    }
    
    m_waitingForResponse = true;
    m_timeoutTimer.start(TIMEOUT_MS);
    
    return true;
}

void NetworkDevice::sendNextRequest()
{
    if (m_pendingCommands.isEmpty() || m_waitingForResponse) {
        return;
    }
    
    QByteArray req = m_pendingCommands.dequeue();
    
    // Send request
    qint64 bytesWritten = m_socket.write(req);
    if (bytesWritten != req.size()) {
        emit logMessage(3, "Failed to write data to socket");
        return;
    }
    
    m_waitingForResponse = true;
    m_timeoutTimer.start(TIMEOUT_MS);
} 