#pragma once
#ifndef PLS_FIT_RESPONSE_MODEL_H
#define PLS_FIT_RESPONSE_MODEL_H

#include "identity_model.h"

namespace Model
{
  /*
  * Pls analysis model for response
  */
  class PlsFitResponseModel : public IdentityModel
  {
  public:
    PlsFitResponseModel();
    ~PlsFitResponseModel() = default;

    static inline std::string DefaultNumberPC = "Default number PC"; // size_t
    // this value from RMSE plot tab
    static inline std::string ChosenNumberPC = "Chosen number PC"; // size_t
    static inline std::string P_Title = "P Title"; // StringList. like header info
    static inline std::string T_TitleGrad = "T Title grad"; // AnalysisMetaInfoList
    static inline std::string T_TitleProv = "T Title prov"; // AnalysisMetaInfoList
    static inline std::string T_TitlePossible = "T Title possible"; // AnalysisMetaInfoList
    static inline std::string PlsComponents = "Pls components"; // PlsComponents
    static inline std::string BinaryModel = "Binary Model"; // bytearray
    static inline std::string RMSEProvAll = "RMSE prov all"; // Double vector
    static inline std::string RMSEGradAll = "RMSE grad all"; // Double vector
    static inline std::string VarianceX = "Variance X"; // Double vector
    static inline std::string VarianceYGrad = "Variance Y grad"; // Double vector
    static inline std::string VarianceYProv = "Variance Y prov"; // Double vector
    static inline std::string YxGrad = "Y x grad"; // Double vector
    static inline std::string YxProv = "Y x prov"; // Double vector
    static inline std::string P_id = "P_id"; // StringList

    // not stored on backend. initialize it manually
    static inline std::string PlsId = "Pls id"; // size_t
    static inline std::string Autoscale = "Autoscale"; // bool
    static inline std::string Normdata = "Norm data"; // bool
    static inline std::string Predict = "predict"; // string
    static inline std::string SamplesPerCount = "Samples per count"; // size_t
    static inline std::string ExportRequestBody = "Export request body"; // size_t
    static inline std::string PossibleTestSamplesConcentrations = "Possible test samples concentrations";
    static inline std::string PossibleTestSamplesIds= "Possible test samples ids";
    static inline std::string TestSamplesConcentrations = "Test samples concentrations";
    static inline std::string TestSamplesIds = "Test samples ids";
    static inline std::string nMaxLv = "Entered user max count";
    static inline std::string AnalysisTable = "Analysis table";
    static inline std::string LayerName = "Layer name";
  };
  using PlsFitResponseModelPtr = std::shared_ptr<PlsFitResponseModel>;
}
#endif