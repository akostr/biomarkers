#include "pca_analysis_request_model_json_serializer.h"

#include "logic/known_json_tag_names.h"
#include "logic/models/pca_analysis_request_model.h"

#include <QJsonArray>

#define _Q(str) QString::fromStdString(str)
#define _S(str) str.toStdString()

namespace Service
{
  QJsonObject PcaAnalysisRequestModelJsonSerializer::ToJson(const IdentityModelPtr model) const
  {
    return
    {
      IntVectorToJsonPair(model, PcaAnalysisRequestModel::SampleIds, _S(JsonTagNames::SamplesIds)),
      IntVectorToJsonPair(model, PcaAnalysisRequestModel::MarkerIds, _S(JsonTagNames::MarkersIds)),
      { JsonTagNames::Autoscale, model->Get<bool>(PcaAnalysisRequestModel::Autoscale) },
      { JsonTagNames::Key, _Q(model->Get<std::string>(PcaAnalysisRequestModel::Key)) },
      { JsonTagNames::Normdata, model->Get<bool>(PcaAnalysisRequestModel::Normdata) },
      { JsonTagNames::TableId, model->Get<int>(PcaAnalysisRequestModel::TableId) },
    };
  }

  IdentityModelPtr PcaAnalysisRequestModelJsonSerializer::ToModel(const QJsonObject& json) const
  {
    auto model = std::make_shared<PcaAnalysisRequestModel>();
    model->Set(PcaAnalysisRequestModel::Autoscale, ParseBool(json, _S(JsonTagNames::Autoscale)));
    model->Set(PcaAnalysisRequestModel::Key, ParseString(json, _S(JsonTagNames::Key)));
    model->Set(PcaAnalysisRequestModel::Normdata, ParseBool(json, _S(JsonTagNames::Normdata)));
    model->Set(PcaAnalysisRequestModel::SampleIds, ParseIntVector(json, _S(JsonTagNames::SamplesIds)));
    model->Set(PcaAnalysisRequestModel::MarkerIds, ParseIntVector(json, _S(JsonTagNames::MarkersIds)));
    model->Set(PcaAnalysisRequestModel::TableId, json.value(JsonTagNames::TableId).toInt());
    return model;
  }
}