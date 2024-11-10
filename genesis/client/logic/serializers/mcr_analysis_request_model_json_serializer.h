#pragma once
#ifndef MCR_ANALYSIS_REQUEST_MODEL_JSON_SERIALIZER_H
#define MCR_ANALYSIS_REQUEST_MODEL_JSON_SERIALIZER_H

#include "model_base_json_serializer.h"

namespace Service
{
  class McrAnalysisRequestModelJsonSerializer : public ModelBaseJsonSerializer
  {
  public:
    QJsonObject ToJson(const IdentityModelPtr model) const override;
    IdentityModelPtr ToModel(const QJsonObject& json) const override;

  protected:
    ReferenceSettingsList ParseReferenceSettingsList(const QJsonArray& list) const;
    QJsonArray ReferenceSettingsListToJsonArray(const IdentityModelPtr model, std::string& tag) const;
  };
}
#endif