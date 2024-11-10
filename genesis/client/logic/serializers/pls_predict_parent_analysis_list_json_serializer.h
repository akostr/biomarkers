#pragma once
#ifndef PLS_PREDICT_PARENT_ANALYSIS_LIST_JSON_SERIALIZER_H
#define PLS_PREDICT_PARENT_ANALYSIS_LIST_JSON_SERIALIZER_H

#include "model_base_json_serializer.h"
#include "logic/structures/common_structures.h"

namespace Service
{
  class PlsPredictParentAnalysisListJsonSerializer final : public ModelBaseJsonSerializer
  {
  public:
    virtual QJsonObject ToJson(const IdentityModelPtr model) const override;
    virtual IdentityModelPtr ToModel(const QJsonObject& json) const override;

  private:
    ParentAnalysisIds ParseParentAnalysisIds(const QJsonObject& json) const;
    ParentAnalysisId ParseParentAnalysisId(const QJsonObject& json) const;

    QJsonArray ParentAnalysisIdsToArray(const IdentityModelPtr model) const;
  };
}

#endif