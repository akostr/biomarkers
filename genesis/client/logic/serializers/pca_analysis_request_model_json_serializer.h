#pragma once
#ifndef PCA_ANALYSIS_REQUEST_MODEL_JSON_SERIALIZER_H
#define PCA_ANALYSIS_REQUEST_MODEL_JSON_SERIALIZER_H

#include "model_base_json_serializer.h"

namespace Service
{
  class PcaAnalysisRequestModelJsonSerializer final : public ModelBaseJsonSerializer
  {
  public:
    QJsonObject ToJson(const IdentityModelPtr model) const override;
    IdentityModelPtr ToModel(const QJsonObject& json) const override;
  };
}

#endif