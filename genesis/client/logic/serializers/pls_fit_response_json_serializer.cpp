#include "pls_fit_response_json_serializer.h"

#include "logic/models/pls_fit_response_model.h"
#include "logic/serializers/json_combiner.h"
#include "logic/known_json_tag_names.h"

#include <QJsonArray>
#include <QJsonDocument>

#define _Q(str) QString::fromStdString(str)
#define _S(str) str.toStdString()

namespace Service
{
  using namespace Model;

  QJsonObject PlsFitResponseJsonSerializer::ToJson(const IdentityModelPtr model) const
  {
    JSonCombiner combiner;
    return combiner.Combine(
      {
        { UnsignedIntToJsonPair(model, PlsFitResponseModel::DefaultNumberPC, _S(JsonTagNames::DefaultNumberPC)) },
        { UnsignedIntToJsonPair(model, PlsFitResponseModel::ChosenNumberPC, _S(JsonTagNames::chosen_pc_count)) },
        { ComponentsToJsonPair(model, PlsFitResponseModel::PlsComponents) },
        { StringListToJsonPair(model, PlsFitResponseModel::P_Title, _S(JsonTagNames::PTitle)) },
        { TTitleToJsonPair(model, PlsFitResponseModel::T_TitleGrad) },
        { TTitleToJsonPair(model, PlsFitResponseModel::T_TitleProv) },
        DoubleVectorToJson(model, PlsFitResponseModel::VarianceX, _S(JsonTagNames::VarianceX)),
        DoubleVectorToJson(model, PlsFitResponseModel::VarianceYGrad, _S(JsonTagNames::VarianceYGrad)),
        DoubleVectorToJson(model, PlsFitResponseModel::VarianceYProv, _S(JsonTagNames::VarianceYProv)),
        DoubleVectorToJson(model, PlsFitResponseModel::RMSEGradAll, _S(JsonTagNames::RMSEGradAll)),
        DoubleVectorToJson(model, PlsFitResponseModel::RMSEProvAll, _S(JsonTagNames::RMSEProvAll)),
        DoubleVectorToJson(model, PlsFitResponseModel::YxGrad, _S(JsonTagNames::YxGrad)),
        DoubleVectorToJson(model, PlsFitResponseModel::YxProv, _S(JsonTagNames::YxProv)),
        { ByteDataToJsonPair(model, PlsFitResponseModel::BinaryModel, _S(JsonTagNames::BinaryModel)) },
        { { JsonTagNames::ExportRequestBody, QJsonDocument::fromJson(model->Get<QByteArray>(PlsFitResponseModel::ExportRequestBody)).object() } },
        { IntVectorToJsonPair(model, PlsFitResponseModel::P_id, _S(JsonTagNames::Pids)) },
        { DoubleVectorToJsonPair(model, PlsFitResponseModel::PossibleTestSamplesConcentrations, _S(JsonTagNames::PossibleTestSampleConcentrations)) },
        { IntVectorToJsonPair(model, PlsFitResponseModel::PossibleTestSamplesIds, _S(JsonTagNames::PossibleTestSampleIds)) },
        { DoubleVectorToJsonPair(model, PlsFitResponseModel::TestSamplesConcentrations, _S(JsonTagNames::TestSampleConcentrations)) },
        { IntVectorToJsonPair(model, PlsFitResponseModel::TestSamplesIds, _S(JsonTagNames::TestSampleIds)) },
        { TTitleToJsonPair(model, PlsFitResponseModel::T_TitlePossible) },
        { UnsignedIntToJsonPair(model, PlsFitResponseModel::nMaxLv, _S(JsonTagNames::NMaxLV)) },
        { { JsonTagNames::AnalysisTable, AnalysisTableToJson(model->Get<AnalysisTable>(PlsFitResponseModel::AnalysisTable)) } },
        { StringToJsonPair(model, PlsFitResponseModel::LayerName, _S(JsonTagNames::LayerName)) },
      }
    );
  }

