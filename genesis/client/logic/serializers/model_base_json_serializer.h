#pragma once
#ifndef ANALYSIS_MODEL_JSON_SERIALIZER_H
#define ANALYSIS_MODEL_JSON_SERIALIZER_H

#include "logic/service/ijson_serializer.h"
#include "logic/models/identity_model.h"
#include "logic/structures/analysis_model_structures.h"

namespace Service
{
  using namespace Structures;

  using JsonPair = QPair<QString, QJsonValue>;

  class ModelBaseJsonSerializer : public IJsonSerializer
  {
  protected:
    QJsonObject ParseSubObject(const QJsonObject& json, std::string_view tagName) const;

    /*
    * Base types json to model
    */
    bool ParseBool(const QJsonObject& json, std::string_view boolTagName) const;
    double ParseDouble(const QJsonObject& json, std::string_view tagName) const;
    std::string ParseString(const QJsonObject& json, std::string_view stringTagName) const;
    size_t ParseUnsignedInt(const QJsonObject& json, std::string_view uintTagName) const;
    StringList ParseStringList(const QJsonObject& json, std::string_view stringListTagName) const;
    StringList2D ParseStringList2D(const QJsonObject& json, std::string_view stringListTagName) const;
    ByteArray ParseByteData(const QJsonObject& json, std::string_view byteDataName) const;
    DoubleVector ParseDoubleVector(const QJsonObject& json, std::string_view arrayTag) const;
    DoubleVector ParseDoubleVector(const QJsonArray& jsonArray) const;
    Double2DVector ParseDouble2DVector(const QJsonObject& json, std::string_view tagName) const;
    IntVector ParseIntVector(const QJsonObject& json, std::string_view arrayTag) const;
    IntVector ParseIntVector(const QJsonArray& jsonArray) const;
    OptionalDoubleVector ParseOptionalDoubleVector(const QJsonArray& jsonArray) const;
    OptionalDoubleVector ParseOptionalDoubleVector(const QJsonObject& json, std::string_view tagName) const;
    OptionalDouble2DVector ParseOptionalDouble2DVector(const QJsonObject& json, std::string_view tagName) const;

    /*
    * Base types model to json
    */
    JsonPair BoolToJsonPair(const IdentityModelPtr model, std::string_view boolModelName, std::string_view boolJsonTagName) const;
    JsonPair StringToJsonPair(const IdentityModelPtr model, std::string_view stringModelName, std::string_view stringJsonTagName) const;
    JsonPair UnsignedIntToJsonPair(const IdentityModelPtr model, std::string_view uintModelName, std::string_view uintJsonTagName) const;
    JsonPair IntToJsonPair(const IdentityModelPtr model, std::string_view uintModelName, std::string_view uintJsonTagName) const;
    JsonPair StringListToJsonPair(const IdentityModelPtr model, std::string_view stringListModelName, std::string_view stringListJsonTagName) const;
    JsonPair StringList2DToJsonPair(const IdentityModelPtr model, std::string_view stringListModelName, std::string_view stringListJsonTagName) const;
    QJsonArray StringListToJsonArray(const StringList& list);
    JsonPair ByteDataToJsonPair(const IdentityModelPtr model, std::string_view fitName, std::string_view jsonTagName) const;
    QJsonObject DoubleVectorToJson(const IdentityModelPtr model,
      std::string_view vectorName, std::string_view jsonTagName) const;
    JsonPair DoubleVectorToJsonPair(const IdentityModelPtr model,
      std::string_view vectorName, std::string_view jsonTagName) const;
    QJsonArray DoubleVectorToJsonArray(const DoubleVector& vec) const;
    JsonPair DoubleVector2DToJson(const IdentityModelPtr model, std::string_view modelTagName, std::string_view jsonTagName) const;
    QJsonArray DoubleOptionalVectorToJsonArray(const OptionalDoubleVector& vec) const;
    JsonPair DoubleOptionalVectorToJsonPair(const IdentityModelPtr model, std::string_view modelTagName, std::string_view jsonTagName) const;
    JsonPair DoubleOptionalVector2DToJsonPair(const IdentityModelPtr model, std::string_view modelTagName, std::string_view jsonTagName) const;
    QJsonArray DoubleOptionalVector2DToJsonArray(const OptionalDouble2DVector& vec) const;
    QJsonArray IntVectorToJsonArray(const IntVector& vec) const;
    JsonPair IntVectorToJsonPair(const IdentityModelPtr model, std::string_view modelTagName, std::string_view jsonTagName) const;

    /*
    * from json to model
    */
    PlsComponents ParseComponents(const QJsonObject& json) const;
    PlsComponent ParseComponent(const QJsonObject& json) const;
    MetaInfoList ParseMetaInfoList(const QJsonObject& json) const; // T title
    MetaInfo ParseMetaInfo(const QJsonObject& json) const;
    Statistic ParseStatistic(const QJsonObject& json, std::string_view provTag, std::string_view gradTag) const;
    AnalysisMetaInfoList ParseAnalysisMetaInfoList(const QJsonObject& json, std::string_view ttitleTag) const;
    AnalysisMetaInfoMapsList ParseAnalysisMetaInfoMap(const QJsonObject& json, std::string_view ttitleTag) const;
    AnalysisMetaInfo ParseAnalysisMetaInfo(const QJsonObject& json) const;
    AnalysisMetaInfoWithPeaksList ParseAnalysisMetaInfoWithPeaksList(const QJsonObject& json,
      std::string_view dataTag) const;
    AnalysisMetaInfoWithPeaks ParseAnalysisMetaInfoWithPeaks(const QJsonObject& json) const;
    MetaAnalysisPeaks ParsePeaks(const QJsonObject& json) const;
    MetaAnalysisPeak ParsePeak(const QJsonObject& json) const;
    /*
    * from model to json
    */
    JsonPair ComponentsToJsonPair(const IdentityModelPtr model, std::string_view componetsName) const;
    JsonPair TTitleToJsonPair(const IdentityModelPtr model, std::string_view ttitleName) const;
    QJsonObject MetaInfoToJsonObject(const MetaInfo& info) const;
    QJsonObject StatisticToJson(const IdentityModelPtr model,
      std::string_view statisticName,
      std::string_view provName,
      std::string_view gradName) const;
    QJsonObject PlsComponentToJsonObject(const PlsComponent& component) const;
    QJsonArray AnalysisMetaInfoListToJson(const AnalysisMetaInfoList& metaList) const;
    QJsonObject AnalysisMetaInfoToJson(const AnalysisMetaInfo& metaInfo) const;
    QJsonObject DoubleVector2DToJson(const Double2DVector& vec2d, std::string_view tagName) const;
    QJsonArray StringListToJsonArray(const StringList& list) const;
    QJsonArray ComponetsToJsonArray(const PlsComponents& components) const;
    QJsonObject AnalysisMetaInfoWithPeaksToJson(const AnalysisMetaInfoWithPeaks& data) const;
    QJsonArray AnalysisMetaInfoWithPeaksListToJson(const AnalysisMetaInfoWithPeaksList& list) const;
    QJsonArray AnalysisMetaInfoWithPeaksListToJson(const IdentityModelPtr& model,
      std::string_view dataTag) const;
    QJsonArray PeaksToJson(const MetaAnalysisPeaks& peaks) const;
    QJsonObject PeakToJson(const MetaAnalysisPeak& peak) const;
  };
}
#endif
