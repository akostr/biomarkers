#include "mcr_analysis_common_json_serializer.h"

#include "logic/known_json_tag_names.h"
#include "logic/structures/common_structures.h"
#include "logic/models/mcr_analysis_identity_model.h"

using namespace Structures;

#define _Q(str) QString::fromStdString(str)
#define _S(str) str.toStdString()

namespace Service
{

  QJsonObject McrAnalysisCommonJsonSerializer::CalculatedParametersToJson(const IdentityModelPtr model) const
  {
    const auto params = model->Get<MCRCalculatedParameters>(McrAnalysisIdentityModel::CalculatedParameters);
    return
    {
      { JsonTagNames::MSE, params.MSE },
      { JsonTagNames::Lof, params.Lof },
      { JsonTagNames::Variance, params.Vairance },
      { JsonTagNames::NIter, static_cast<int>(params.NIter) },
    };
  }

  QJsonObject McrAnalysisCommonJsonSerializer::InputParametersToJson(const IdentityModelPtr model) const
  {
    const auto params = model->Get<MCRParameters>(McrAnalysisIdentityModel::InputParameters);
    return
    {
      { JsonTagNames::Key, _Q(params.Key) },
      { JsonTagNames::NumberComponents, static_cast<int>(params.NumberComponents) },
      { JsonTagNames::ClosedSystem, params.ClosedSystem },
      { JsonTagNames::Normdata, params.NormData },
      { JsonTagNames::MaxIter, static_cast<int>(params.MaxIter) },
      { JsonTagNames::TolErrChange, params.TolErrChange},
      { JsonTagNames::Method, _Q(params.Method) },
      { JsonTagNames::ReferenceSettings, ReferenceSettingsToJsonArray(params.Settings) },
    };
  }

  QJsonArray McrAnalysisCommonJsonSerializer::McrConcentrationsTableToJsonArray(const MCRConcentrationTable& table) const
  {
    QJsonArray result;
    std::transform(table.begin(), table.end(), std::back_inserter(result),
      [&](const MCRConcentrationTableItem& item) {return McrConcentrationsTableItemToJson(item); });
    return result;
  }

  QJsonObject McrAnalysisCommonJsonSerializer::McrConcentrationsTableItemToJson(const MCRConcentrationTableItem& item) const
  {
    return
    {
      { JsonTagNames::ComponentId, static_cast<int>(item.Id) },
      { JsonTagNames::ColumnValues, McrConcentrationsListToJsonArray(item.ColumnValues) },
    };
  }

  QJsonArray McrAnalysisCommonJsonSerializer::ReferenceSettingsToJsonArray(const ReferenceSettingsList& list) const
  {
    QJsonArray result;
    std::transform(list.begin(), list.end(), std::back_inserter(result),
      [&](const ReferenceSettings& item) -> QJsonObject
      {
        return ReferenceSettingsToJson(item);
      });
    return result;
  }

  QJsonObject McrAnalysisCommonJsonSerializer::ReferenceSettingsToJson(const ReferenceSettings& settings) const
  {
    return
    {
      { JsonTagNames::Fix, settings.Fix },
      { JsonTagNames::NonNegativeConcentration, settings.NonNegativeConcentration },
      { JsonTagNames::NonNegativeSpectrum, settings.NonNegativeSpectrum },
      { JsonTagNames::ReferenceId, static_cast<int>(settings.ReferenceId) },
    };
  }

  QJsonArray McrAnalysisCommonJsonSerializer::McrConcentrationsListToJsonArray(const MCRConcentrationList& list) const
  {
    QJsonArray result;
    std::transform(list.begin(), list.end(), std::back_inserter(result),
      [&](const MCRConcentration& item) -> QJsonObject
      {
        return McrConcentrationToJson(item);
      });
    return result;
  }

  QJsonObject McrAnalysisCommonJsonSerializer::McrConcentrationToJson(const MCRConcentration& item) const
  {
    return
    {
      { JsonTagNames::Value, item.Value },
      { JsonTagNames::TableHeader, _Q(item.TableHeader) },
      { JsonTagNames::TitleId, static_cast<int>(item.Id) },
    };
  }

  QJsonArray McrAnalysisCommonJsonSerializer::McrCalculationListToJson(const MCRCalculationList& list) const
  {
    QJsonArray result;
    std::transform(list.begin(), list.end(), std::back_inserter(result),
      [&](const MCRCalculation& item) {return McrCalculationToJson(item); });
    return result;
  }

  QJsonObject McrAnalysisCommonJsonSerializer::McrCalculationToJson(const MCRCalculation& item) const
  {
    return
    {
      { JsonTagNames::Values, DoubleVectorToJsonArray(item.Values) },
      { JsonTagNames::TitleId, static_cast<int>(item.Id) },
      { JsonTagNames::ComponentNumber, static_cast<int>(item.ComponentNumber) },
    };
  }

