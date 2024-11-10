#pragma once
#ifndef MCR_ANALYSIS_COMMON_JSON_SERIALIZER_H
#define MCR_ANALYSIS_COMMON_JSON_SERIALIZER_H

#include "analysis_identity_model_json_serializer.h"

#include <QJsonArray>

namespace Service
{
  class McrAnalysisCommonJsonSerializer : public AnalysisIdentityModelJsonSerializer
  {
  protected:
    QJsonObject CalculatedParametersToJson(const IdentityModelPtr model) const;
    QJsonObject InputParametersToJson(const IdentityModelPtr model) const;

    QJsonArray McrConcentrationsTableToJsonArray(const MCRConcentrationTable& table) const;
    QJsonObject McrConcentrationsTableItemToJson(const MCRConcentrationTableItem& item) const;
    QJsonArray ReferenceSettingsToJsonArray(const ReferenceSettingsList& list) const;
    QJsonObject ReferenceSettingsToJson(const ReferenceSettings& settings) const;
    QJsonArray McrConcentrationsListToJsonArray(const MCRConcentrationList& list) const;
    QJsonObject McrConcentrationToJson(const MCRConcentration& item) const;
    QJsonArray McrCalculationListToJson(const MCRCalculationList& list) const;
    QJsonObject McrCalculationToJson(const MCRCalculation& item) const;

    MCRCalculatedParameters ParseCalculatedParameters(const QJsonObject& json, const std::string_view tagName) const;
    MCRParameters ParseInputParameters(const QJsonObject& json, const std::string_view tagName) const;
    ReferenceSettingsList ParseReferenceSettingsList(const QJsonObject& json, const std::string_view& tagName) const;
    ReferenceSettings ParseReferenceSettings(const QJsonObject& json) const;
    MCRConcentrationTable ParseConcentrationTable(const QJsonObject& json, const std::string_view jsonTag) const;
    MCRConcentrationTableItem ParseConcentrationTableItem(const QJsonObject& json) const;
    MCRConcentrationList ParseMcrConcentrationList(const QJsonObject& json, const std::string_view jsonTag) const;
    MCRConcentration ParseMcrConcentration(const QJsonObject& json) const;
    MCRCalculationList ParseMcrCalculationList(const QJsonObject& json, const std::string_view tagName) const;
    MCRCalculation ParseMcrCalculation(const QJsonObject& json) const;
  };
}
#endif