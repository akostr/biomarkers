#include <openssl/rsa.h>
#include <openssl/sha.h>
#include <openssl/bio.h>
#include <openssl/objects.h>
#include <openssl/pem.h>
#include <openssl/err.h>

#include "Rsa.h"


//RsaS Rsa::FromTlv(const TlvS& tlv, QString& errorText)
//{
//  int index = 1;
//  QByteArray modulus = tlv->Childs[index++]->Value;
//  QByteArray exponent = tlv->Childs[index++]->Value;
//  QByteArray d = tlv->Childs[index++]->Value;
//  QByteArray p = tlv->Childs[index++]->Value;
//  QByteArray q = tlv->Childs[index++]->Value;
//  QByteArray dp = tlv->Childs[index++]->Value;
//  QByteArray dq = tlv->Childs[index++]->Value;
//  QByteArray incerseQ = tlv->Childs[index++]->Value;
//}

RsaS Rsa::FromBio(const QByteArray& bioText, QString& errorText)
{
  bio_stS bio(BIO_new(BIO_s_mem()), BIO_free_all);
  if (BIO_write(bio.data(), bioText.constData(), bioText.size()) <= 0) {
    errorText = "RSA key write to BIO fail";
    return RsaS();
  }
  RSA* rsaPtr = nullptr;
  if (!PEM_read_bio_RSAPrivateKey(bio.data(), &rsaPtr, nullptr, nullptr)) {
    errorText = "RSA key read from BIO fail";
    return RsaS();
  }
  RsaS rsa(new Rsa());
  rsa->mRsa.reset(rsaPtr, RSA_free);
  return rsa;
}

QByteArray Rsa::EncryptPub(const QByteArray& data, QString& errorText)
{
  unsigned char buffer[4098];
  int size = RSA_public_encrypt(data.size(), (const unsigned char*)data.constData()
                                , buffer, mRsa.data(), RSA_PKCS1_PADDING);
  if (size > 0) {
    return QByteArray((const char*)buffer, size);
  } else {
    errorText = QString("Encrypt failed");
    return QByteArray();
  }
}

QByteArray Rsa::DecryptPriv(const QByteArray& data, QString& errorText)
{
  unsigned char buffer[4098];
  int size = RSA_private_decrypt(data.size(), (const unsigned char*)data.constData()
                                 , buffer, mRsa.data(), RSA_PKCS1_PADDING);
  if (size > 0) {
    return QByteArray((const char*)buffer, size);
  } else {
    errorText = QString("Decrypt failed");
    return QByteArray();
  }
}

QByteArray Rsa::SignSha256(const QByteArray& data, QString& errorText)
{
  if (!mRsa) {
    errorText = "RSA sign fail: PK not initialized";
    return QByteArray();
  }
  uchar hash[SHA256_DIGEST_LENGTH];
  if (!SHA256(reinterpret_cast<const uchar*>(data.constBegin()), data.size(), hash)) {
    errorText = "SHA256 calc fail";
    return QByteArray();
  }
  QByteArray sign;
  uint size = RSA_size(mRsa.data());
  sign.resize(size);
  if (RSA_sign(NID_sha256, hash, SHA256_DIGEST_LENGTH, reinterpret_cast<uchar*>(sign.data()), &size, mRsa.data()) <= 0) {
    errorText = "RSA sign generate fail";
    return QByteArray();
  }
  return sign;
}

bool Rsa::VerifySha256(const QByteArray& data, const QByteArray& sign, QString& errorText)
{
  if (!mRsa) {
    errorText = "RSA sign fail: PK not initialized";
    return false;
  }
  uchar hash[SHA256_DIGEST_LENGTH];
  if (!SHA256(reinterpret_cast<const uchar*>(data.constBegin()), data.size(), hash)) {
    errorText = "SHA256 calc fail";
    return false;
  }
  if (RSA_verify(NID_sha256, hash, SHA256_DIGEST_LENGTH, reinterpret_cast<const uchar*>(sign.data()), (unsigned int)sign.size(), mRsa.data()) <= 0) {
    errorText = "RSA verify fail";
    return false;
  }
  return true;
}


Rsa::Rsa()
{
}