  IdentityModelPtr PlsFitResponseJsonSerializer::ToModel(const QJsonObject& json) const
  {
    auto plsModel = std::make_shared<PlsFitResponseModel>();
    const auto defaultNumber = ParseUnsignedInt(json, _S(JsonTagNames::DefaultNumberPC));
    plsModel->Set(PlsFitResponseModel::DefaultNumberPC, defaultNumber);
    const auto choosen = ParseUnsignedInt(json, _S(JsonTagNames::chosen_pc_count));
    plsModel->Set(PlsFitResponseModel::ChosenNumberPC, !choosen ? defaultNumber : choosen);
    plsModel->Set(PlsFitResponseModel::P_Title, ParseStringList(json, _S(JsonTagNames::PTitle)));
    plsModel->Set(PlsFitResponseModel::T_TitleGrad, ParseAnalysisMetaInfoList(json, _S(JsonTagNames::TTitleGrad)));
    plsModel->Set(PlsFitResponseModel::T_TitleProv, ParseAnalysisMetaInfoList(json, _S(JsonTagNames::TTitleProv)));
    plsModel->Set(PlsFitResponseModel::T_TitlePossible, ParseAnalysisMetaInfoList(json, _S(JsonTagNames::TTitlePossible)));
    plsModel->Set(PlsFitResponseModel::PlsComponents, ParseComponents(json));
    plsModel->Set(PlsFitResponseModel::RMSEGradAll, ParseDoubleVector(json, _S(JsonTagNames::RMSEGradAll)));
    plsModel->Set(PlsFitResponseModel::RMSEProvAll, ParseDoubleVector(json, _S(JsonTagNames::RMSEProvAll)));
    plsModel->Set(PlsFitResponseModel::VarianceX, ParseDoubleVector(json, _S(JsonTagNames::VarianceX)));
    plsModel->Set(PlsFitResponseModel::VarianceYGrad, ParseDoubleVector(json, _S(JsonTagNames::VarianceYGrad)));
    plsModel->Set(PlsFitResponseModel::VarianceYProv, ParseDoubleVector(json, _S(JsonTagNames::VarianceYProv)));
    plsModel->Set(PlsFitResponseModel::BinaryModel, ParseByteData(json, _S(JsonTagNames::BinaryModel)));
    plsModel->Set(PlsFitResponseModel::YxGrad, ParseDoubleVector(json, _S(JsonTagNames::YxGrad)));
    plsModel->Set(PlsFitResponseModel::YxProv, ParseDoubleVector(json, _S(JsonTagNames::YxProv)));
    plsModel->Set(PlsFitResponseModel::PossibleTestSamplesConcentrations, ParseDoubleVector(json, _S(JsonTagNames::PossibleTestSampleConcentrations)));
    plsModel->Set(PlsFitResponseModel::PossibleTestSamplesIds, ParseIntVector(json, _S(JsonTagNames::PossibleTestSampleIds)));
    plsModel->Set(PlsFitResponseModel::TestSamplesConcentrations, ParseDoubleVector(json, _S(JsonTagNames::TestSampleConcentrations)));
    plsModel->Set(PlsFitResponseModel::TestSamplesIds, ParseIntVector(json, _S(JsonTagNames::TestSampleIds)));
    /// just save string bcs data not used in front end. only for export in excel
    plsModel->Set(PlsFitResponseModel::ExportRequestBody, QJsonDocument(json.value(JsonTagNames::ExportRequestBody).toObject()).toJson(QJsonDocument::Compact));
    plsModel->Set(PlsFitResponseModel::P_id, ParseIntVector(json, _S(JsonTagNames::Pids)));
    plsModel->Set(PlsFitResponseModel::nMaxLv, ParseUnsignedInt(json, _S(JsonTagNames::NMaxLV)));
    plsModel->Set(PlsFitResponseModel::AnalysisTable, ParseAnalysisTable(json.value(JsonTagNames::AnalysisTable).toObject()));
    plsModel->Set(PlsFitResponseModel::LayerName, ParseString(json, _S(JsonTagNames::LayerName)));
    return plsModel;
  }
}