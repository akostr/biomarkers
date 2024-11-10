#pragma once
#ifndef PLS_FIT_RESPONSE_JSON_SERIALIZER_H
#define PLS_FIT_RESPONSE_JSON_SERIALIZER_H

#include "analysis_identity_model_json_serializer.h"

namespace Service
{
  class PlsFitResponseJsonSerializer final : public AnalysisIdentityModelJsonSerializer
  {
  public:
    virtual QJsonObject ToJson(const IdentityModelPtr model) const;
    virtual IdentityModelPtr ToModel(const QJsonObject& json) const;

  private:

  };
}
#endif