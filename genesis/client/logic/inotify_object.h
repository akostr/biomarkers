#pragma once
#ifndef INOTIFY_OBJECT_H
#define INOTIFY_OBJECT_H

#include <QObject>
#include <QString>
#include <QVariant>
#include <QPointer>

#include "istorage.h"

namespace Core
{
  class INotifyObject : public QObject
  {
    Q_OBJECT

  public:
    INotifyObject(QObject* parent) : QObject(parent) {}

    virtual void NotifyChanged(const QString& dataId, const QVariant& data) = 0;
    virtual void NotifyReset() = 0;

  signals:
    void Changed(IStorage* storage, const QString& dataId, const QVariant& data);
    void Reset(IStorage* storage);
  };
  using INotifyObjectPtr = QPointer<INotifyObject>;
}
#endif