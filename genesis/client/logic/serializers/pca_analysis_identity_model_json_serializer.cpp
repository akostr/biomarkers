#include "pca_analysis_identity_model_json_serializer.h"

#include "logic/models/pca_analysis_identity_model.h"
#include "logic/known_json_tag_names.h"

#include <QJsonArray>

using namespace Model;

namespace Service
{
  QJsonObject PcaAnalysisIdentitytModelJsonSerializer::ToJson(const IdentityModelPtr model) const
  {
    auto analysisJson = AnalysisIdentityModelJsonSerializer::ToJson(model);
    auto parameterObject = analysisJson.value(JsonTagNames::Parameters).toObject();
    parameterObject.insert(JsonTagNames::Autoscale,
      model->Get<bool>(PcaAnalysisIdentityModel::Autoscale));
    parameterObject.insert(JsonTagNames::Normdata,
      model->Get<bool>(PcaAnalysisIdentityModel::Normalization));
    parameterObject.insert(JsonTagNames::PC95_99,
      model->Get<bool>(PcaAnalysisIdentityModel::PC9599));
    parameterObject.insert(JsonTagNames::PassportKeys, QJsonArray::fromStringList(model->Get<QStringList>(PcaAnalysisIdentityModel::PassportKeys)));
    parameterObject.insert(JsonTagNames::PassportHeaders, QJsonArray::fromStringList(model->Get<QStringList>(PcaAnalysisIdentityModel::PassportHeaders)));
    analysisJson[JsonTagNames::Parameters] = parameterObject;
    auto analysisData = analysisJson.value(JsonTagNames::AnalysisData).toObject();
    model->Exec<QJsonObject>(PcaAnalysisIdentityModel::Hotteling,
      [&](const QJsonObject& obj) { analysisData.insert(JsonTagNames::HottelingTable, obj); });
    analysisJson.insert(JsonTagNames::AnalysisData, analysisData);
    return analysisJson;
  }

  IdentityModelPtr PcaAnalysisIdentitytModelJsonSerializer::ToModel(const QJsonObject& json) const
  {
    auto model = AnalysisIdentityModelJsonSerializer::ToModel(json);
    model->Set(PcaAnalysisIdentityModel::Hotteling,
      ParseSubObject(
        ParseSubObject(json, JsonTagNames::AnalysisData.toStdString())
        , JsonTagNames::HottelingTable.toStdString())
    );
    const auto params = ParseSubObject(json, JsonTagNames::Parameters.toStdString());
    model->Set(PcaAnalysisIdentityModel::Normalization, ParseBool(params, JsonTagNames::Normdata.toStdString()));
    model->Set(PcaAnalysisIdentityModel::Autoscale, ParseBool(params, JsonTagNames::Autoscale.toStdString()));
    model->Set(PcaAnalysisIdentityModel::PC9599, ParseBool(params, JsonTagNames::PC95_99.toStdString()));
    QStringList pk, ph;
    auto jpk = params[JsonTagNames::PassportKeys].toArray();
    auto jph = params[JsonTagNames::PassportHeaders].toArray();
    for(int i = 0; i < jpk.size(); i++)
      pk << jpk[i].toString();
    for(int i = 0; i < jph.size(); i++)
      ph << jph[i].toString();
    model->Set(PcaAnalysisIdentityModel::PassportKeys, pk);
    model->Set(PcaAnalysisIdentityModel::PassportHeaders, ph);
    return model;
  }
}
