#include "mcr_analysis_request_model_json_serializer.h"

#include "logic/models/mcr_analysis_request_model.h"
#include "logic/known_json_tag_names.h"

#include <QJsonArray>

#define _Q(str) QString::fromStdString(str)
#define _S(str) str.toStdString()

namespace Service
{
    QJsonObject McrAnalysisRequestModelJsonSerializer::ToJson(const IdentityModelPtr model) const
    {
      return
      {
        StringToJsonPair(model, McrAnalysisRequestModel::Key, _S(JsonTagNames::Key)),
        UnsignedIntToJsonPair(model, McrAnalysisRequestModel::NumberComponents, _S(JsonTagNames::NumberComponents)),
        BoolToJsonPair(model, McrAnalysisRequestModel::ClosedSystem, _S(JsonTagNames::ClosedSystem)),
        BoolToJsonPair(model, McrAnalysisRequestModel::Normdata, _S(JsonTagNames::Normdata)),
        UnsignedIntToJsonPair(model, McrAnalysisRequestModel::MaxIter, _S(JsonTagNames::MaxIter)),
        { JsonTagNames::TolErrChange, model->Get<double>(McrAnalysisRequestModel::TolErrorChange) },
        StringToJsonPair(model, McrAnalysisRequestModel::Method, _S(JsonTagNames::Method)),
        { JsonTagNames::ReferenceSettings, ReferenceSettingsListToJsonArray(model, McrAnalysisRequestModel::ReferenceSettings)},
        IntVectorToJsonPair(model, McrAnalysisRequestModel::SampleIds, _S(JsonTagNames::SamplesIds)),
        IntVectorToJsonPair(model, McrAnalysisRequestModel::MarkerIds, _S(JsonTagNames::MarkersIds)),
        IntToJsonPair(model, McrAnalysisRequestModel::TableId, _S(JsonTagNames::TableId)),
      };
    }

    IdentityModelPtr McrAnalysisRequestModelJsonSerializer::ToModel(const QJsonObject& json) const
    {
      const auto model = std::make_shared<McrAnalysisRequestModel>();
      model->Set(McrAnalysisRequestModel::Key, ParseString(json, _S(JsonTagNames::Key)));
      model->Set(McrAnalysisRequestModel::NumberComponents, ParseUnsignedInt(json, _S(JsonTagNames::NumberComponents)));
      model->Set(McrAnalysisRequestModel::ClosedSystem, ParseBool(json, _S(JsonTagNames::ClosedSystem)));
      model->Set(McrAnalysisRequestModel::Normdata, ParseBool(json, _S(JsonTagNames::Normdata)));
      model->Set(McrAnalysisRequestModel::MaxIter, ParseUnsignedInt(json, _S(JsonTagNames::MaxIter)));
      model->Set(McrAnalysisRequestModel::TolErrorChange, ParseDouble(json, _S(JsonTagNames::TolErrChange)));
      model->Set(McrAnalysisRequestModel::Method, ParseString(json, _S(JsonTagNames::Method)));
      model->Set(McrAnalysisRequestModel::ReferenceSettings, ParseReferenceSettingsList(json.value(JsonTagNames::ReferenceSettings).toArray()));
      model->Set(McrAnalysisRequestModel::SampleIds, ParseIntVector(json, _S(JsonTagNames::SamplesIds)));
      model->Set(McrAnalysisRequestModel::MarkerIds, ParseIntVector(json, _S(JsonTagNames::MarkersIds)));
      model->Set(McrAnalysisRequestModel::TableId, json.value(JsonTagNames::TableId).toInt());
      return model;
    }

    ReferenceSettingsList McrAnalysisRequestModelJsonSerializer::ParseReferenceSettingsList(const QJsonArray& list) const
    {
      ReferenceSettingsList result;
      std::transform(list.begin(), list.end(), std::back_inserter(result),
#ifdef QT_6_4_0
      [&](QJsonValueConstRef value) -> ReferenceSettings
#else
      [&](QJsonValueRef value) -> ReferenceSettings
#endif
        {
          const auto obj = value.toObject();
          return
          {
            ParseUnsignedInt(obj, _S(JsonTagNames::ReferenceId)),
            ParseBool(obj, _S(JsonTagNames::NonNegativeConcentration)),
            ParseBool(obj, _S(JsonTagNames::NonNegativeSpectrum)),
            ParseBool(obj, _S(JsonTagNames::Fix)),
          };
        });
      return result;
    }

    QJsonArray McrAnalysisRequestModelJsonSerializer::ReferenceSettingsListToJsonArray(const IdentityModelPtr model, std::string& tag) const
    {
      QJsonArray result;
      model->Exec<ReferenceSettingsList>(tag, [&](const ReferenceSettingsList& list)
        {
          std::transform(list.begin(), list.end(), std::back_inserter(result),
          [](const ReferenceSettings& item) -> QJsonObject
            {
              return
              {
                  { JsonTagNames::ReferenceId, static_cast<int>(item.ReferenceId) },
                  { JsonTagNames::NonNegativeConcentration, item.NonNegativeConcentration },
                  { JsonTagNames::NonNegativeSpectrum, item.NonNegativeSpectrum },
                  { JsonTagNames::Fix, item.Fix },
              };
            });
        });
      return result;
    }
}
