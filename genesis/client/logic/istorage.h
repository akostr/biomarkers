#pragma once
#ifndef I_STORAGE_H
#define I_STORAGE_H

#include <QPointer>
#include <QString>
#include <QVariant>

namespace Core
{
  class IStorage
  {
  public:
    virtual void SetData(const QVariantMap& data) = 0;
    virtual void SetData(const QString& dataId, const QVariant& data) = 0;
    virtual void SetData(const std::string& dataId, const QVariant& data) {SetData(QString::fromStdString(dataId), data);};

    virtual QVariantMap GetData() const = 0;
    virtual QVariant GetData(const QString& dataId) const = 0;
    virtual void ClearData() = 0;
    virtual void ClearData(const QString& dataId) = 0;
    virtual bool HasData(const QString& dataId) = 0;
  };
  using IStoragePtr = QPointer<IStorage>;
}
#endif
