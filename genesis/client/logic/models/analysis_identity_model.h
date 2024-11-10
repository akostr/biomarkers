#pragma once
#ifndef ANALYSIS_DENTITY_MODEL_H
#define ANALYSIS_DENTITY_MODEL_H

#include "identity_model.h"

namespace Model
{
  class AnalysisIdentityModel : public IdentityModel
  {
    Q_OBJECT
  public:
    AnalysisIdentityModel();
    AnalysisIdentityModel(std::string&& name);
    ~AnalysisIdentityModel() = default;

    static inline std::string AnalysisData = "Analysis data";
    static inline std::string AnalysisTable = "Analysis table";
    static inline std::string AnalysisComment = "Analysis comment";
    static inline std::string AnalysisParameters = "Analysis parameters";
    static inline std::string AnalysisTitle = "Analysis title";
    static inline std::string AnalysisTypeId = "Analysis type id";
    static inline std::string AnalysisUserId = "Analysis user id";
    static inline std::string AnalysisTableId = "Analysis table id";
    static inline std::string AnalysisId = "Analysis id";
    static inline std::string ExportRequestBody = "Export request body";
    static inline std::string EntityModel = "Entity model shared pointer";
  };
}
#endif
