#pragma once
#ifndef COEFFICIENT_CALCULATION_MODEL_H
#define COEFFICIENT_CALCULATION_MODEL_H

#include "reference_analysis_data_table_model.h"

namespace Model
{
  class CoefficientCalculationModel final : public ReferenceAnalysisDataTableModel
  {
  public:
    CoefficientCalculationModel() = default;

    static inline std::string ValuesLibraryElementIds = "Values library element ids"; // vector of int
    static inline std::string ValuesLibraryGroupIds = "Values library group ids"; // vector of int
    static inline std::string SampleDataFileId = "Sample data file ids"; // vector of int
  };
}


#endif