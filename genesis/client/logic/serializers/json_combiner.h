#pragma once
#ifndef JSON_COMBINER_H
#define JSON_COMBINER_H

#include "logic/service/ijson_combiner.h"

namespace Service
{
  class JSonCombiner final : public IJsonCombiner
  {
  public:
    QJsonObject Combine(const QJsonObject& left, const QJsonObject& right) const override;
    QJsonObject Combine(const QVector<QJsonObject>& items) const override;
  };
}
#endif