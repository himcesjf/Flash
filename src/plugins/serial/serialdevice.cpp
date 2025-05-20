#include "serialdevice.h"

#include <QDebug>
#include <QCoreApplication>
#include <QDateTime>

// Constants
const int TIMEOUT_MS = 3000;
const qint64 DEFAULT_CHUNK_SIZE = 1024;

SerialDevice::SerialDevice(const QString &portName, QObject *parent)
    : DeviceInterface(parent),
      m_portName(portName),
      m_status(Disconnected),
      m_state(Idle),
      m_waitingForAck(false)
{
    // Setup serial port
    m_serialPort.setPortName(portName);
    m_serialPort.setBaudRate(QSerialPort::Baud115200);
    m_serialPort.setDataBits(QSerialPort::Data8);
    m_serialPort.setParity(QSerialPort::NoParity);
    m_serialPort.setStopBits(QSerialPort::OneStop);
    m_serialPort.setFlowControl(QSerialPort::NoFlowControl);
    
    // Connect signals
    QObject::connect(&m_serialPort, &QSerialPort::readyRead,
                     this, &SerialDevice::onReadyRead);
    QObject::connect(&m_serialPort, &QSerialPort::errorOccurred,
                     this, &SerialDevice::onError);
    
    // Setup timeout timer
    m_timeoutTimer.setSingleShot(true);
    QObject::connect(&m_timeoutTimer, &QTimer::timeout,
                     this, &SerialDevice::onTimeout);
}

SerialDevice::~SerialDevice()
{
    disconnect();
}

QString SerialDevice::deviceId() const
{
    return QString("serial:%1").arg(m_portName);
}

QMap<QString, QString> SerialDevice::deviceInfo() const
{
    QMap<QString, QString> info;
    info["type"] = "Serial";
    info["port"] = m_portName;
    info["baudRate"] = QString::number(m_serialPort.baudRate());
    info["status"] = m_serialPort.isOpen() ? "Connected" : "Disconnected";
    return info;
}

bool SerialDevice::connect()
{
    if (m_serialPort.isOpen()) {
        // Already connected
        return true;
    }
    
    emit logMessage(1, QString("Connecting to serial port %1...").arg(m_portName));
    
    m_status = Connecting;
    emit connectionStatusChanged(m_status);
    
    if (m_serialPort.open(QIODevice::ReadWrite)) {
        emit logMessage(1, "Connected to serial device");
        m_status = Connected;
        emit connectionStatusChanged(m_status);
        
        // Send initial handshake
        sendCommand(createCommand("INFO"));
        return true;
    } else {
        emit logMessage(3, QString("Failed to open serial port: %1").arg(m_serialPort.errorString()));
        m_status = Error;
        emit connectionStatusChanged(m_status);
        return false;
    }
}

void SerialDevice::disconnect()
{
    if (m_serialPort.isOpen()) {
        m_serialPort.close();
    }
    
    m_buffer.clear();
    m_pendingCommands.clear();
    m_timeoutTimer.stop();
    m_waitingForAck = false;
    
    m_status = Disconnected;
    emit connectionStatusChanged(m_status);
    
    emit logMessage(1, "Disconnected from serial device");
}

bool SerialDevice::isConnected() const
{
    return m_serialPort.isOpen();
}

DeviceInterface::ConnectionStatus SerialDevice::connectionStatus() const
{
    return m_status;
}

DeviceInterface::DeviceState SerialDevice::deviceState() const
{
    return m_state;
}

bool SerialDevice::beginUpdate()
{
    if (!isConnected()) {
        emit logMessage(3, "Cannot begin update: device not connected");
        return false;
    }
    
    emit logMessage(1, "Beginning firmware update...");
    
    // Send update start command
    if (!sendCommand(createCommand("UPDATE_BEGIN"))) {
        emit logMessage(3, "Failed to send update begin command");
        return false;
    }
    
    return true;
}

bool SerialDevice::sendFirmwareChunk(const QByteArray &data, qint64 offset)
{
    if (!isConnected() || m_state != Updating) {
        emit logMessage(3, "Cannot send firmware: device not in update mode");
        return false;
    }
    
    // Create chunk command with offset and data
    QByteArray offsetBytes;
    offsetBytes.resize(4);
    for (int i = 0; i < 4; ++i) {
        offsetBytes[i] = (offset >> (i * 8)) & 0xFF;
    }
    
    QByteArray cmd = createCommand("CHUNK", offsetBytes + data);
    
    if (!sendCommand(cmd)) {
        emit logMessage(3, QString("Failed to send firmware chunk at offset %1").arg(offset));
        return false;
    }
    
    return true;
}

