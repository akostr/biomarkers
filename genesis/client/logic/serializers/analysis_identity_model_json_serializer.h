#pragma once
#ifndef ANALYSIS_IDENTITY_MODEL_JSON_SERIALIZER_H
#define ANALYSIS_IDENTITY_MODEL_JSON_SERIALIZER_H

#include "model_base_json_serializer.h"
#include "logic/structures/analysis_model_structures.h"

class AnalysisEntityModel;
namespace Service
{
  using namespace Structures;

  class AnalysisIdentityModelJsonSerializer : public ModelBaseJsonSerializer
  {
  public:
    QJsonObject ToJson(const IdentityModelPtr model) const override;
    IdentityModelPtr ToModel(const QJsonObject& json) const override;

  protected:
    // virtual methods for inherited classes,
    // should call this method bcs here store values for common analysis model
    virtual QJsonObject ParametersToJson(const IdentityModelPtr model) const;

    // Should parse json and set model parameters
    // should call this method bcs here store values for common analysis model
    virtual void ParseParameters(const QJsonObject& json, const IdentityModelPtr model) const;

    // Parse analysis table
    AnalysisTable ParseAnalysisTable(const QJsonObject& json) const;
    CheckedPeakIds ParseCheckedPeaksId(const QJsonObject& json) const;
    AnalysisMetaInfoListByFileId ParseChromatogramData(const QJsonObject& json) const;
    AnalysisPeaks ParseAnalysisPeaks(const QJsonObject& json) const;
    AnalysisPeak ParseAnalysisPeak(const QJsonObject& json) const;

    // Parse analysis data
    AnalysisData ParseAnalysisData(const QJsonObject& json) const;
    AnalysisInnerData ParseAnalysisInnerData(const QJsonObject& json) const;
    CountPlotSettings ParseCountPlotSettings(const QJsonObject& json) const;
    WidgetSetting ParseWidgetSettings(const QJsonObject& json) const;
    PickedComponent ParsePickedComponent(const QJsonObject& json, std::string_view tagName) const;
    AnalysisComponents ParseAnalysisComponents(const QJsonObject& json) const;
    AnalysisComponent ParseAnalysisComponent(const QJsonObject& json) const;
    GroupingParams ParseGroupingParams(const QJsonObject& json) const;
    ObjectStyles ParseStyles(const QJsonObject& json, std::string_view tagName) const;
    ObjectStyle ParseStyle(const QJsonObject& json) const;
    AnalysisStyles ParseAnalysisStyles(const QJsonObject& json, std::string_view tagName) const;
    PredictAnalysisMetaInfoList ParsePredictAnalysisMetaInfoList(const QJsonObject& json, std::string_view tagName) const;
    PredictAnalysisMetaInfo ParsePredictAnalysisMetaInfo(const QJsonObject& json) const;
    PlsPredictParameter ParsePlsPredictParameter(const QJsonObject& json, std::string_view tagName) const;

    // Parse analysis entity model
    QPointer<AnalysisEntityModel> ParseAnalysisEntityModel(const QJsonObject& json) const;

    // from model to json
    QJsonObject AnalysisTableToJson(const AnalysisTable& analysis_table) const;
    QJsonObject CheckedPeaksIdsToJson(const CheckedPeakIds& checked_peaks) const;
    QJsonObject ChromatogrammDataToJson(const AnalysisMetaInfoListByFileId& chromatogramm_data) const;
    QJsonObject ChromatogrammDataToJson(const MetaInfo& chromatogramm_data) const;
    QJsonArray PeaksToJsonArray(const AnalysisPeaks& peaks) const;
    QJsonObject PeakToJson(const AnalysisPeak& peak) const;
    QJsonObject AnalysisDataToJson(const AnalysisData& analysis_data) const;
    QJsonObject CountPlotSettingsToJson(const CountPlotSettings& count_plot_settings) const;
    QJsonObject GroupingParamsToJson(const GroupingParams& grouping_params) const;
    QJsonArray ObjectStylesToJson(const ObjectStyles& styles) const;
    QJsonObject ObjectStyleToJson(const ObjectStyle& style) const;
    QJsonArray AnalysisStylesToJson(const AnalysisStyles& styles) const;
    QJsonObject AnalysisStyleToJson(const AnalysisStyle& style) const;
    QJsonObject AnalysisInnerDataToJson(const AnalysisInnerData& inner_data) const;
    QJsonArray AnalysisComponentsToJson(const AnalysisComponents& comps) const;
    QJsonObject AnalysisComponentToJson(const AnalysisComponent& comp) const;
    QJsonObject WidgetSettginsToJson(const WidgetSetting& w_setting) const;
    QJsonObject PickedComponentToJson(const PickedComponent& comp) const;
    QJsonObject AnalysisEntityModelToJson(QPointer<AnalysisEntityModel> modelPtr) const;
    QJsonObject PlsPredictParamsToJson(const PlsPredictParameter& data) const;
    QJsonObject PredictAnalysisMetaInfoToJson(const PredictAnalysisMetaInfo& data) const;
    QJsonArray PredictAnalysisMetaInfoListToJson(const PredictAnalysisMetaInfoList& dataList) const;
  };
}
#endif
