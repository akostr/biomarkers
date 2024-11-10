#pragma once
#ifndef PLS_ANALYSIS_LIST_MODEL_H
#define PLS_ANALYSIS_LIST_MODEL_H

#include "logic/models/identity_model.h"

namespace Model
{
  class PlsAnalysisListModel : public IdentityModel
  {
  public:
    PlsAnalysisListModel() : IdentityModel("PLS analysis list model") {}
    ~PlsAnalysisListModel() = default;

    static inline std::string PlsModels = "PlsModels";
    static inline std::string PlsPredicts = "PlsPredicts";
  };
}
#endif