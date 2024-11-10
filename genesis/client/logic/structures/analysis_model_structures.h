#pragma once
#ifndef ANALYSIS_MODEL_STRUCTURES_H
#define ANALYSIS_MODEL_STRUCTURES_H

#include "common_structures.h"

#include <logic/structures/peak_data_structure.h>

/*!
* This file contain strucutres and aliases
* with user types from common_structures.h
*/

namespace Structures
{
  /*!
  * Struct represent statistic for slope-offset plot widget
  * Statistic RMSE, R2, Slope, Offset data
  */
  struct PlsStatistic
  {
    Statistic RMSE;
    Statistic R2;
    Statistic Slope;
    Statistic Offset;
  };

  /*!
  * Structure for slope-offset plot widget
  *
  * size_t Number
  * PlsStatistic Statistic
  * DoubleVector Y_prov
  * DoubleVector Y_grad
  * DoubleVector Y_loadings
  * DoubleVector Coefficients
  * DoubleVector T
  * DoubleVector P
  */
  struct PlsComponent
  {
    size_t Number;
    PlsStatistic Statistic;
    DoubleVector Y_prov;
    DoubleVector Y_grad;
    DoubleVector Y_loadings;
    DoubleVector Coefficients;
    DoubleVector T;
    DoubleVector P;
  };
  using PlsComponents = std::vector<PlsComponent>;

  /*!
  * Strucutre extend MetaInfo
  *
  * string WellCluster
  * Peaks Peaks
  */
  struct AnalysisMetaInfo : public MetaInfo
  {
    AnalysisMetaInfo() = default;

    AnalysisMetaInfo(MetaInfo&& info)
      : MetaInfo(std::forward<MetaInfo>(info))
    {
    }

    std::optional<double> YConcentration;
  };
  using AnalysisMetaInfoList = std::vector<AnalysisMetaInfo>;
  using AnalysisMetaInfoMapsList = QList<QVariantMap>;
  using AnalysisMetaInfoListByFileId = std::map<std::string, AnalysisMetaInfo>;

  /*!
  * Strucutre extend AnalysisMetaInfo
  *
  * double YPredConcentration
  */
  struct PredictAnalysisMetaInfo : public AnalysisMetaInfo
  {
    PredictAnalysisMetaInfo() = default;

    PredictAnalysisMetaInfo(AnalysisMetaInfo&& info)
      : AnalysisMetaInfo(std::forward<AnalysisMetaInfo>(info))
    {
    }

    std::optional<double> YPredConcentration;
  };
  using PredictAnalysisMetaInfoList = std::vector<PredictAnalysisMetaInfo>;

  /*!
  * Strucutre extend AnalysisMetaInfo
  *
  * MetaAnalysisPeaks Peaks
  */
  struct AnalysisMetaInfoWithPeaks : public AnalysisMetaInfo
  {
    AnalysisMetaInfoWithPeaks() = default;

    AnalysisMetaInfoWithPeaks(AnalysisMetaInfo&& info)
      : AnalysisMetaInfo(std::forward<AnalysisMetaInfo>(info))
    {
    }

    MetaAnalysisPeaks Peaks;
  };
  using AnalysisMetaInfoWithPeaksList = std::vector<AnalysisMetaInfoWithPeaks>;

  /*!
  *
  * PlsComponents Components;
  * DoubleVector VarianceX;
  * DoubleVector VarianceYGrad;
  * DoubleVector VarianceYProv;
  * DoubleVector RMSEGradAll;
  * DoubleVector RMSEProvAll;
  * DoubleVector YxGrad;
  * DoubleVector YxProv;
  * IntVector PossibleTestSamplesConcentrations;
  * IntVector PossibleTestSamplesIds;
  * IntVector TestSamplesConcentrations;
  * IntVector TestSamplesIds;
  * AnalysisMetaInfoList PossibleTTitle;
  */
  struct PlsFitResponse
  {
    PlsComponents Components;
    DoubleVector VarianceX;
    DoubleVector VarianceYGrad;
    DoubleVector VarianceYProv;
    DoubleVector RMSEGradAll;
    DoubleVector RMSEProvAll;
    DoubleVector YxGrad;
    DoubleVector YxProv;
    DoubleVector PossibleTestSamplesConcentrations;
    IntVector PossibleTestSamplesIds;
    DoubleVector TestSamplesConcentrations;
    IntVector TestSamplesIds;
    AnalysisMetaInfoList PossibleTTitle;
  };

