#ifndef CRYPTOUTILS_H
#define CRYPTOUTILS_H

#include <QString>
#include <QByteArray>

/**
 * @brief The CryptoUtils class provides cryptographic utility functions
 */
class CryptoUtils
{
public:
    /**
     * @brief Calculate SHA-256 hash of data
     * @param data Input data
     * @return Hash as hex string
     */
    static QString calculateSHA256(const QByteArray &data);
    
    /**
     * @brief Verify signature against data using public key
     * @param data Data to verify
     * @param signature Signature as hex string
     * @param publicKey Public key as PEM string
     * @return true if signature is valid
     */
    static bool verifySignature(const QByteArray &data, const QString &signature, const QString &publicKey);
};

#endif // CRYPTOUTILS_H 