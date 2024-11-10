#pragma once
#ifndef HOTTELING_REQUEST_JSON_SERIALIZER_H
#define HOTTELING_REQUEST_JSON_SERIALIZER_H

#include "model_base_json_serializer.h"
#include "logic/structures/common_structures.h"

namespace Service
{
  class HottelingRequestJsonSerialzier final : public ModelBaseJsonSerializer
  {
  public:
    QJsonObject ToJson(const IdentityModelPtr model) const override;
    IdentityModelPtr ToModel(const QJsonObject& json) const override;

  private:
    Structures::HottelingData ParseHottelingData(const QJsonObject& json, std::string_view tag) const;
    JsonPair HottelingDataToJson(const IdentityModelPtr model) const;
  };
}
#endif