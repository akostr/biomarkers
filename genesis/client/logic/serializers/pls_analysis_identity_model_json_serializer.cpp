#include "pls_analysis_identity_model_json_serializer.h"

#include <QJsonArray>
#include <QJsonDocument>

#include "logic/serializers/json_combiner.h"
#include "logic/known_json_tag_names.h"
#include "logic/models/pls_analysis_identity_model.h"

#include <string>
#include <algorithm>

namespace Service
{
  QJsonObject PlsAnalysisIdentityModelJsonSerializer::ToJson(const IdentityModelPtr model) const
  {
    auto analysisJson = AnalysisIdentityModelJsonSerializer::ToJson(model);
    auto analysisData = analysisJson.value(JsonTagNames::AnalysisData).toObject();
    analysisData.insert(JsonTagNames::pls_fit_response, PlsFitResponseToJson(model));
    analysisJson.insert(JsonTagNames::AnalysisData, analysisData);
    JSonCombiner combiner;
    return combiner.Combine(
      {
        analysisJson,
        PlsFitPickleToJson(model),
      });
  }

  IdentityModelPtr PlsAnalysisIdentityModelJsonSerializer::ToModel(const QJsonObject& json) const
  {
    const auto model = AnalysisIdentityModelJsonSerializer::ToModel(json);
    model->Set<ByteArray>(PlsAnalysisIdentityModel::PlsFitPickle, ParseByteData(json, JsonTagNames::PlsFitPickle.toStdString()));
    const auto analysisData = json.value(JsonTagNames::AnalysisData).toObject();
    model->Set<PlsFitResponse>(AnalysisIdentityModel::AnalysisData,
      ParsePlsFitResponse(analysisData));
    return model;
  }

  QJsonObject PlsAnalysisIdentityModelJsonSerializer::ParametersToJson(const IdentityModelPtr model) const
  {
    JSonCombiner combiner;
    return combiner.Combine(
      {
        AnalysisIdentityModelJsonSerializer::ParametersToJson(model),
        PlsParametersToJson(model)
      });
  }

  QJsonObject PlsAnalysisIdentityModelJsonSerializer::PlsParametersToJson(const IdentityModelPtr model) const
  {
    const auto plsParameters = model->Get<PlsParameters>(PlsAnalysisIdentityModel::PlsParameters);
    return
    {
      { JsonTagNames::DefaultNumberPC, static_cast<int>(plsParameters.DefaultNumberPC) },
      { JsonTagNames::NMaxLV, static_cast<int>(plsParameters.nMaxLv) },
      { JsonTagNames::chosen_pc_count, static_cast<int>(plsParameters.ChosenNumberPC) },
      { JsonTagNames::AnalysisSamplesCount, static_cast<int>(plsParameters.SamplesPerCount) },
      { JsonTagNames::Autoscale, plsParameters.Autoscale },
      { JsonTagNames::Normdata, plsParameters.Normdata },
      { JsonTagNames::Predict, QString::fromStdString(plsParameters.Predict) },
      { JsonTagNames::LayerName, QString::fromStdString(plsParameters.LayerName) },
    };
  }

  QJsonObject PlsAnalysisIdentityModelJsonSerializer::PlsFitPickleToJson(const IdentityModelPtr model) const
  {
    return
    {
      ByteDataToJsonPair(model, PlsAnalysisIdentityModel::PlsFitPickle, JsonTagNames::PlsFitPickle.toStdString()),
    };
  }

