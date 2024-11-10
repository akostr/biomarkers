#include "pls_predict_response_json_serializer.h"

#include "logic/models/pls_predict_response_model.h"
#include "logic/known_json_tag_names.h"
#include "logic/serializers/json_combiner.h"
#include "logic/models/analysis_identity_model.h"

#include <QJsonArray>
#include <QJsonDocument>

namespace Service
{
  QJsonObject PlsPredictResponseJsonSerializer::ToJson(const IdentityModelPtr model) const
  {
    QJsonObject result;
    model->Exec<QByteArray>(AnalysisIdentityModel::ExportRequestBody,
      [&](const QByteArray& data) { result.insert(JsonTagNames::ExportRequestBody, QJsonDocument::fromJson(data).object()); });
    model->Exec<DoubleVector>(PlsPredictResponseModel::PTitle,
      [&](const DoubleVector& vec) { result.insert(JsonTagNames::PTitle, DoubleVectorToJsonArray(vec)); });
    model->Exec<PredictAnalysisMetaInfoList>(PlsPredictResponseModel::PTitle,
      [&](const PredictAnalysisMetaInfoList& data) { result.insert(JsonTagNames::T_Title, PredictAnalysisMetaInfoListToJson(data)); });
    model->Exec<PlsPredictParameter>(PlsPredictResponseModel::AnalysisParams,
      [&](const PlsPredictParameter& params) { result.insert(JsonTagNames::AnalysisParams, PlsPredictParamsToJson(params)); });
    model->Exec<CheckedPeakIds>(PlsPredictResponseModel::AnalysisTable,
      [&](const CheckedPeakIds& peaks) { result.insert(JsonTagNames::AnalysisTable, CheckedPeaksIdsToJson(peaks)); });
    model->Exec<int>(PlsPredictResponseModel::ChoosenNumberPC,
      [&](int value) { result.insert(JsonTagNames::ChoosenNumberPC, value); });
    model->Exec<int>(PlsPredictResponseModel::MaxNumberPC,
      [&](int value) { result.insert(JsonTagNames::MaxNumberPC, value); });
    model->Exec<DoubleVector>(PlsPredictResponseModel::YPredConcentration,
      [&](const DoubleVector& data) { result.insert(JsonTagNames::YPredConcentration, DoubleVectorToJsonArray(data)); });
    return result;
  }

  IdentityModelPtr PlsPredictResponseJsonSerializer::ToModel(const QJsonObject& json) const
  {
    const auto model = std::make_shared<PlsPredictResponseModel>();
    model->Set(AnalysisIdentityModel::ExportRequestBody,
      QJsonDocument(json.value(JsonTagNames::ExportRequestBody).toObject()).toJson(QJsonDocument::Compact));
    model->Set(PlsPredictResponseModel::PTitle, ParseDoubleVector(json, JsonTagNames::PTitle.toStdString()));
    model->Set(PlsPredictResponseModel::TTitle, ParsePredictAnalysisMetaInfoList(json, JsonTagNames::T_Title.toStdString()));
    model->Set(PlsPredictResponseModel::AnalysisParams, ParsePlsPredictParameter(json, JsonTagNames::AnalysisParams.toStdString()));
    model->Set(PlsPredictResponseModel::AnalysisTable, ParseCheckedPeaksId(ParseSubObject(json, JsonTagNames::AnalysisTable.toStdString())));
    model->Set(PlsPredictResponseModel::ChoosenNumberPC, json.value(JsonTagNames::ChoosenNumberPC).toInt());
    model->Set(PlsPredictResponseModel::MaxNumberPC, json.value(JsonTagNames::MaxNumberPC).toInt());
    model->Set(PlsPredictResponseModel::YPredConcentration, ParseDoubleVector(json, JsonTagNames::YPredConcentration.toStdString()));
    return model;
  }
}