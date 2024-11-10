#pragma once
#ifndef PCA_ANALYSIS_REQUEST_MODEL_H
#define PCA_ANALYSIS_REQUEST_MODEL_H

#include "logic/models/identity_model.h"

namespace Model
{
  class PcaAnalysisRequestModel final : public IdentityModel
  {
  public :
    PcaAnalysisRequestModel();
    ~PcaAnalysisRequestModel() override = default;

    static inline std::string Autoscale = "Autoscale";
    static inline std::string Key = "Key";
    static inline std::string Normdata = "Normdata";
    static inline std::string SampleIds = "Sample ids";
    static inline std::string MarkerIds = "Marker ids";
    static inline std::string TableId = "Table id";
  };
}

#endif