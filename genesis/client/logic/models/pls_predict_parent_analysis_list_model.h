#pragma once
#ifndef PLS_PREDICT_PARENT_ANALYSIS_LIST_MODEL_H
#define PLS_PREDICT_PARENT_ANALYSIS_LIST_MODEL_H

#include "identity_model.h"

namespace Model
{
  class PlsPredictParentAnalysisListModel final : public IdentityModel
  {
  public:
    PlsPredictParentAnalysisListModel();
    ~PlsPredictParentAnalysisListModel() = default;

    static inline std::string ParentList = "Parent list";
  };
  using PlsPredictParentAnalysisListModelPtr = std::shared_ptr<PlsPredictParentAnalysisListModel>;
}
#endif