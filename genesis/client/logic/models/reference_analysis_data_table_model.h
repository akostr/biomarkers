#pragma once
#ifndef REFERENCE_ANALYSIS_DATA_TABLE_MODEL_H
#define REFERENCE_ANALYSIS_DATA_TABLE_MODEL_H

#include "logic/models/identity_model.h"

namespace Model
{
  class ReferenceAnalysisDataTableModel : public IdentityModel
  {
  public:
    ReferenceAnalysisDataTableModel();

    static inline std::string Title = "Title"; // string
    static inline std::string Comment = "Comment"; // string
    static inline std::string TableType = "Table type"; // string
    static inline std::string RowCount = "Row count"; // int
    static inline std::string SampleHeaders = "Sample headers"; // vector of strings
    static inline std::string SampleData = "Sample data"; // vector of strings
    static inline std::string SampleDataIds = "Sample data ids"; // vector of strings
    static inline std::string PassportHeader = "Passport header"; // vector of strings
    static inline std::string PassportData = "Passport data"; // 2d vector of strings
    static inline std::string ConcentrationHeader = "Concentration header"; // string
    static inline std::string LayerNames = "Layer names"; // vector of strings
    static inline std::string LayerConcentrationData = "Layer concentration data"; // 2d vector of optional double
    static inline std::string ValuesHeightData = "Values height data"; // 2d vector of double
    static inline std::string ValuesAreaData = "Values area data"; // 2d vector of double
    static inline std::string ValuesCovatsData = "Values covats data"; // 2d vector of double
    static inline std::string ValuesRetTimeData = "Values ret time data"; // 2d vector of double
    static inline std::string ValuesCustomData = "Values custom data"; // 2d vector of double
    static inline std::string ValuesHeaders = "Values headers"; // vector of strings
    static inline std::string ValuesIds = "Values ids"; // vector of strings
    static inline std::string ValuesLibraryGroupIds = "Values library group id"; // vector of strings
    static inline std::string ValuesLibraryElementIds = "Values library group id"; // vector of strings
  };
}

#endif
