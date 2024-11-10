#pragma once
#ifndef PLS_PREDICT_ANALYSIS_MODEL_H
#define PLS_PREDICT_ANALYSIS_MODEL_H

#include "identity_model.h"

namespace Model
{
  class PlsPredictAnalysisModel final : public IdentityModel
  {
  public:
    PlsPredictAnalysisModel();
    ~PlsPredictAnalysisModel() = default;

    static inline std::string ParentId = "Parent id";
  };
  using PlsPredictAnalysisModelPtr = std::shared_ptr<PlsPredictAnalysisModel>;
}
#endif
