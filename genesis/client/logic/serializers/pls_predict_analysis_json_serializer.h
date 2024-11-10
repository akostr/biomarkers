#pragma once
#ifndef PLS_PREDICT_ANALYSIS_JSON_SERIALIZER_H
#define PLS_PREDICT_ANALYSIS_JSON_SERIALIZER_H

#include "analysis_identity_model_json_serializer.h"

namespace Service
{
  class PlsPredictAnalysisJsonSerializer final : public AnalysisIdentityModelJsonSerializer
  {
  public:
    QJsonObject ToJson(const IdentityModelPtr model) const override;
    IdentityModelPtr ToModel(const QJsonObject& json) const override;
  };
}

#endif