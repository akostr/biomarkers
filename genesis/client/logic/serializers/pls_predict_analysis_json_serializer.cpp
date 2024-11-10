#include "pls_predict_analysis_json_serializer.h"

#include "logic/serializers/json_combiner.h"
#include "logic/models/pls_predict_response_model.h"
#include "logic/models/pls_predict_analysis_model.h"
#include "logic/models/pls_analysis_identity_model.h"
#include "logic/models/analysis_identity_model.h"
#include "logic/known_json_tag_names.h"

#include <QJsonArray>
#include <QJsonDocument>

namespace Service
{
  QJsonObject PlsPredictAnalysisJsonSerializer::ToJson(const IdentityModelPtr model) const
  {
    auto json = AnalysisIdentityModelJsonSerializer::ToJson(model);
    auto parameterObject = json.value(JsonTagNames::Parameters).toObject();
    model->Exec<PlsPredictParameter>(AnalysisIdentityModel::AnalysisParameters,
      [&](const PlsPredictParameter& params)
      {
        parameterObject.insert(JsonTagNames::AnalysisParams, PlsPredictParamsToJson(params));
      });
    json[JsonTagNames::Parameters] = parameterObject;
    auto analysisData = json.value(JsonTagNames::AnalysisData).toObject();
    analysisData.insert(JsonTagNames::pls_predict,
      PredictAnalysisMetaInfoListToJson(model->Get<PredictAnalysisMetaInfoList>(PlsPredictResponseModel::TTitle)));
    json.insert(JsonTagNames::AnalysisData, analysisData);
    JSonCombiner combiner;
    return combiner.Combine(json,
      { UnsignedIntToJsonPair(model, PlsPredictAnalysisModel::ParentId, JsonTagNames::AnalysisParentId.toStdString()) }
    );
  }

  IdentityModelPtr PlsPredictAnalysisJsonSerializer::ToModel(const QJsonObject& json) const
  {
    const auto model = AnalysisIdentityModelJsonSerializer::ToModel(json);
    model->Set<PredictAnalysisMetaInfoList>(PlsPredictResponseModel::TTitle,
      ParsePredictAnalysisMetaInfoList(json.value(JsonTagNames::AnalysisData).toObject(), JsonTagNames::pls_predict.toStdString()));
    model->Set<size_t>(PlsPredictAnalysisModel::ParentId, ParseUnsignedInt(json, JsonTagNames::AnalysisParentId.toStdString()));
    model->Set<PlsPredictParameter>(AnalysisIdentityModel::AnalysisParameters, ParsePlsPredictParameter(ParseSubObject(json, JsonTagNames::Parameters.toStdString()), JsonTagNames::AnalysisParams.toStdString()));
    return model;
  }
}