  QJsonObject PlsAnalysisIdentityModelJsonSerializer::PlsFitResponseToJson(const IdentityModelPtr model) const
  {
    auto fitResponse = model->Get<PlsFitResponse>(AnalysisIdentityModel::AnalysisData);
    return
    {
      { JsonTagNames::Components, ComponetsToJsonArray(fitResponse.Components) },
      { JsonTagNames::VarianceX, DoubleVectorToJsonArray(fitResponse.VarianceX) },
      { JsonTagNames::VarianceYGrad, DoubleVectorToJsonArray(fitResponse.VarianceYGrad) },
      { JsonTagNames::VarianceYProv, DoubleVectorToJsonArray(fitResponse.VarianceYProv) },
      { JsonTagNames::RMSEGradAll, DoubleVectorToJsonArray(fitResponse.RMSEGradAll) },
      { JsonTagNames::RMSEProvAll, DoubleVectorToJsonArray(fitResponse.RMSEProvAll) },
      { JsonTagNames::YxGrad, DoubleVectorToJsonArray(fitResponse.YxGrad) },
      { JsonTagNames::YxProv, DoubleVectorToJsonArray(fitResponse.YxProv) },
      { JsonTagNames::PossibleTestSampleConcentrations, DoubleVectorToJsonArray(fitResponse.PossibleTestSamplesConcentrations) },
      { JsonTagNames::PossibleTestSampleIds, IntVectorToJsonArray(fitResponse.PossibleTestSamplesIds) },
      { JsonTagNames::TestSampleConcentrations, DoubleVectorToJsonArray(fitResponse.TestSamplesConcentrations) },
      { JsonTagNames::TestSampleIds, IntVectorToJsonArray(fitResponse.TestSamplesIds) },
      { JsonTagNames::TTitlePossible, AnalysisMetaInfoListToJson(fitResponse.PossibleTTitle) },
    };
  }

  void PlsAnalysisIdentityModelJsonSerializer::ParseParameters(const QJsonObject& json, const IdentityModelPtr model) const
  {
    AnalysisIdentityModelJsonSerializer::ParseParameters(json, model);
    ParsePlsParameters(json, model);
  }

  void PlsAnalysisIdentityModelJsonSerializer::ParsePlsParameters(const QJsonObject& json, const IdentityModelPtr model) const
  {
    model->Set<PlsParameters>(PlsAnalysisIdentityModel::PlsParameters,
      {
        ParseString(json, JsonTagNames::Predict.toStdString()),
        ParseString(json, JsonTagNames::LayerName.toStdString()),
        ParseUnsignedInt(json, JsonTagNames::DefaultNumberPC.toStdString()),
        ParseUnsignedInt(json, JsonTagNames::chosen_pc_count.toStdString()),
        ParseUnsignedInt(json, JsonTagNames::AnalysisSamplesCount.toStdString()),
        ParseUnsignedInt(json, JsonTagNames::NMaxLV.toStdString()),
        ParseBool(json, JsonTagNames::Autoscale.toStdString()),
        ParseBool(json, JsonTagNames::Normdata.toStdString()),
      });
  }

  PlsFitResponse PlsAnalysisIdentityModelJsonSerializer::ParsePlsFitResponse(const QJsonObject& json) const
  {
    const auto fitResponse = json.value(JsonTagNames::pls_fit_response).toObject();
    return
    {
      ParseComponents(fitResponse),
      ParseDoubleVector(fitResponse, JsonTagNames::VarianceX.toStdString()),
      ParseDoubleVector(fitResponse, JsonTagNames::VarianceYGrad.toStdString()),
      ParseDoubleVector(fitResponse, JsonTagNames::VarianceYProv.toStdString()),
      ParseDoubleVector(fitResponse, JsonTagNames::RMSEGradAll.toStdString()),
      ParseDoubleVector(fitResponse, JsonTagNames::RMSEProvAll.toStdString()),
      ParseDoubleVector(fitResponse, JsonTagNames::YxGrad.toStdString()),
      ParseDoubleVector(fitResponse, JsonTagNames::YxProv.toStdString()),
      ParseDoubleVector(fitResponse, JsonTagNames::PossibleTestSampleConcentrations.toStdString()),
      ParseIntVector(fitResponse, JsonTagNames::PossibleTestSampleIds.toStdString()),
      ParseDoubleVector(fitResponse, JsonTagNames::TestSampleConcentrations.toStdString()),
      ParseIntVector(fitResponse, JsonTagNames::TestSampleIds.toStdString()),
      ParseAnalysisMetaInfoList(fitResponse, JsonTagNames::TTitlePossible.toStdString()),
    };
  }
}