bool SerialDevice::finalizeUpdate()
{
    if (!isConnected() || m_state != Updating) {
        emit logMessage(3, "Cannot finalize update: device not in update mode");
        return false;
    }
    
    emit logMessage(1, "Finalizing firmware update...");
    
    // Send update end command
    if (!sendCommand(createCommand("UPDATE_END"))) {
        emit logMessage(3, "Failed to send update end command");
        return false;
    }
    
    return true;
}

bool SerialDevice::cancelUpdate()
{
    if (!isConnected()) {
        return false;
    }
    
    emit logMessage(1, "Canceling firmware update...");
    
    // Send cancel command
    if (!sendCommand(createCommand("UPDATE_CANCEL"))) {
        emit logMessage(3, "Failed to send update cancel command");
        return false;
    }
    
    m_state = Idle;
    emit deviceStateChanged(m_state);
    
    return true;
}

qint64 SerialDevice::optimalChunkSize() const
{
    return DEFAULT_CHUNK_SIZE;
}

void SerialDevice::onReadyRead()
{
    // Read available data
    QByteArray data = m_serialPort.readAll();
    m_buffer.append(data);
    
    // Process complete responses
    processResponse();
}

void SerialDevice::onError(QSerialPort::SerialPortError error)
{
    if (error == QSerialPort::NoError) {
        return;
    }
    
    QString errorString = m_serialPort.errorString();
    emit logMessage(3, QString("Serial port error: %1").arg(errorString));
    
    if (error != QSerialPort::NotOpenError) {
        m_status = Error;
        emit connectionStatusChanged(m_status);
    }
}

void SerialDevice::onTimeout()
{
    emit logMessage(2, "Command timeout");
    
    if (m_waitingForAck) {
        m_waitingForAck = false;
        
        // Send next command if any
        if (!m_pendingCommands.isEmpty()) {
            sendNextCommand();
        }
    }
}

void SerialDevice::processResponse()
{
    // Process complete responses in buffer
    // Assumes each response ends with '\n'
    
    while (m_buffer.contains('\n')) {
        int newlinePos = m_buffer.indexOf('\n');
        QByteArray line = m_buffer.left(newlinePos).trimmed();
        m_buffer.remove(0, newlinePos + 1);
        
        // Parse response
        emit logMessage(0, QString("Serial response: %1").arg(QString::fromUtf8(line)));
        
        if (line.startsWith("ACK")) {
            // Acknowledge received, send next command
            m_timeoutTimer.stop();
            m_waitingForAck = false;
            
            if (!m_pendingCommands.isEmpty()) {
                sendNextCommand();
            }
        } else if (line.startsWith("INFO:")) {
            // Device info
            QByteArray info = line.mid(5);
            emit logMessage(1, QString("Device info: %1").arg(QString::fromUtf8(info)));
        } else if (line.startsWith("STATE:")) {
            // Device state change
            QByteArray stateStr = line.mid(6);
            
            if (stateStr == "IDLE") {
                m_state = Idle;
            } else if (stateStr == "READY") {
                m_state = Ready;
            } else if (stateStr == "UPDATING") {
                m_state = Updating;
            } else if (stateStr == "REBOOTING") {
                m_state = Rebooting;
            }
            
            emit deviceStateChanged(m_state);
        } else if (line.startsWith("ERROR:")) {
            // Error message
            QByteArray errorMsg = line.mid(6);
            emit logMessage(3, QString("Device error: %1").arg(QString::fromUtf8(errorMsg)));
        }
    }
}

QByteArray SerialDevice::createCommand(const QString &cmd, const QByteArray &data)
{
    // Simple command format: "CMD:data\n"
    QByteArray result = cmd.toUtf8() + ":";
    if (!data.isEmpty()) {
        result += data;
    }
    result += "\n";
    return result;
}

bool SerialDevice::sendCommand(const QByteArray &cmd)
{
    if (!m_serialPort.isOpen()) {
        return false;
    }
    
    // If already waiting for ACK, queue the command
    if (m_waitingForAck) {
        m_pendingCommands.enqueue(cmd);
        return true;
    }
    
    // Send command
    qint64 bytesWritten = m_serialPort.write(cmd);
    if (bytesWritten != cmd.size()) {
        emit logMessage(3, "Failed to write command to serial port");
        return false;
    }
    
    m_waitingForAck = true;
    m_timeoutTimer.start(TIMEOUT_MS);
    
    return true;
}

void SerialDevice::sendNextCommand()
{
    if (m_pendingCommands.isEmpty() || m_waitingForAck) {
        return;
    }
    
    QByteArray cmd = m_pendingCommands.dequeue();
    
    // Send command
    qint64 bytesWritten = m_serialPort.write(cmd);
    if (bytesWritten != cmd.size()) {
        emit logMessage(3, "Failed to write command to serial port");
        return;
    }
    
    m_waitingForAck = true;
    m_timeoutTimer.start(TIMEOUT_MS);
} 