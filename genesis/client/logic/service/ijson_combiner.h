#pragma once
#ifndef IJSON_COMBINER_H
#define IJSON_COMBINER_H

#include <qjsonobject>

#include "iservice.h"

namespace Service
{
  class IJsonCombiner : public IService
  {
  public:
    virtual QJsonObject Combine(const QJsonObject& left, const QJsonObject& right) const = 0;
    virtual QJsonObject Combine(const QVector<QJsonObject>& items) const = 0;
  };
}

#endif