#pragma once
#ifndef MCR_ANALYSIS_IDENTITY_JSON_SERIALIZER_H
#define MCR_ANALYSIS_IDENTITY_JSON_SERIALIZER_H

#include "mcr_analysis_common_json_serializer.h"

namespace Service
{
  class McrAnalysisIdentityJsonSerializer final : public McrAnalysisCommonJsonSerializer
  {
  public:
    QJsonObject ToJson(const IdentityModelPtr model) const override;
    IdentityModelPtr ToModel(const QJsonObject& json) const override;
  };
}
#endif