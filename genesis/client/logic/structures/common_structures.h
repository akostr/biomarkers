#pragma once
#ifndef COMMON_STRUCTURES_H
#define COMMON_STRUCTURES_H

#include <vector>
#include <string>
#include <map>
#include <optional>
#include <QVector>

/*!
* This file contain strucutres and aliases
* with integral types (e.g. double, int, string)
*/

namespace Structures
{
  using DoubleVector = QVector<double>;
  using OptionalDoubleVector = QVector<std::optional<double>>;
  using IntVector = QVector<int>;
  using Double2DVector = std::vector<DoubleVector>;
  using OptionalDouble2DVector = std::vector<OptionalDoubleVector>;
  using StringList = std::vector<std::string>;
  using StringList2D = std::vector<StringList>;
  using ByteArray = std::vector<std::byte>;
  using HottelingData = std::map<std::string, Double2DVector>;

  /*!
  * Represent container of pairs of string id and integer id
  */
  using Pids = std::map<std::string, size_t>;

  /*!
  * Represent container for get parent ids for build pls predict model
  *
  * size_t Id;
  * std::string Title;
  * IntVector CheckedMarkerIds;
  *
  */
  struct ParentAnalysisId
  {
    size_t Id;
    std::string Title;
    std::string LayerName;
    IntVector CheckedMarkerIds;
  };
  using ParentAnalysisIds = std::vector<ParentAnalysisId>;

  /*!
  * double Proverka
  * double Graduirovka
  */
  struct Statistic
  {
    double Proverka;
    double Graduirovka;
  };

  /*!
  * This is alias for structure T title of backend json structure
  *
  * size_t FileId
  * double YConcentration
  * string Date
  * string Field
  * string FileName
  * string Layer
  * string Well
  */
  struct MetaInfo
  {
    size_t FileId;
    size_t SampleId;

    std::string Date;
    std::string Field;
    std::string FileName;
    std::string Layer;
    std::string Well;
    std::string WellCluster;
    std::string Depth;
  };
  using MetaInfoList = std::vector<MetaInfo>;

  /*!
  * This structure represent peak info from backend json structure
  *
  * std::string Title;
  * double Area;
  * double Height;
  * int Id;
  * bool IsMarker;
  */
  struct MetaAnalysisPeak
  {
    std::string Title;
    double Area;
    double Height;
    double Custom;
    int Id;
    bool IsMarker;
  };
  using MetaAnalysisPeaks = std::vector<MetaAnalysisPeak>;

  /*!
  * Represent style json structure
  *
  * size_t Color
  * int Shape
  */
  struct GroupingParams
  {
    size_t Color;
    int Shape;
  };

  /*!
  *  Represent style json structure with ID number
  *
  * size_t ColorId
  * string ParameterValue
  */
  struct AnalysisStyle
  {
    size_t ColorId;
    std::string ParameterValue;
  };
  using AnalysisStyles = std::vector<AnalysisStyle>;

  /*!
  * Represent components from json data structure from backend
  * Reapeat structure from pca_data_strucutre.h Component
  *
  * size_t Number
  * double ExplPCsVariance
  * DoubleVector P
  * DoubleVector T
  */
  struct AnalysisComponent
  {
    size_t Number;
    double ExplPCsVariance;
    DoubleVector P;
    DoubleVector T;
  };
  using AnalysisComponents = std::vector<AnalysisComponent>;

  /*!
  * Store floating values of x, y components for widget settings
  * structure from json backend
  *
  * double XComponentNum
  * double YComponentNum
  */
  struct PickedComponent
  {
    double XComponentNum;
    double YComponentNum;
  };

  /*!
  * Store json value Parameters for analysis model
  * structure from json backend
  *
  * std::string ValueType;
  */
  struct Parameters
  {
    std::string ValueType;
  };

  /*!
  * Store json value Parameters for pls analysis model
  * structure from json backend
  *
  * std::string Predict;
  * size_t DefaultNumberPC;
  * size_t ChosenNumberPC;
  * size_t SamplesPerCount;
  * bool Autoscale;
  * bool Normdata;
  */
  struct PlsParameters
  {
    std::string Predict;
    std::string LayerName;
    size_t DefaultNumberPC;
    size_t ChosenNumberPC;
    size_t SamplesPerCount;
    size_t nMaxLv;
    bool Autoscale;
    bool Normdata;
  };

  /*!
  * Store data for concentration table in pls plots
  *
  * std::string SampleName;
  * double YConcentration;
  * double YxGrad;
  * double YxProv;
  */
  struct ConcentrationTableRow
  {
    std::string SampleName;
    std::optional<double> YConcentration;
    double YxGrad;
    double YxProv;
  };
  using ConcetrantionTable = std::vector<ConcentrationTableRow>;

  /*!
  * Store reference settings in mcr analysis model
  *
  * size_t ReferenceId = 0;
  * bool NonNegativeConcentration;
  * bool NonNegativeSpectrum;
  * bool Fix;
  */
  struct ReferenceSettings
  {
    size_t ReferenceId = -1;
    bool NonNegativeConcentration;
    bool NonNegativeSpectrum;
    bool Fix;
  };
  using ReferenceSettingsList = std::vector<ReferenceSettings>;

  /*!
  * MCR parameters
  *
  * double MSE;
  * double Lof;
  * double Vairance;
  * size_t NIter;
  */
  struct MCRCalculatedParameters
  {
    double MSE;
    double Lof;
    double Vairance;
    size_t NIter;
  };

  /*!
  * MCR Concentration
  *
  * double Value;
  * std::string TableHeader;
  * size_t Id; // component_id
  */
  struct MCRConcentration
  {
    double Value;
    std::string TableHeader;
    size_t Id; // component_id
  };
  using MCRConcentrationList = std::vector<MCRConcentration>;

  /*!
  * MCR Concentration Table item
  *
  * size_t Id; // T_title_id
  * MCRConcentrationList ColumnValues;
  */
  struct MCRConcentrationTableItem
  {
    size_t Id; // T_title_id
    MCRConcentrationList ColumnValues;
  };
  using MCRConcentrationTable = std::vector<MCRConcentrationTableItem>;

  /*!
  * MCR calculation
  *
  * DoubleVector Values;
  * size_t Id;
  * size_t ComponentNumber;
  */
  struct MCRCalculation
  {
    DoubleVector Values;
    size_t Id;
    size_t ComponentNumber;
  };
  using MCRCalculationList = std::vector<MCRCalculation>;

  /*!
  * MCR Parameters
  *
  * std::string Key;
  * std::string Method;
  * double TolErrChange;
  * size_t NumberComponents;
  * size_t MaxIter;
  * ReferenceSettingsList Settings;
  * bool ClosedSystem;
  * bool NormData;
  */
  struct MCRParameters
  {
    std::string Key;
    std::string Method;

    double TolErrChange;

    size_t NumberComponents;
    size_t MaxIter;

    ReferenceSettingsList Settings;

    bool ClosedSystem;
    bool NormData;
  };

  /*!
  * Reference for markup version
  */
  struct Reference
  {
    size_t Id;
    size_t ProjectFileId;
    size_t CreatedMarkupId;
    std::string Title;
    std::string Comment;
    std::string MarkupTitle;
    std::string Changed;
    std::string Author;
  };
  using ReferenceList = std::vector<Reference>;

  struct PlsPredictParameter
  {
    bool autoscale;
    int cv;
    std::string layer_name;
    int nMaxLV;
    bool normdata;
    int parent_analysis_id;
    std::string predict;
    int table_id;
  };
}

#endif
