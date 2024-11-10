#pragma once
#ifndef MCR_ANALYSIS_REQUEST_MODEL_H
#define MCR_ANALYSIS_REQUEST_MODEL_H

#include "identity_model.h"

namespace Model
{
  class McrAnalysisRequestModel : public IdentityModel
  {
  public:
    McrAnalysisRequestModel();

    static inline std::string Key = "Key";
    static inline std::string NumberComponents = "Number components";
    static inline std::string ClosedSystem = "Closed system";
    static inline std::string Normdata = "Norm data";
    static inline std::string MaxIter = "Maximum iteration";
    static inline std::string TolErrorChange = "Tolerance error change";
    static inline std::string Method = "Method";
    static inline std::string ReferenceSettings = "Reference settings";
    static inline std::string MarkerIds = "Marker ids";
    static inline std::string SampleIds = "Sample ids";
    static inline std::string TableId = "Table id";
  };
  using McrAnalysisRequestModelPtr = std::shared_ptr<McrAnalysisRequestModel>;
}

#endif