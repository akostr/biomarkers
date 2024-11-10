#include <QtGlobal>
#include <QFile>

#include "InnerCrypt.h"
#include "Rsa.h"

QByteArray InnerCrypt::Encrypt(const QByteArray& decData)
{
  return mRsa->EncryptPub(decData, mErrorText);
}

QByteArray InnerCrypt::Decrypt(const QByteArray& encData)
{
  return mRsa->DecryptPriv(encData, mErrorText);
}

QByteArray InnerCrypt::Sign(const QByteArray& data)
{
  return mRsa->SignSha256(data, mErrorText);
}

bool InnerCrypt::Verify(const QByteArray& data, const QByteArray& sign)
{
  return mRsa->VerifySha256(data, sign, mErrorText);
}


InnerCrypt::InnerCrypt()
{
  Q_INIT_RESOURCE(InnerData);

  QFile sk(":/Inner.txt");
  if (sk.open(QFile::ReadOnly)) {
    QByteArray data = sk.readAll();
    if (!data.isEmpty()) {
      data = data.replace("\\n", "\n").replace("\\u003d", "=");
      mRsa = Rsa::FromBio(data, mErrorText);
    } else {
      mErrorText = QString("Read SK: ") + sk.errorString();
    }
  } else {
    mErrorText = QString("Open SK: ") + sk.errorString();
  }
}