  MCRCalculatedParameters McrAnalysisCommonJsonSerializer::ParseCalculatedParameters(const QJsonObject& json, const std::string_view tagName) const
  {
    const auto obj = json.value(tagName.data()).toObject();
    return
    {
      ParseDouble(obj, _S(JsonTagNames::MSE)),
      ParseDouble(obj, _S(JsonTagNames::Lof)),
      ParseDouble(obj, _S(JsonTagNames::Variance)),
      ParseUnsignedInt(obj, _S(JsonTagNames::NIter)),
    };
  }

  MCRParameters McrAnalysisCommonJsonSerializer::ParseInputParameters(const QJsonObject& json, const std::string_view tagName) const
  {
    const auto obj = json.value(tagName.data()).toObject();
    return
    {
      ParseString(obj, _S(JsonTagNames::Key)),
      ParseString(obj, _S(JsonTagNames::Method)),
      ParseDouble(obj, _S(JsonTagNames::TolErrChange)),
      ParseUnsignedInt(obj, _S(JsonTagNames::NumberComponents)),
      ParseUnsignedInt(obj, _S(JsonTagNames::MaxIter)),
      ParseReferenceSettingsList(obj, _S(JsonTagNames::ReferenceSettings)),
      ParseBool(obj, _S(JsonTagNames::ClosedSystem)),
      ParseBool(obj, _S(JsonTagNames::Normdata)),
    };
  }

  ReferenceSettingsList McrAnalysisCommonJsonSerializer::ParseReferenceSettingsList(const QJsonObject& json, const std::string_view& tagName) const
  {
    ReferenceSettingsList result;
    const auto jsonArray = json.value(tagName.data()).toArray();
    std::transform(jsonArray.begin(), jsonArray.end(), std::back_inserter(result),
#ifdef QT_6_4_0
      [&](QJsonValueConstRef value)
#else
      [&](QJsonValueRef value)
#endif
    { return ParseReferenceSettings(value.toObject()); });
    return result;
  }

  ReferenceSettings McrAnalysisCommonJsonSerializer::ParseReferenceSettings(const QJsonObject& json) const
  {
    return
    {
      ParseUnsignedInt(json, _S(JsonTagNames::ReferenceId)),
      ParseBool(json, _S(JsonTagNames::NonNegativeConcentration)),
      ParseBool(json, _S(JsonTagNames::NonNegativeSpectrum)),
      ParseBool(json, _S(JsonTagNames::Fix)),
    };
  }

  MCRConcentrationTable McrAnalysisCommonJsonSerializer::ParseConcentrationTable(const QJsonObject& json, const std::string_view jsonTag) const
  {
    MCRConcentrationTable table;
    const auto tableArray = json.value(jsonTag.data()).toArray();
    table.reserve(tableArray.size());
    std::transform(tableArray.begin(), tableArray.end(), std::back_inserter(table),
#ifdef QT_6_4_0
      [&](QJsonValueConstRef value)
#else
      [&](QJsonValueRef value)
#endif
    { return ParseConcentrationTableItem(value.toObject()); });
    return table;
  }

  MCRConcentrationTableItem McrAnalysisCommonJsonSerializer::ParseConcentrationTableItem(const QJsonObject& json) const
  {
    return
    {
      ParseUnsignedInt(json, _S(JsonTagNames::TitleId)),
      ParseMcrConcentrationList(json, _S(JsonTagNames::ColumnValues))
    };
  }

  MCRConcentrationList McrAnalysisCommonJsonSerializer::ParseMcrConcentrationList(const QJsonObject& json, const std::string_view jsonTag) const
  {
    MCRConcentrationList list;
    const auto tableArray = json.value(jsonTag.data()).toArray();
    std::transform(tableArray.begin(), tableArray.end(), std::back_inserter(list),
#ifdef QT_6_4_0
      [&](QJsonValueConstRef value)
#else
      [&](QJsonValueRef value)
#endif
    { return ParseMcrConcentration(value.toObject()); });
    return list;
  }

  MCRConcentration McrAnalysisCommonJsonSerializer::ParseMcrConcentration(const QJsonObject& json) const
  {
    return
    {
      ParseDouble(json, _S(JsonTagNames::Value)),
      ParseString(json, _S(JsonTagNames::TableHeader)),
      ParseUnsignedInt(json, _S(JsonTagNames::ComponentId)),
    };
  }

  MCRCalculationList McrAnalysisCommonJsonSerializer::ParseMcrCalculationList(const QJsonObject& json, const std::string_view tagName) const
  {
    MCRCalculationList result;
    const auto list = json.value(tagName.data()).toArray();
    std::transform(list.begin(), list.end(), std::back_inserter(result),
#ifdef QT_6_4_0
      [&](QJsonValueConstRef value)
#else
      [&](QJsonValueRef value)
#endif
    { return ParseMcrCalculation(value.toObject()); });
    return result;
  }

  MCRCalculation McrAnalysisCommonJsonSerializer::ParseMcrCalculation(const QJsonObject& json) const
  {
    return
    {
      ParseDoubleVector(json, _S(JsonTagNames::Values)),
      ParseUnsignedInt(json, _S(JsonTagNames::TitleId)),
      ParseUnsignedInt(json, _S(JsonTagNames::ComponentNumber)),
    };
  }
}
