#pragma once
#ifndef PCA_ANALYISIS_IDENTITY_MODEL_H
#define PCA_ANALYISIS_IDENTITY_MODEL_H

#include "analysis_identity_model.h"

namespace Model
{
  class PcaAnalysisIdentityModel : public AnalysisIdentityModel
  {
  public:
    PcaAnalysisIdentityModel();

    static inline std::string Normalization = "Normalization";
    static inline std::string Autoscale = "Autoscale";
    static inline std::string PC9599 = "PC 95-99";
    static inline std::string Hotteling = "Hotteling";
    static inline std::string PassportKeys = "passport_keys";
    static inline std::string PassportHeaders = "passport_headers";
  };
}
#endif
