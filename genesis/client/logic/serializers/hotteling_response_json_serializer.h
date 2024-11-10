#pragma once
#ifndef HOTTELING_RESPONSE_JSON_SERIALIZER_H
#define HOTTELING_RESPONSE_JSON_SERIALIZER_H

#include "model_base_json_serializer.h"

namespace Service
{
  class HottelingResponseJsonSerializer final : public ModelBaseJsonSerializer
  {
  public:
    QJsonObject ToJson(const IdentityModelPtr model) const override;
    IdentityModelPtr ToModel(const QJsonObject& json) const override;
  };
}
#endif