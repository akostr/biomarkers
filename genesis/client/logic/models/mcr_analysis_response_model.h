#pragma once
#ifndef MCR_ANALYSIS_IDENTITY_MODEL_H
#define MCR_ANALYSIS_IDENTITY_MODEL_H

#include "mcr_analysis_identity_model.h"

namespace Model
{
  class McrAnalysisResponseModel : public McrAnalysisIdentityModel
  {
  public:
    McrAnalysisResponseModel();

    static inline std::string TTitle = "T Title";
  };
}
#endif