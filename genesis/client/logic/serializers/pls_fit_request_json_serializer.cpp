#include "pls_fit_request_json_serializer.h"

#include "logic/known_json_tag_names.h"
#include "logic/models/pls_fit_request_model.h"
#include "logic/serializers/json_combiner.h"

#include <QJsonArray>

#define _Q(str) QString::fromStdString(str)
#define _S(str) str.toStdString()

namespace Service
{
  QJsonObject PlsFitRequestJsonSerializer::ToJson(const IdentityModelPtr model) const
  {
    return
    {
      { JsonTagNames::Autoscale, model->Get<bool>(PlsFitRequestModel::Autoscale) },
      { JsonTagNames::Key, _Q(model->Get<std::string>(PlsFitRequestModel::Key)) },
      { JsonTagNames::Normdata, model->Get<bool>(PlsFitRequestModel::Normdata) },
      { JsonTagNames::NMaxLV, static_cast<int>(model->Get<size_t>(PlsFitRequestModel::NMaxLV)) },
      { JsonTagNames::Predict, _Q(model->Get<std::string>(PlsFitRequestModel::Predict)) },
      { JsonTagNames::CV, static_cast<int>(model->Get<size_t>(PlsFitRequestModel::CVCount)) },
      UnsignedIntToJsonPair(model, PlsFitRequestModel::ParentId, _S(JsonTagNames::AnalysisParentId)),
      IntVectorToJsonPair(model, PlsFitRequestModel::SampleIds, _S(JsonTagNames::SamplesIds)),
      DoubleOptionalVectorToJsonPair(model, PlsFitRequestModel::SampleConcentrations, _S(JsonTagNames::SamplesConcentrations)),
      IntVectorToJsonPair(model, PlsFitRequestModel::MarkerIds, _S(JsonTagNames::MarkersIds)),
      IntVectorToJsonPair(model, PlsFitRequestModel::TestSampleIds, _S(JsonTagNames::TestSampleIds)),
      DoubleVectorToJsonPair(model, PlsFitRequestModel::TestSampleConcentrations, _S(JsonTagNames::TestSampleConcentrations)),
      IntVectorToJsonPair(model, PlsFitRequestModel::PossibleTestSampleIds, _S(JsonTagNames::PossibleTestSampleIds)),
      DoubleVectorToJsonPair(model, PlsFitRequestModel::PossibleTestSampleConcentrations, _S(JsonTagNames::PossibleTestSampleConcentrations)),
      IntToJsonPair(model, PlsFitRequestModel::TableId, _S(JsonTagNames::TableId)),
      StringToJsonPair(model, PlsFitRequestModel::LayerName, _S(JsonTagNames::LayerName))
    };
  }

  IdentityModelPtr PlsFitRequestJsonSerializer::ToModel(const QJsonObject& json) const
  {
    auto model = std::make_shared<PlsFitRequestModel>();
    model->Set(PlsFitRequestModel::Autoscale, ParseBool(json, _S(JsonTagNames::Autoscale)));
    model->Set(PlsFitRequestModel::Key, ParseString(json, _S(JsonTagNames::Key)));
    model->Set(PlsFitRequestModel::Normdata, ParseBool(json, _S(JsonTagNames::Normdata)));
    model->Set(PlsFitRequestModel::NMaxLV, ParseUnsignedInt(json, _S(JsonTagNames::NMaxLV)));
    model->Set(PlsFitRequestModel::Predict, ParseString(json, _S(JsonTagNames::Predict)));
    model->Set(PlsFitRequestModel::CVCount, ParseUnsignedInt(json, _S(JsonTagNames::CV)));
    model->Set(PlsFitRequestModel::ParentId, ParseUnsignedInt(json, _S(JsonTagNames::AnalysisParentId)));
    model->Set(PlsFitRequestModel::SampleIds, ParseIntVector(json, _S(JsonTagNames::SamplesIds)));
    model->Set(PlsFitRequestModel::MarkerIds, ParseIntVector(json, _S(JsonTagNames::MarkersIds)));
    model->Set(PlsFitRequestModel::LayerName, ParseString(json, _S(JsonTagNames::LayerName)));
    return model;
  }
}