#include "mcr_analysis_identity_model_json_serializer.h"

#include "logic/known_json_tag_names.h"
#include "logic/models/mcr_analysis_identity_model.h"

#define _Q(str) QString::fromStdString(str)
#define _S(str) str.toStdString()

namespace Service
{
  QJsonObject McrAnalysisIdentityJsonSerializer::ToJson(const IdentityModelPtr model) const
  {
    auto json = AnalysisIdentityModelJsonSerializer::ToJson(model);

    auto parameterObject = json.value(JsonTagNames::Parameters).toObject();
    parameterObject.insert( JsonTagNames::CalculatedParameters,
      CalculatedParametersToJson(model));
    parameterObject.insert(JsonTagNames::InputParameters,
      InputParametersToJson(model));
    json[JsonTagNames::Parameters] = parameterObject;

    auto analysisData = json.value(JsonTagNames::AnalysisData).toObject();
    analysisData.insert(JsonTagNames::ConcentrationsTable,
      McrConcentrationsTableToJsonArray(model->Get<MCRConcentrationTable>(McrAnalysisIdentityModel::ConcentrationsTable)));
    analysisData.insert(JsonTagNames::Concentrations,
      McrCalculationListToJson(model->Get<MCRCalculationList>(McrAnalysisIdentityModel::Concentrations)));
    analysisData.insert(JsonTagNames::Spectra,
      McrCalculationListToJson(model->Get<MCRCalculationList>(McrAnalysisIdentityModel::Spectra)) );
    analysisData.insert(JsonTagNames::Difference,
      McrCalculationListToJson(model->Get<MCRCalculationList>(McrAnalysisIdentityModel::Difference)) );
    analysisData.insert(JsonTagNames::Reconstructions,
      McrCalculationListToJson(model->Get<MCRCalculationList>(McrAnalysisIdentityModel::Reconstructions)) );
    analysisData.insert(JsonTagNames::Originals,
      McrCalculationListToJson(model->Get<MCRCalculationList>(McrAnalysisIdentityModel::Originals)) );
    analysisData.insert(JsonTagNames::InputParameters, InputParametersToJson(model));
    auto innerData = analysisData.value(JsonTagNames::Data).toObject();
    innerData.insert(JsonTagNames::Pids, IntVectorToJsonArray(model->Get<IntVector>(McrAnalysisIdentityModel::Pids)));
    analysisData[JsonTagNames::Data] = innerData;
    json[JsonTagNames::AnalysisData] = analysisData;
    return json;
  }

  IdentityModelPtr McrAnalysisIdentityJsonSerializer::ToModel(const QJsonObject& json) const
  {
    const auto model = AnalysisIdentityModelJsonSerializer::ToModel(json);
    const auto parameters = json.value(JsonTagNames::Parameters).toObject();
    model->Set(McrAnalysisIdentityModel::CalculatedParameters,
      ParseCalculatedParameters(parameters, _S(JsonTagNames::CalculatedParameters)));

    const auto analysisData = json.value(JsonTagNames::AnalysisData).toObject();
    model->Set(McrAnalysisIdentityModel::ConcentrationsTable, ParseConcentrationTable(analysisData, _S(JsonTagNames::ConcentrationsTable)));
    model->Set(McrAnalysisIdentityModel::InputParameters,
      ParseInputParameters(analysisData, _S(JsonTagNames::InputParameters)));
    model->Set(McrAnalysisIdentityModel::Concentrations, ParseMcrCalculationList(analysisData, _S(JsonTagNames::Concentrations)));
    model->Set(McrAnalysisIdentityModel::Spectra, ParseMcrCalculationList(analysisData, _S(JsonTagNames::Spectra)));
    model->Set(McrAnalysisIdentityModel::Difference, ParseMcrCalculationList(analysisData, _S(JsonTagNames::Difference)));
    model->Set(McrAnalysisIdentityModel::Reconstructions, ParseMcrCalculationList(analysisData, _S(JsonTagNames::Reconstructions)));
    model->Set(McrAnalysisIdentityModel::Originals, ParseMcrCalculationList(analysisData, _S(JsonTagNames::Originals)));
    model->Set(McrAnalysisIdentityModel::Pids, ParseIntVector(analysisData.value(JsonTagNames::Data).toObject(), _S(JsonTagNames::Pids)));
    return model;
  }
}