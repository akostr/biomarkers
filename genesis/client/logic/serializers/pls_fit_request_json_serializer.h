#pragma once
#ifndef PLS_FIT_REQUEST_JSON_SERIALIZER_H
#define PLS_FIT_REQUEST_JSON_SERIALIZER_H

#include "model_base_json_serializer.h"
#include <logic/structures/analysis_model_structures.h>

namespace Service
{
  using namespace Structures;

  class PlsFitRequestJsonSerializer final : public ModelBaseJsonSerializer
  {
  public:
    virtual QJsonObject ToJson(const IdentityModelPtr model) const;
    virtual IdentityModelPtr ToModel(const QJsonObject& json) const;
  };
}
#endif