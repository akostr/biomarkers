#pragma once
#ifndef MARKUP_REFERENCE_MODEL_JSON_SERIALIZER_H
#define MARKUP_REFERENCE_MODEL_JSON_SERIALIZER_H

#include "model_base_json_serializer.h"

#include "logic/structures/common_structures.h"

namespace Service
{
  using StringMap = std::map<std::string, int>;
  class MarkupReferenceModelJsonSerializer : public ModelBaseJsonSerializer
  {
  public:
    QJsonObject ToJson(const IdentityModelPtr model) const override;
    IdentityModelPtr ToModel(const QJsonObject& json) const override;

  protected:
    QJsonArray ReferenceToJsonArray(const Structures::Reference& reference) const;
    StringMap ParseColumnNames(const QJsonObject& jsonObject) const;
    Reference ParseReference(const QJsonArray& json, const StringMap& columnList) const;
    QVariant ParseColumnValues(const QJsonArray& json,
      const StringMap& columnList, const std::string& tag) const;
  };
}
#endif
