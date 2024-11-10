#pragma once
#ifndef PLS_FIT_REQUEST_MODEL_H
#define PLS_FIT_REQUEST_MODEL_H

#include "identity_model.h"

namespace Model
{
  class PlsFitRequestModel final : public IdentityModel
  {
  public:
    PlsFitRequestModel();
    ~PlsFitRequestModel() = default;

    static inline std::string Autoscale = "Autoscale";
    static inline std::string Key = "Key";
    static inline std::string Normdata = "Normdata";
    static inline std::string NMaxLV = "NMaxLV";
    static inline std::string Predict = "Predict";
    static inline std::string CVCount = "cv";
    static inline std::string ParentId = "Parent id";
    static inline std::string SampleIds = "Sample ids";
    static inline std::string SampleConcentrations = "Sample concentrations";
    static inline std::string MarkerIds = "Marker ids";
    static inline std::string PossibleTestSampleIds = "Possible test samples";
    static inline std::string PossibleTestSampleConcentrations = "Possible test sample concentrations";
    static inline std::string TestSampleIds = "Test sample ids";
    static inline std::string TestSampleConcentrations = "Test samples concentrations";
    static inline std::string TableId = "Table id";
    static inline std::string LayerName = "Layer name";
  };
}
#endif