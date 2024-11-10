#include "coefficient_calculation_model_json_serializer.h"

#include "logic/known_json_tag_names.h"
#include "logic/models/coefficient_calculation_model.h"

#include <QJsonArray>

namespace Service
{
  QJsonObject CoefficientCalculationModelJsonSerializer::ToJson(const IdentityModelPtr model) const
  {
    auto object = ReferenceAnalysisDataTableModelJsonSerializer::ToJson(model);
    object.insert(JsonTagNames::ValuesLibraryElementIds, IntVectorToJsonArray(model->Get<IntVector>(CoefficientCalculationModel::ValuesLibraryElementIds)));
    object.insert(JsonTagNames::ValuesLibraryGroupIds, IntVectorToJsonArray(model->Get<IntVector>(CoefficientCalculationModel::ValuesLibraryGroupIds)));
    object.insert(JsonTagNames::SampleDataFileIds, IntVectorToJsonArray(model->Get<IntVector>(CoefficientCalculationModel::SampleDataFileId)));
    return object;
  }

  IdentityModelPtr CoefficientCalculationModelJsonSerializer::ToModel(const QJsonObject& json) const
  {
    auto model = ReferenceAnalysisDataTableModelJsonSerializer::ToModel(json);
    model->Set<IntVector>(CoefficientCalculationModel::ValuesLibraryElementIds, ParseIntVector(json, JsonTagNames::ValuesLibraryElementIds.toStdString()));
    model->Set<IntVector>(CoefficientCalculationModel::ValuesLibraryGroupIds, ParseIntVector(json, JsonTagNames::ValuesLibraryGroupIds.toStdString()));
    model->Set<IntVector>(CoefficientCalculationModel::SampleDataFileId, ParseIntVector(json, JsonTagNames::SampleDataFileIds.toStdString()));
    return model;
  }
}