  /*!
  * Represent "style" json structure with parameter Id and array of styles
  *
  * size_t ParameterId
  * AnalysisStyles Styles
  */
  struct ObjectStyle
  {
    size_t ParameterId;
    AnalysisStyles Styles;
  };
  using ObjectStyles = std::vector<ObjectStyle>;

  /*!
  * Represent "count_plot_settings" json structure from backend
  *
  * GroupingParams Params
  * ObjectStyles ColorStyles
  * ObjectStyles CustomStyles
  * ObjectStyles ShapeStyles
  */
  struct CountPlotSettings
  {
    GroupingParams Params;
    ObjectStyles ColorStyles;
    ObjectStyles CustomStyles;
    ObjectStyles ShapeStyles;
  };

  /*!
  * Represent json structure from beckand
  * "analysis_data" : "data" - warning no mix up with "analysis_table
  *
  * AnalysisMetaInfoWithPeaksList T_Title
  * AnalysisMetaInfoList TestData - for pls only
  * Analysis Components Components
  * Pids P_Ids
  * StringList P_Title
  * DoubleVector CountsTable
  * DoubleVector InputMatrix
  * DoubleVector LoadsTable
  * DoubleVector NormalizationTable
  * DoubleVector VarianceTable
  */
  struct AnalysisInnerData
  {
    AnalysisMetaInfoList T_Title;
    AnalysisMetaInfoList TestData;
    AnalysisMetaInfoMapsList T_Title_expanded;
    AnalysisComponents Components;
    IntVector P_Id;
    StringList P_Title;
  };

  /*!
  * Represent structure "widget_settings" from backend json data structure
  *
  * PickedComponent Counts
  * PickedComponent Loads
  */
  struct WidgetSetting
  {
    PickedComponent Counts;
    PickedComponent Loads;
  };

  /*!
  * Represent "analysis_data" json structure form backend
  *
  * AnalysisInnerData Data
  * CountPlotSettings PlotSettings
  * WidgetSetting WidgetSettings
  */
  struct AnalysisData
  {
    AnalysisInnerData Data;
    CountPlotSettings PlotSettings;
    WidgetSetting WidgetSettings;
  };

  /*!
  * Represent "checked_values" json data structure from backend
  *
  * DoubleVector MarkersIds
  * DoubleVector SamplesIds
  */
  struct CheckedPeakIds
  {
    IntVector MarkersIds;
    IntVector SamplesIds;
  };

  /*!
  * Represent "peaks" structure from json data structure from backend
  *
  * int Id
  * int DisplayNumber
  * GeoParam Parameters
  * Interval XInterval
  * Interval YInterval
  * string Title
  * string FormFactor
  * Rectangle PeakRectangle
  * PeakType Type
  * Color PeakColor
  */
  struct AnalysisPeak
  {
    int Id;
    int DisplayNumber;
    GeoParams Parameters;
    Interval XInterval;
    Interval YInterval;
    std::string Title;
    std::string FormFactor;
    Rectangle PeakRectangle;
    PeakType Type;
    std::string PeakColor;
  };
  using AnalysisPeaks = std::vector<AnalysisPeak>;

  /*!
  * Represent "analysis_table" json data structure from backend
  *
  * CheckedPeakIds CheckedValues;
  * OptionalDouble2DVector Concentrations;
  */
  struct AnalysisTable
  {
    CheckedPeakIds CheckedValues;
    OptionalDouble2DVector Concentrations;
  };

  /*!
  * Represent row data for height ratio matrix
  *
  * DoubleVector Columns
  * AnalysisMetaInfo Info
  */
  struct HeightRatioMatrixRow
  {
    DoubleVector Columns;
    AnalysisMetaInfo Info;
  };
  using HeightRatioMatrixTable = std::vector<HeightRatioMatrixRow>;
}
#endif
