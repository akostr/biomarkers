#pragma once
#ifndef PLS_ANALYSIS_LIST_SERIALIZER_H
#define PLS_ANALYSIS_LIST_SERIALIZER_H

#include "model_base_json_serializer.h"
#include <logic/structures/pls_model_structures.h>

namespace Service
{
  using namespace Structures;
  using StringMap = std::map<std::string, int>;

  class PlsAnalysisListSerializer final : public ModelBaseJsonSerializer
  {
  public:
    QJsonObject ToJson(const IdentityModelPtr model) const override;
    IdentityModelPtr ToModel(const QJsonObject& json) const override;

  protected:
    StringMap ParseColumnNames(const QJsonObject& jsonObject) const;

    PlsModel ParsePlsModel(const QJsonArray& json, const StringMap& columnList) const;

    QVariant ParseColumnValues(const QJsonArray& json,
      const StringMap& columnList, const std::string& tag) const;

    QJsonArray PlsModelToJsonArray(const PlsModel& item) const;
    QJsonArray PlsModelColumnList() const;
    QJsonObject ColumnObject(std::string_view tagName, const std::string& type) const;
  };
}
#endif