#ifndef FIRMWAREPACKAGE_H
#define FIRMWAREPACKAGE_H

#include <QString>
#include <QByteArray>
#include <QMap>
#include <QFile>
#include <QTemporaryFile>
#include <memory>

/**
 * @brief The FirmwarePackage class handles firmware file parsing and validation
 */
class FirmwarePackage
{
public:
    /**
     * @brief Constructs a FirmwarePackage from a firmware file
     * @param filePath Path to firmware file
     * @throws std::runtime_error if firmware file is invalid
     */
    explicit FirmwarePackage(const QString &filePath);
    ~FirmwarePackage();

    /**
     * @brief Get firmware metadata
     * @return Map of metadata key-value pairs
     */
    QMap<QString, QString> metadata() const;

    /**
     * @brief Get firmware binary data
     * @return Binary data
     */
    QByteArray data() const;

    /**
     * @brief Get SHA-256 hash of firmware
     * @return Hash as hex string
     */
    QString sha256Hash() const;

    /**
     * @brief Get firmware signature
     * @return Signature as hex string
     */
    QString signature() const;

    /**
     * @brief Verify firmware integrity
     * @return true if firmware is valid
     */
    bool verify() const;

    /**
     * @brief Get firmware file size
     * @return Size in bytes
     */
    qint64 size() const;

    /**
     * @brief Get chunk of firmware data
     * @param offset Starting position
     * @param size Chunk size in bytes
     * @return Data chunk
     */
    QByteArray getChunk(qint64 offset, qint64 size) const;

    /**
     * @brief Get total number of chunks
     * @param chunkSize Size of each chunk
     * @return Number of chunks
     */
    int chunkCount(qint64 chunkSize) const;

private:
    QString m_filePath;
    std::unique_ptr<QFile> m_file;
    QMap<QString, QString> m_metadata;
    QString m_sha256;
    QString m_signature;
    qint64 m_dataOffset;
    qint64 m_dataSize;

    void parseMetadata();
    void calculateHash();
};

#endif // FIRMWAREPACKAGE_H 