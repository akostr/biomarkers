#pragma once

#include <QByteArray>

#include "Common.h"

DefineClassS(Rsa);

class Jwt
{
public:
  static QByteArray Create(const QByteArray& email, const QByteArray& scope, RsaS& privateKey, QString& errorText);
  static QByteArray CreateRaw(const QByteArray& header, const QByteArray& payload, RsaS& privateKey, QString& errorText);
};
