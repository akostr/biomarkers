#pragma once
#ifndef PLS_PREDICT_RESPONSE_MODEL_H
#define PLS_PREDICT_RESPONSE_MODEL_H

#include "identity_model.h"

namespace Model
{
  class PlsPredictResponseModel final : public IdentityModel
  {
  public:
    PlsPredictResponseModel();
    ~PlsPredictResponseModel() = default;

    static inline std::string PredictData = "Predict data";
    static inline std::string PTitle = "P Title";
    static inline std::string TTitle = "T Title";
    static inline std::string AnalysisParams = "Analysis params";
    static inline std::string AnalysisTable = "Analysis table";
    static inline std::string ChoosenNumberPC = "Choosen number pc";
    static inline std::string MaxNumberPC = "Max number pc";
    static inline std::string YPredConcentration = "Y pred concentration";
  };
  using PlsPredictResponseModelPtr = std::shared_ptr<PlsPredictResponseModel>;
}
#endif
