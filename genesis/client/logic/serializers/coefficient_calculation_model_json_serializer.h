#pragma once
#ifndef COEFFICIENT_CALCULATION_MODEL_JSON_SERIALIZER_H
#define COEFFICIENT_CALCULATION_MODEL_JSON_SERIALIZER_H

#include "reference_analysis_data_table_model_json_serializer.h"

namespace Service
{
  class CoefficientCalculationModelJsonSerializer : public ReferenceAnalysisDataTableModelJsonSerializer
  {
  public:
    QJsonObject ToJson(const IdentityModelPtr model) const override;
    IdentityModelPtr ToModel(const QJsonObject& json) const override;
  };
}
#endif