#pragma once
#ifndef PLS_PREDICT_RESPONSE_JSON_SERIALIZER_H
#define PLS_PREDICT_RESPONSE_JSON_SERIALIZER_H

#include "analysis_identity_model_json_serializer.h"

namespace Service
{
  class PlsPredictResponseJsonSerializer final : public AnalysisIdentityModelJsonSerializer
  {
  public:
    virtual QJsonObject ToJson(const IdentityModelPtr model) const override;
    virtual IdentityModelPtr ToModel(const QJsonObject& json) const override;
  };
}

#endif