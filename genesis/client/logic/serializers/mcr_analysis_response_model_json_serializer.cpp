#include "mcr_analysis_response_model_json_serializer.h"

#include "logic/models/mcr_analysis_response_model.h"
#include "logic/known_json_tag_names.h"
#include "logic/structures/common_structures.h"

#include <QJsonArray>
#include <QJsonDocument>

using namespace Structures;

#define _Q(str) QString::fromStdString(str)
#define _S(str) str.toStdString()

namespace Service
{
  QJsonObject McrAnalysisResponseModelJsonSerializer::ToJson(const IdentityModelPtr model) const
  {
    return
    {
      { JsonTagNames::CalculatedParameters, CalculatedParametersToJson(model) },
      { JsonTagNames::InputParameters, InputParametersToJson(model) },
      { JsonTagNames::TTitle, AnalysisMetaInfoListToJson(model->Get<AnalysisMetaInfoList>(McrAnalysisResponseModel::TTitle)) },
      { JsonTagNames::ConcentrationsTable, McrConcentrationsTableToJsonArray(model->Get<MCRConcentrationTable>(McrAnalysisResponseModel::ConcentrationsTable))},
      { JsonTagNames::Concentrations, McrCalculationListToJson(model->Get<MCRCalculationList>(McrAnalysisResponseModel::Concentrations))},
      { JsonTagNames::Spectra, McrCalculationListToJson(model->Get<MCRCalculationList>(McrAnalysisResponseModel::Spectra))},
      { JsonTagNames::Difference, McrCalculationListToJson(model->Get<MCRCalculationList>(McrAnalysisResponseModel::Difference))},
      { JsonTagNames::Reconstructions, McrCalculationListToJson(model->Get<MCRCalculationList>(McrAnalysisResponseModel::Reconstructions))},
      { JsonTagNames::Originals, McrCalculationListToJson(model->Get<MCRCalculationList>(McrAnalysisResponseModel::Originals))},
      IntVectorToJsonPair(model, McrAnalysisResponseModel::Pids, JsonTagNames::Pids.toStdString()),
      { JsonTagNames::AnalysisTable, AnalysisTableToJson(model->Get<AnalysisTable>(McrAnalysisIdentityModel::AnalysisTable)) },
    };
  }

  IdentityModelPtr McrAnalysisResponseModelJsonSerializer::ToModel(const QJsonObject& json) const
  {
    const auto model = std::make_shared<McrAnalysisResponseModel>();
    model->Set(McrAnalysisResponseModel::CalculatedParameters,
      ParseCalculatedParameters(json, _S(JsonTagNames::CalculatedParameters)));
    model->Set(McrAnalysisResponseModel::InputParameters,
      ParseInputParameters(json, _S(JsonTagNames::InputParameters)));
    model->Set(McrAnalysisResponseModel::TTitle,
      ParseAnalysisMetaInfoList(json, _S(JsonTagNames::TTitle)));
    model->Set(McrAnalysisResponseModel::ConcentrationsTable, ParseConcentrationTable(json, _S(JsonTagNames::ConcentrationsTable)));
    model->Set(McrAnalysisResponseModel::Concentrations, ParseMcrCalculationList(json, _S(JsonTagNames::Concentrations)));
    model->Set(McrAnalysisResponseModel::Spectra, ParseMcrCalculationList(json, _S(JsonTagNames::Spectra)));
    model->Set(McrAnalysisResponseModel::Difference, ParseMcrCalculationList(json, _S(JsonTagNames::Difference)));
    model->Set(McrAnalysisResponseModel::Reconstructions, ParseMcrCalculationList(json, _S(JsonTagNames::Reconstructions)));
    model->Set(McrAnalysisResponseModel::Originals, ParseMcrCalculationList(json, _S(JsonTagNames::Originals)));
    model->Set(AnalysisIdentityModel::ExportRequestBody, QJsonDocument(json.value(JsonTagNames::ExportRequestBody).toObject()).toJson(QJsonDocument::Compact));
    model->Set(McrAnalysisResponseModel::Pids, ParseIntVector(json, JsonTagNames::Pids.toStdString()));
    model->Set(McrAnalysisResponseModel::AnalysisTable, ParseAnalysisTable(json.value(JsonTagNames::AnalysisTable).toObject()));
    return model;
  }
}