#include "firmwarepackage.h"
#include "cryptoutils.h"

#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QCryptographicHash>
#include <stdexcept>

// Magic signature to identify firmware files
static constexpr char FIRMWARE_MAGIC[] = "FLASHUP";

FirmwarePackage::FirmwarePackage(const QString &filePath)
    : m_filePath(filePath),
      m_dataOffset(0),
      m_dataSize(0)
{
    m_file = std::make_unique<QFile>(filePath);
    
    if (!m_file->open(QIODevice::ReadOnly)) {
        throw std::runtime_error(QString("Failed to open firmware file: %1").arg(m_file->errorString()).toStdString());
    }
    
    // Verify magic signature
    QByteArray magic = m_file->read(7);
    if (magic != FIRMWARE_MAGIC) {
        throw std::runtime_error("Invalid firmware file format");
    }
    
    // Read metadata and validate
    parseMetadata();
    calculateHash();
    
    if (!verify()) {
        throw std::runtime_error("Firmware validation failed");
    }
}

FirmwarePackage::~FirmwarePackage()
{
    if (m_file && m_file->isOpen()) {
        m_file->close();
    }
}

QMap<QString, QString> FirmwarePackage::metadata() const
{
    return m_metadata;
}

QByteArray FirmwarePackage::data() const
{
    if (!m_file || !m_file->isOpen()) {
        return QByteArray();
    }
    
    m_file->seek(m_dataOffset);
    return m_file->read(m_dataSize);
}

QString FirmwarePackage::sha256Hash() const
{
    return m_sha256;
}

QString FirmwarePackage::signature() const
{
    return m_signature;
}

bool FirmwarePackage::verify() const
{
    // TODO: Implement full firmware verification
    // 1. Data integrity check (current implementation)
    // 2. Cryptographic signature verification using trusted keys
    // 3. Version compatibility check
    // 4. Device-specific validation
    
    // Current implementation only verifies data integrity via SHA-256 hash
    QByteArray firmware = data();
    QByteArray calculatedHash = QCryptographicHash::hash(firmware, QCryptographicHash::Sha256);
    QString calculatedHashHex = calculatedHash.toHex();
    
    // Check if calculated hash matches the one in metadata
    return (calculatedHashHex == m_sha256);
}

qint64 FirmwarePackage::size() const
{
    return m_dataSize;
}

QByteArray FirmwarePackage::getChunk(qint64 offset, qint64 size) const
{
    if (!m_file || !m_file->isOpen() || offset >= m_dataSize) {
        return QByteArray();
    }
    
    // Adjust size if it would go beyond the end of the data
    if (offset + size > m_dataSize) {
        size = m_dataSize - offset;
    }
    
    m_file->seek(m_dataOffset + offset);
    return m_file->read(size);
}

int FirmwarePackage::chunkCount(qint64 chunkSize) const
{
    if (chunkSize <= 0) {
        return 0;
    }
    
    // Calculate number of chunks (rounding up)
    return static_cast<int>((m_dataSize + chunkSize - 1) / chunkSize);
}

void FirmwarePackage::parseMetadata()
{
    // File format:
    // - 7 bytes: Magic "FLASHUP"
    // - 4 bytes: Metadata size (N)
    // - N bytes: JSON metadata
    // - Rest: Binary firmware data
    
    m_file->seek(7); // Skip magic
    
    // Read metadata size (4 bytes)
    QByteArray sizeData = m_file->read(4);
    if (sizeData.size() != 4) {
        throw std::runtime_error("Invalid firmware file format");
    }
    
    // Convert to integer (little-endian)
    quint32 metadataSize = static_cast<quint32>(sizeData[0]) |
                          (static_cast<quint32>(sizeData[1]) << 8) |
                          (static_cast<quint32>(sizeData[2]) << 16) |
                          (static_cast<quint32>(sizeData[3]) << 24);
    
    // Read metadata JSON
    QByteArray metadataJson = m_file->read(metadataSize);
    if (metadataJson.size() != static_cast<int>(metadataSize)) {
        throw std::runtime_error("Invalid firmware file format");
    }
    
    // Parse JSON
    QJsonDocument doc = QJsonDocument::fromJson(metadataJson);
    if (doc.isNull() || !doc.isObject()) {
        throw std::runtime_error("Invalid metadata format");
    }
    
    QJsonObject obj = doc.object();
    
    // Extract metadata fields
    for (auto it = obj.constBegin(); it != obj.constEnd(); ++it) {
        m_metadata[it.key()] = it.value().toString();
    }
    
    // Required fields
    static const QStringList requiredFields = {
        "name", "version", "target", "timestamp", "sha256"
    };
    
    for (const auto &field : requiredFields) {
        if (!m_metadata.contains(field) || m_metadata[field].isEmpty()) {
            throw std::runtime_error(QString("Missing required metadata field: %1").arg(field).toStdString());
        }
    }
    
    // Store hash and signature
    m_sha256 = m_metadata["sha256"];
    m_signature = m_metadata.value("signature", QString());
    
    // Calculate data offset and size
    m_dataOffset = 7 + 4 + metadataSize;
    m_dataSize = m_file->size() - m_dataOffset;
    
    if (m_dataSize <= 0) {
        throw std::runtime_error("Firmware file contains no data");
    }
}

void FirmwarePackage::calculateHash()
{
    // TODO: Implement full hash verification
    // 1. Calculate SHA-256 hash of firmware data
    // 2. Compare with hash in metadata
    // 3. Add hash verification to update process
    // 4. Implement hash caching for large files
    
    // Current implementation only verifies hash field exists
    if (!m_metadata.contains("sha256") || m_metadata["sha256"].isEmpty()) {
        throw std::runtime_error("Missing SHA-256 hash in firmware metadata");
    }
} 