#pragma once
#ifndef PCA_ANALYSIS_IDENTITY_MODEL_JSON_SERIALIZER_H
#define PCA_ANALYSIS_IDENTITY_MODEL_JSON_SERIALIZER_H

#include "analysis_identity_model_json_serializer.h"

namespace Service
{
  class PcaAnalysisIdentitytModelJsonSerializer final : public AnalysisIdentityModelJsonSerializer
  {
  public:
    QJsonObject ToJson(const IdentityModelPtr model) const override;
    IdentityModelPtr ToModel(const QJsonObject& json) const override;
  };
}
#endif