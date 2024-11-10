#include "analysis_identity_model.h"

namespace Model
{
  AnalysisIdentityModel::AnalysisIdentityModel()
    : IdentityModel("Analysis model")
  {
  }

  AnalysisIdentityModel::AnalysisIdentityModel(std::string&& name)
    : IdentityModel(std::forward<std::string>(name))
  {
  }
}