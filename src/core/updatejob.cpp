#include "updatejob.h"
#include "deviceinterface.h"
#include "firmwarepackage.h"

#include <QDebug>

// Constants
const int DEFAULT_MAX_RETRIES = 3;
const int DEFAULT_RETRY_INTERVAL_MS = 1000;
const int DEFAULT_CHUNK_INTERVAL_MS = 10;

UpdateJob::UpdateJob(std::shared_ptr<DeviceInterface> device, 
                     FirmwarePackage *firmware,
                     QObject *parent)
    : QObject(parent),
      m_device(device),
      m_firmware(firmware),
      m_state(Idle),
      m_progress(0),
      m_currentOffset(0),
      m_retryCount(0),
      m_maxRetries(DEFAULT_MAX_RETRIES),
      m_paused(false)
{
    // Connect device signals
    connect(m_device.get(), &DeviceInterface::connectionStatusChanged,
            this, &UpdateJob::onDeviceConnectionStatusChanged);
    connect(m_device.get(), &DeviceInterface::deviceStateChanged,
            this, &UpdateJob::onDeviceStateChanged);
    connect(m_device.get(), &DeviceInterface::logMessage,
            this, &UpdateJob::logMessage);
    
    // Setup timers
    m_retryTimer.setSingleShot(true);
    connect(&m_retryTimer, &QTimer::timeout,
            this, &UpdateJob::onRetryTimeout);
    
    m_chunkTimer.setSingleShot(true);
    connect(&m_chunkTimer, &QTimer::timeout,
            this, &UpdateJob::onUploadNextChunk);
    
    // Get optimal chunk size from device
    m_chunkSize = m_device->optimalChunkSize();
    if (m_chunkSize <= 0) {
        m_chunkSize = 4096; // Default chunk size
    }
    
    emit logMessage(0, QString("Update job created for device %1").arg(m_device->deviceId()));
}

UpdateJob::~UpdateJob()
{
    if (m_state == Uploading || m_state == Preparing || m_state == Connecting) {
        cancel();
    }
}

void UpdateJob::start()
{
    if (m_state != Idle) {
        emit logMessage(2, "Update already in progress");
        return;
    }
    
    emit logMessage(1, "Starting update...");
    
    setState(Connecting);
    setProgress(0);
    
    // Connect to device
    if (m_device->isConnected()) {
        // Already connected, proceed to prepare
        setState(Preparing);
        if (!m_device->beginUpdate()) {
            failUpdate("Failed to initialize update on device");
        }
    } else {
        // Connect first
        if (!m_device->connect()) {
            failUpdate("Failed to connect to device");
        }
    }
}

void UpdateJob::cancel()
{
    if (m_state == Complete || m_state == Failed || m_state == Canceled) {
        return;
    }
    
    emit logMessage(1, "Canceling update...");
    
    m_retryTimer.stop();
    m_chunkTimer.stop();
    
    if (m_device->isConnected()) {
        m_device->cancelUpdate();
    }
    
    setState(Canceled);
    emit completed(false, "Update canceled");
}

UpdateJob::State UpdateJob::state() const
{
    return m_state;
}

int UpdateJob::progress() const
{
    return m_progress;
}

void UpdateJob::onDeviceConnectionStatusChanged(DeviceInterface::ConnectionStatus status)
{
    emit logMessage(0, QString("Device connection status: %1").arg(status));
    
    if (m_state == Connecting) {
        if (status == DeviceInterface::Connected) {
            // Connected, proceed to prepare
            setState(Preparing);
            if (!m_device->beginUpdate()) {
                failUpdate("Failed to initialize update on device");
            }
        } else if (status == DeviceInterface::Error) {
            // Connection failed
            failUpdate("Failed to connect to device");
        }
    } else if (status == DeviceInterface::Disconnected && 
              (m_state == Uploading || m_state == Preparing || m_state == Finalizing)) {
        // Device disconnected during update
        failUpdate("Device disconnected during update");
    }
}

