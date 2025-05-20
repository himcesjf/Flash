#include "cryptoutils.h"

#include <QCryptographicHash>
#include <QDebug>

QString CryptoUtils::calculateSHA256(const QByteArray &data)
{
    QCryptographicHash hash(QCryptographicHash::Sha256);
    hash.addData(data);
    return hash.result().toHex();
}

bool CryptoUtils::verifySignature(const QByteArray &data, const QString &signature, const QString &publicKey)
{
    // TODO: Implement proper signature verification
    // - Use OpenSSL's EVP_Verify* functions for RSA/ECDSA verification
    // - Convert signature from hex string to binary
    // - Load public key from PEM format
    // - Verify signature against data hash
    // - Return false if verification fails or on any error
    
    qDebug() << "Signature verification not implemented";
    return true;
} 