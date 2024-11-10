#pragma once
#ifndef REFERENCE_ANALYSIS_DATA_TABLE_MODEL_JSON_SERIALIZER_H
#define REFERENCE_ANALYSIS_DATA_TABLE_MODEL_JSON_SERIALIZER_H

#include "model_base_json_serializer.h"

namespace Service
{
  class ReferenceAnalysisDataTableModelJsonSerializer : public ModelBaseJsonSerializer
  {
  public:
    virtual QJsonObject ToJson(const IdentityModelPtr model) const override;
    virtual IdentityModelPtr ToModel(const QJsonObject& json) const override;
  };
}

#endif