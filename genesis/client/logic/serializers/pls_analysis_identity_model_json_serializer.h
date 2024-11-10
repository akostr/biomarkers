#pragma once
#ifndef PLS_ANALYSIS_IDENTITY_MODEL_JSON_SERIALIZER_H
#define PLS_ANALYSIS_IDENTITY_MODEL_JSON_SERIALIZER_H

#include "analysis_identity_model_json_serializer.h"

namespace Service
{
  class PlsAnalysisIdentityModelJsonSerializer final : public AnalysisIdentityModelJsonSerializer
  {
  public:
    QJsonObject ToJson(const IdentityModelPtr model) const override;
    IdentityModelPtr ToModel(const QJsonObject& json) const override;

  protected:
    QJsonObject ParametersToJson(const IdentityModelPtr model) const override;
    QJsonObject PlsParametersToJson(const IdentityModelPtr model) const;

    QJsonObject PlsFitPickleToJson(const IdentityModelPtr model) const;
    QJsonObject PlsFitResponseToJson(const IdentityModelPtr model) const;

    void ParseParameters(const QJsonObject& json, const IdentityModelPtr model) const override;
    void ParsePlsParameters(const QJsonObject& json, const IdentityModelPtr model) const;

    PlsFitResponse ParsePlsFitResponse(const QJsonObject& json) const;
  };
}

#endif