#pragma once

#include <QByteArray>

#include "Common.h"

DefineClassS(InnerCrypt);
DefineClassS(Rsa);

class InnerCrypt
{
  RsaS    mRsa;
  QString mErrorText;

public:
  const QString& ErrorText() { return mErrorText; }

public:
  QByteArray Encrypt(const QByteArray& decData);
  QByteArray Decrypt(const QByteArray& encData);
  QByteArray Sign(const QByteArray& data);
  bool Verify(const QByteArray& data, const QByteArray& sign);

public:
  explicit InnerCrypt();
  virtual ~InnerCrypt() = default;
};