void UpdateJob::onDeviceStateChanged(DeviceInterface::DeviceState state)
{
    emit logMessage(0, QString("Device state: %1").arg(state));
    
    if (m_state == Preparing && state == DeviceInterface::Ready) {
        // Device is ready to receive firmware
        m_currentOffset = 0;
        startUpload();
    } else if (m_state == Finalizing && state == DeviceInterface::Rebooting) {
        // Device is rebooting, update is complete
        completeUpdate();
    } else if (state == DeviceInterface::Error) {
        // Device reported an error
        failUpdate("Device reported an error");
    }
}

void UpdateJob::onUploadNextChunk()
{
    if (m_state != Uploading || m_paused) {
        return;
    }
    
    // Check if we're done
    if (m_currentOffset >= m_firmware->size()) {
        setState(Finalizing);
        if (!m_device->finalizeUpdate()) {
            failUpdate("Failed to finalize update");
        }
        return;
    }
    
    // Get next chunk
    QByteArray chunk = m_firmware->getChunk(m_currentOffset, m_chunkSize);
    
    // Send chunk to device
    if (m_device->sendFirmwareChunk(chunk, m_currentOffset)) {
        // Chunk sent successfully
        m_currentOffset += chunk.size();
        m_retryCount = 0;
        
        // Update progress
        int progress = static_cast<int>((static_cast<double>(m_currentOffset) / m_firmware->size()) * 100);
        setProgress(progress);
        
        // Schedule next chunk
        m_chunkTimer.start(DEFAULT_CHUNK_INTERVAL_MS);
    } else if (m_retryCount < m_maxRetries) {
        // Failed to send chunk, retry
        m_retryCount++;
        emit logMessage(2, QString("Failed to send chunk, retrying (%1/%2)...").arg(m_retryCount).arg(m_maxRetries));
        m_retryTimer.start(DEFAULT_RETRY_INTERVAL_MS);
    } else {
        // Too many retries, fail
        failUpdate("Failed to send firmware chunk after maximum retries");
    }
}

void UpdateJob::onRetryTimeout()
{
    // Try sending the chunk again
    onUploadNextChunk();
}

void UpdateJob::setState(State state)
{
    if (m_state != state) {
        m_state = state;
        
        QString stateStr;
        switch (state) {
            case Idle: stateStr = "Idle"; break;
            case Connecting: stateStr = "Connecting to device"; break;
            case Preparing: stateStr = "Preparing device"; break;
            case Uploading: stateStr = "Uploading firmware"; break;
            case Finalizing: stateStr = "Finalizing update"; break;
            case Complete: stateStr = "Update complete"; break;
            case Failed: stateStr = "Update failed"; break;
            case Canceled: stateStr = "Update canceled"; break;
        }
        
        emit progressChanged(m_progress, stateStr);
        emit logMessage(1, QString("Update state: %1").arg(stateStr));
    }
}

void UpdateJob::setProgress(int progress)
{
    if (m_progress != progress) {
        m_progress = progress;
        
        QString stateStr;
        switch (m_state) {
            case Idle: stateStr = "Idle"; break;
            case Connecting: stateStr = "Connecting to device"; break;
            case Preparing: stateStr = "Preparing device"; break;
            case Uploading: stateStr = QString("Uploading firmware (%1%)").arg(progress); break;
            case Finalizing: stateStr = "Finalizing update"; break;
            case Complete: stateStr = "Update complete"; break;
            case Failed: stateStr = "Update failed"; break;
            case Canceled: stateStr = "Update canceled"; break;
        }
        
        emit progressChanged(progress, stateStr);
    }
}

void UpdateJob::startUpload()
{
    setState(Uploading);
    setProgress(0);
    emit logMessage(1, "Starting firmware upload...");
    
    // Start sending chunks
    m_currentOffset = 0;
    m_retryCount = 0;
    m_paused = false;
    
    // Schedule first chunk
    m_chunkTimer.start(0);
}

void UpdateJob::failUpdate(const QString &reason)
{
    emit logMessage(3, QString("Update failed: %1").arg(reason));
    setState(Failed);
    emit completed(false, reason);
}

void UpdateJob::completeUpdate()
{
    emit logMessage(1, "Update completed successfully");
    setState(Complete);
    emit completed(true, "Firmware updated successfully");
} 