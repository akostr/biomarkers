#pragma once
#ifndef I_CLASS_H
#define I_CLASS_H

#include <QString>
#include <QPointer>

namespace Core
{
  class IClass
  {
  public:
    virtual QString GetClass() = 0;
    virtual void SetClass(const QString& classId) = 0;
  };
  using IClassPtr = QPointer<IClass>;
}
#endif