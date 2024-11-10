#pragma once
#ifndef PLS_ANALYSIS_DENTITY_MODEL_H
#define PLS_ANALYSIS_DENTITY_MODEL_H

#include "analysis_identity_model.h"

namespace Model
{
  class PlsAnalysisIdentityModel : public AnalysisIdentityModel
  {
  public:
    PlsAnalysisIdentityModel();

    static inline std::string DefaultNumberPC = "Default Number PC";
    static inline std::string PlsFitPickle = "Pls fit pickle";
    static inline std::string PlsParameters = "Pls parameters";
  };
}

#endif