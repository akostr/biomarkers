#include "reference_analysis_data_table_model_json_serializer.h"

#include "logic/models/reference_analysis_data_table_model.h"
#include "logic/known_json_tag_names.h"

#include <QJsonArray>

using namespace Model;

namespace Service
{
  QJsonObject ReferenceAnalysisDataTableModelJsonSerializer::ToJson(const IdentityModelPtr model) const
  {
    return
    {
      { JsonTagNames::Title, QString::fromStdString(model->Get<std::string>(ReferenceAnalysisDataTableModel::Title)) },
      { JsonTagNames::TableType, QString::fromStdString(model->Get<std::string>(ReferenceAnalysisDataTableModel::TableType)) },
      { JsonTagNames::ConcentrationHeader, QString::fromStdString(model->Get<std::string>(ReferenceAnalysisDataTableModel::ConcentrationHeader)) },
      UnsignedIntToJsonPair(model, ReferenceAnalysisDataTableModel::RowCount, JsonTagNames::RowCount.toStdString()),
      StringListToJsonPair(model, ReferenceAnalysisDataTableModel::SampleHeaders, JsonTagNames::SampleHeaders.toStdString()),
      StringListToJsonPair(model, ReferenceAnalysisDataTableModel::SampleData, JsonTagNames::SampleData.toStdString()),
      StringListToJsonPair(model, ReferenceAnalysisDataTableModel::PassportHeader, JsonTagNames::PassportHeader.toStdString()),
      StringListToJsonPair(model, ReferenceAnalysisDataTableModel::LayerNames, JsonTagNames::LayerNames.toStdString()),
      StringListToJsonPair(model, ReferenceAnalysisDataTableModel::ValuesHeaders, JsonTagNames::ValuesHeaders.toStdString()),
      StringList2DToJsonPair(model, ReferenceAnalysisDataTableModel::PassportData, JsonTagNames::PassportData.toStdString()),
      DoubleOptionalVector2DToJsonPair(model, ReferenceAnalysisDataTableModel::LayerConcentrationData, JsonTagNames::LayerConcentrationData.toStdString()),
      DoubleVector2DToJson(model, ReferenceAnalysisDataTableModel::ValuesHeightData, JsonTagNames::ValuesHeightData.toStdString()),
      DoubleVector2DToJson(model, ReferenceAnalysisDataTableModel::ValuesAreaData, JsonTagNames::ValuesAreaData.toStdString()),
      DoubleVector2DToJson(model, ReferenceAnalysisDataTableModel::ValuesCovatsData, JsonTagNames::ValuesCovatsData.toStdString()),
      DoubleVector2DToJson(model, ReferenceAnalysisDataTableModel::ValuesRetTimeData, JsonTagNames::ValuesRetTimeData.toStdString()),
      DoubleVector2DToJson(model, ReferenceAnalysisDataTableModel::ValuesCustomData, JsonTagNames::ValuesCustomData.toStdString()),
      { JsonTagNames::ValuesIds, IntVectorToJsonArray(model->Get<IntVector>(ReferenceAnalysisDataTableModel::ValuesIds)) },
      { JsonTagNames::SampleDataIds, IntVectorToJsonArray(model->Get<IntVector>(ReferenceAnalysisDataTableModel::SampleDataIds)) },
      { JsonTagNames::ValuesLibraryGroupIds, IntVectorToJsonArray(model->Get<IntVector>(ReferenceAnalysisDataTableModel::ValuesLibraryGroupIds)) },
      { JsonTagNames::ValuesLibraryElementIds, IntVectorToJsonArray(model->Get<IntVector>(ReferenceAnalysisDataTableModel::ValuesLibraryElementIds)) },
    };
  }

  IdentityModelPtr ReferenceAnalysisDataTableModelJsonSerializer::ToModel(const QJsonObject& json) const
  {
    const auto model = std::make_shared<ReferenceAnalysisDataTableModel>();
    model->Set<std::string>(ReferenceAnalysisDataTableModel::Title, ParseString(json, JsonTagNames::Title.toStdString()));
    model->Set<std::string>(ReferenceAnalysisDataTableModel::Comment, ParseString(json, JsonTagNames::Comment.toStdString()));
    model->Set<std::string>(ReferenceAnalysisDataTableModel::TableType, ParseString(json, JsonTagNames::TableType.toStdString()));
    model->Set<std::string>(ReferenceAnalysisDataTableModel::ConcentrationHeader, ParseString(json, JsonTagNames::ConcentrationHeader.toStdString()));
    model->Set<size_t>(ReferenceAnalysisDataTableModel::RowCount, ParseUnsignedInt(json, JsonTagNames::RowCount.toStdString()));
    model->Set<StringList>(ReferenceAnalysisDataTableModel::SampleHeaders, ParseStringList(json, JsonTagNames::SampleHeaders.toStdString()));
    model->Set<StringList>(ReferenceAnalysisDataTableModel::SampleData, ParseStringList(json, JsonTagNames::SampleData.toStdString()));
    model->Set<StringList>(ReferenceAnalysisDataTableModel::LayerNames, ParseStringList(json, JsonTagNames::LayerNames.toStdString()));
    model->Set<StringList>(ReferenceAnalysisDataTableModel::ValuesHeaders, ParseStringList(json, JsonTagNames::ValuesHeaders.toStdString()));
    model->Set<StringList>(ReferenceAnalysisDataTableModel::PassportHeader, ParseStringList(json, JsonTagNames::PassportHeader.toStdString()));
    model->Set<StringList2D>(ReferenceAnalysisDataTableModel::PassportData, ParseStringList2D(json, JsonTagNames::PassportData.toStdString()));
    model->Set<OptionalDouble2DVector>(ReferenceAnalysisDataTableModel::LayerConcentrationData, ParseOptionalDouble2DVector(json, JsonTagNames::LayerConcentrationData.toStdString()));
    model->Set<Double2DVector>(ReferenceAnalysisDataTableModel::ValuesHeightData, ParseDouble2DVector(json, JsonTagNames::ValuesHeightData.toStdString()));
    model->Set<Double2DVector>(ReferenceAnalysisDataTableModel::ValuesAreaData, ParseDouble2DVector(json, JsonTagNames::ValuesAreaData.toStdString()));
    model->Set<Double2DVector>(ReferenceAnalysisDataTableModel::ValuesCovatsData, ParseDouble2DVector(json, JsonTagNames::ValuesCovatsData.toStdString()));
    model->Set<Double2DVector>(ReferenceAnalysisDataTableModel::ValuesRetTimeData, ParseDouble2DVector(json, JsonTagNames::ValuesRetTimeData.toStdString()));
    model->Set<Double2DVector>(ReferenceAnalysisDataTableModel::ValuesCustomData, ParseDouble2DVector(json, JsonTagNames::ValuesCustomData.toStdString()));
    model->Set<IntVector>(ReferenceAnalysisDataTableModel::ValuesIds, ParseIntVector(json, JsonTagNames::ValuesIds.toStdString()));
    model->Set<IntVector>(ReferenceAnalysisDataTableModel::SampleDataIds, ParseIntVector(json, JsonTagNames::SampleDataIds.toStdString()));
    model->Set<IntVector>(ReferenceAnalysisDataTableModel::ValuesLibraryGroupIds, ParseIntVector(json, JsonTagNames::ValuesLibraryGroupIds.toStdString()));
    model->Set<IntVector>(ReferenceAnalysisDataTableModel::ValuesLibraryElementIds, ParseIntVector(json, JsonTagNames::ValuesLibraryElementIds.toStdString()));
    return model;
  }
}
