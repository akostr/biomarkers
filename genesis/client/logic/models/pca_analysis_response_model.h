#pragma once
#ifndef PCA_ANALYSIS_RESPONSE_MODEL_H
#define PCA_ANALYSIS_RESPONSE_MODEL_H

#include "logic/models/identity_model.h"

namespace Model
{
  class PcaAnalysisResponseModel final : public IdentityModel
  {
  public:
    PcaAnalysisResponseModel();
    ~PcaAnalysisResponseModel() override = default;

    static inline std::string P_Title = "P title";
    static inline std::string P_Ids = "P ids";
    static inline std::string T_Title = "T title";
    static inline std::string Components = "Components";
    static inline std::string Hotteling = "Hotteling";
    static inline std::string PC9599 = "PC 95 - 99";
    static inline std::string PassportKeys = "passport_keys";
    static inline std::string PassportHeaders = "passport_headerss";
    static inline std::string Autoscale = "Autoscale";
    static inline std::string NormData = "Norm data";
    static inline std::string AnalysisTable = "Analysis table";
  };
}
#endif
