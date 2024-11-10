#include "mcr_analysis_identity_model.h"

namespace Model
{
  McrAnalysisIdentityModel::McrAnalysisIdentityModel()
    : AnalysisIdentityModel("MCR analysis identity model")
  {
  }

  McrAnalysisIdentityModel::McrAnalysisIdentityModel(std::string&& name)
    : AnalysisIdentityModel(std::forward<std::string>(name))
  {
  }
}