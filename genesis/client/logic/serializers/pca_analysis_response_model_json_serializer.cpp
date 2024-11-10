#include "pca_analysis_response_model_json_serializer.h"

#include "logic/models/pca_analysis_response_model.h"
#include "logic/models/analysis_identity_model.h"
#include "logic/known_json_tag_names.h"

#include <QJsonArray>
#include <QJsonDocument>

using namespace Model;

namespace Service
{
  QJsonObject PcaAnalysisResponseModelJsonSerializer::ToJson(const IdentityModelPtr model) const
  {
    const auto valueTag = JsonTagNames::Values.toStdString();
    return
    {
      { JsonTagNames::Components, AnalysisComponentsToJson(model->Get<Structures::AnalysisComponents>(PcaAnalysisResponseModel::Components))},
      TTitleToJsonPair(model, PcaAnalysisResponseModel::T_Title),
      IntVectorToJsonPair(model, PcaAnalysisResponseModel::P_Ids, JsonTagNames::Pids.toStdString()),
      StringListToJsonPair(model, PcaAnalysisResponseModel::P_Title, JsonTagNames::PTitle.toStdString()),
      { JsonTagNames::AnalysisTable, AnalysisTableToJson(model->Get<AnalysisTable>(PcaAnalysisResponseModel::AnalysisTable))}
    };
  }

  IdentityModelPtr PcaAnalysisResponseModelJsonSerializer::ToModel(const QJsonObject& json) const
  {
    auto model = std::make_shared<PcaAnalysisResponseModel>();
    model->Set(PcaAnalysisResponseModel::P_Title, ParseStringList(json, JsonTagNames::PTitle.toStdString()));
    model->Set(PcaAnalysisResponseModel::P_Ids, ParseIntVector(json, JsonTagNames::Pids.toStdString()));
    model->Set(PcaAnalysisResponseModel::T_Title, ParseAnalysisMetaInfoList(json, JsonTagNames::TTitle.toStdString()));
    model->Set(PcaAnalysisResponseModel::T_Title, ParseAnalysisMetaInfoMap(json, JsonTagNames::TTitle.toStdString()));
    model->Set(PcaAnalysisResponseModel::Components, ParseAnalysisComponents(json));
    model->Set(PcaAnalysisResponseModel::PC9599, ParseBool(json, JsonTagNames::PC95_99.toStdString()));
    QStringList pk, ph;
    auto jpk = json[JsonTagNames::PassportKeys].toArray();
    auto jph = json[JsonTagNames::PassportHeaders].toArray();
    for(int i = 0; i < jpk.size(); i++)
      pk << jpk[i].toString();
    for(int i = 0; i < jph.size(); i++)
      ph << jph[i].toString();
    model->Set(PcaAnalysisResponseModel::PassportKeys, pk);
    model->Set(PcaAnalysisResponseModel::PassportHeaders, ph);
    model->Set(PcaAnalysisResponseModel::Autoscale, ParseBool(json, JsonTagNames::Autoscale.toStdString()));
    model->Set(PcaAnalysisResponseModel::NormData, ParseBool(json, JsonTagNames::Normdata.toStdString()));
    model->Set(AnalysisIdentityModel::ExportRequestBody, QJsonDocument(json.value(JsonTagNames::ExportRequestBody).toObject()).toJson(QJsonDocument::Compact));
    model->Set(PcaAnalysisResponseModel::AnalysisTable, ParseAnalysisTable(json.value(JsonTagNames::AnalysisTable).toObject()));
    return model;
  }
}
