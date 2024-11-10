#pragma once

#include <QByteArray>

#include "Tlv.h"

DefineClassS(Rsa);
DefineStructS(rsa_st);
DefineStructS(bio_st);

class Rsa
{
  rsa_stS mRsa;

public:
  static RsaS FromBio(const QByteArray& bioText, QString& errorText);

  QByteArray EncryptPub(const QByteArray& data, QString& errorText);
  QByteArray DecryptPriv(const QByteArray& data, QString& errorText);
  QByteArray SignSha256(const QByteArray& data, QString& errorText);
  bool VerifySha256(const QByteArray& data, const QByteArray& sign, QString& errorText);

private:
  Rsa();
};
