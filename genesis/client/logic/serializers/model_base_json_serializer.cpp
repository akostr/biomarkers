#include "model_base_json_serializer.h"

#include <QJsonArray>
#include <QStringList>

#include "logic/known_json_tag_names.h"
#include "logic/serializers/json_combiner.h"

#define _Q(str) QString::fromStdString(str)
#define _S(str) str.toStdString()

namespace Service
{
  MetaInfoList ModelBaseJsonSerializer::ParseMetaInfoList(const QJsonObject& json) const
  {
    const auto ttitleArray = json.value(JsonTagNames::TTitle).toArray();
    MetaInfoList infoList;
    std::transform(ttitleArray.begin(), ttitleArray.end(), std::back_inserter(infoList),
#ifdef QT_6_4_0
      [&](QJsonValueConstRef value)
#else
      [&](QJsonValueRef value)
#endif
      { return ParseMetaInfo(value.toObject()); });
    return infoList;
  }

  MetaInfo ModelBaseJsonSerializer::ParseMetaInfo(const QJsonObject& json) const
  {
    return
    {
      static_cast<size_t>(json.value(JsonTagNames::FileId).toInt()),
      static_cast<size_t>(json.value(JsonTagNames::SampleId).toInt()),
      _S(json.value(JsonTagNames::Date).toString()),
      _S(json.value(JsonTagNames::Field).toString()),
      _S(json.value(JsonTagNames::FileName).toString()),
      _S(json.value(JsonTagNames::Layer).toString()),
      _S(json.value(JsonTagNames::Well).toString()),
      _S(json.value(JsonTagNames::WellCluster).toString()),
      _S(json.value(JsonTagNames::Depth).toString())
    };
  }

  Statistic ModelBaseJsonSerializer::ParseStatistic(const QJsonObject& json,
    std::string_view prov, std::string_view grad) const
  {
    return
    {
      json.value(prov.data()).toDouble(),
      json.value(grad.data()).toDouble()
    };
  }

  AnalysisMetaInfoList ModelBaseJsonSerializer::ParseAnalysisMetaInfoList(
    const QJsonObject& json, std::string_view ttitleTag) const
  {
    AnalysisMetaInfoList list;
    const auto infoArray = json.value(ttitleTag.data()).toArray();
    std::transform(infoArray.begin(), infoArray.end(), std::back_inserter(list),
#ifdef QT_6_4_0
      [&](QJsonValueConstRef value)
#else
      [&](QJsonValueRef value)
#endif
      { return ParseAnalysisMetaInfo(value.toObject()); });
    return list;
  }

  AnalysisMetaInfoMapsList ModelBaseJsonSerializer::ParseAnalysisMetaInfoMap(const QJsonObject &json, std::string_view ttitleTag) const
  {
    AnalysisMetaInfoMapsList list;
    const auto infoArray = json.value(ttitleTag.data()).toArray();
    std::transform(infoArray.begin(), infoArray.end(), std::back_inserter(list),
#ifdef QT_6_4_0
      [&](QJsonValueConstRef value)
#else
      [&](QJsonValueRef value)
#endif
      { return (value.toObject().toVariantMap()); });
    return list;
  }

  AnalysisMetaInfo ModelBaseJsonSerializer::ParseAnalysisMetaInfo(const QJsonObject& json) const
  {
    AnalysisMetaInfo info(ParseMetaInfo(json));
    auto yConcentration = json.value(JsonTagNames::YConcentration.data());
    if (!yConcentration.isNull() && yConcentration.isDouble())
      info.YConcentration = yConcentration.toDouble();
    return info;
  }

  AnalysisMetaInfoWithPeaksList ModelBaseJsonSerializer::ParseAnalysisMetaInfoWithPeaksList(const QJsonObject& json, std::string_view dataTag) const
  {
    AnalysisMetaInfoWithPeaksList dataList;
    const auto dataArray = json.value(dataTag.data()).toArray();
    std::transform(dataArray.begin(), dataArray.end(), std::back_inserter(dataList),
#ifdef QT_6_4_0
      [&](QJsonValueConstRef value)
#else
      [&](QJsonValueRef value)
#endif
      { return ParseAnalysisMetaInfoWithPeaks(value.toObject()); });
    return dataList;
  }

  AnalysisMetaInfoWithPeaks ModelBaseJsonSerializer::ParseAnalysisMetaInfoWithPeaks(
    const QJsonObject& json) const
  {
    AnalysisMetaInfoWithPeaks info(ParseMetaInfo(json));
    auto yconc = json.value(JsonTagNames::YConcentration);
    if(!yconc.isNull() && yconc.isDouble())
      info.YConcentration = yconc.toDouble();
    info.Peaks = ParsePeaks(json);
    return info;
  }

  MetaAnalysisPeaks ModelBaseJsonSerializer::ParsePeaks(const QJsonObject& json) const
  {
    const auto peakArray = json.value(JsonTagNames::Peaks).toArray();
    Structures::MetaAnalysisPeaks peaks;
    std::transform(peakArray.begin(), peakArray.end(), std::back_inserter(peaks),
#ifdef QT_6_4_0
      [&](QJsonValueConstRef value)
#else
      [&](QJsonValueRef value)
#endif
      { return ParsePeak(value.toObject()); });
    return peaks;
  }

  MetaAnalysisPeak ModelBaseJsonSerializer::ParsePeak(const QJsonObject& json) const
  {
    return
    {
          ParseString(json, _S(JsonTagNames::Title)),
          json.value(JsonTagNames::Area).toDouble(),
          json.value(JsonTagNames::Height).toDouble(),
          json.value(JsonTagNames::Custom).toDouble(),
          json.value(JsonTagNames::Id).toInt(),
          ParseBool(json, _S(JsonTagNames::IsMarker))
    };
  }

  PlsComponents ModelBaseJsonSerializer::ParseComponents(const QJsonObject& json) const
  {
    const auto componentsJson = json.value(JsonTagNames::Components).toArray();
    PlsComponents components;
    std::transform(componentsJson.begin(), componentsJson.end(), std::back_inserter(components),
#ifdef QT_6_4_0
      [&](QJsonValueConstRef value)
#else
      [&](QJsonValueRef value)
#endif
      {
        return ParseComponent(value.toObject());
      });
    return components;
  }

  PlsComponent ModelBaseJsonSerializer::ParseComponent(const QJsonObject& json) const
  {
    return
    {
      static_cast<size_t>(json.value(JsonTagNames::Number).toInt()),
      ParseStatistic(json, _S(JsonTagNames::RMSEProv), _S(JsonTagNames::RMSEGrad)),
      ParseStatistic(json, _S(JsonTagNames::R2Prov), _S(JsonTagNames::R2Grad)),
      ParseStatistic(json, _S(JsonTagNames::SlopeProv), _S(JsonTagNames::SlopeGrad)),
      ParseStatistic(json, _S(JsonTagNames::OffsetProv), _S(JsonTagNames::OffsetGrad)),
      ParseDoubleVector(json, _S(JsonTagNames::YProv)),
      ParseDoubleVector(json, _S(JsonTagNames::YGrad)),
      ParseDoubleVector(json, _S(JsonTagNames::YLoadings)),
      ParseDoubleVector(json, _S(JsonTagNames::Coefficients)),
      ParseDoubleVector(json, _S(JsonTagNames::T)),
      ParseDoubleVector(json, _S(JsonTagNames::P))
    };
  }

  DoubleVector ModelBaseJsonSerializer::ParseDoubleVector(const QJsonObject& json,
    std::string_view arrayTag) const
  {
    return ParseDoubleVector(json.value(arrayTag.data()).toArray());
  }

  DoubleVector ModelBaseJsonSerializer::ParseDoubleVector(const QJsonArray& jsonArray) const
  {
    DoubleVector result;
    std::transform(jsonArray.begin(), jsonArray.end(), std::back_inserter(result),
#ifdef QT_6_4_0
      [](QJsonValueConstRef value)
#else
      [](QJsonValueRef value)
#endif
      {return value.toDouble(); });
    return result;
  }

  IntVector ModelBaseJsonSerializer::ParseIntVector(const QJsonObject& json,
    std::string_view arrayTag) const
  {
    return ParseIntVector(json.value(arrayTag.data()).toArray());
  }

  IntVector ModelBaseJsonSerializer::ParseIntVector(const QJsonArray& jsonArray) const
  {
    IntVector result;
    std::transform(jsonArray.begin(), jsonArray.end(), std::back_inserter(result),
#ifdef QT_6_4_0
      [](QJsonValueConstRef value)
#else
      [](QJsonValueRef value)
#endif
      {return value.toDouble(); });
    return result;
  }

  OptionalDoubleVector ModelBaseJsonSerializer::ParseOptionalDoubleVector(const QJsonArray& jsonArray) const
  {
    OptionalDoubleVector result;
    std::transform(jsonArray.begin(), jsonArray.end(), std::back_inserter(result),
#ifdef QT_6_4_0
      [](QJsonValueConstRef value)->std::optional<double>
#else
      [](QJsonValueRef value) -> std::optional<double>
#endif
      {
        if (value.isNull())
          return std::nullopt;
        return std::optional<double>{value.toDouble()};
      });
    return result;
  }

  OptionalDoubleVector ModelBaseJsonSerializer::ParseOptionalDoubleVector(const QJsonObject& json, std::string_view tagName) const
  {
    return ParseOptionalDoubleVector(json.value(tagName.data()).toArray());
  }

  OptionalDouble2DVector ModelBaseJsonSerializer::ParseOptionalDouble2DVector(const QJsonObject& json, std::string_view tagName) const
  {
    OptionalDouble2DVector optDouble2d;
    const auto valarray = json.value(tagName.data()).toArray();
    std::transform(valarray.begin(), valarray.end(), std::back_inserter(optDouble2d),
#ifdef QT_6_4_0
      [&](QJsonValueConstRef value)
#else
      [&](QJsonValueRef value)
#endif
      {
        return ParseOptionalDoubleVector(value.toArray());
      });
    return optDouble2d;
  }

  Double2DVector ModelBaseJsonSerializer::ParseDouble2DVector(const QJsonObject& json, std::string_view tagName) const
  {
    const auto doubleArray = json.value(tagName.data()).toArray();
    Double2DVector array2d;
    std::transform(doubleArray.begin(), doubleArray.end(), std::back_inserter(array2d),
#ifdef QT_6_4_0
      [&](QJsonValueConstRef value)
#else
      [&](QJsonValueRef value)
#endif
      { return ParseDoubleVector(value.toArray()); });
    return array2d;
  }

  QJsonObject ModelBaseJsonSerializer::ParseSubObject(const QJsonObject& json, std::string_view tagName) const
  {
    return json.value(tagName.data()).toObject();
  }

  bool ModelBaseJsonSerializer::ParseBool(const QJsonObject& json, std::string_view boolTagName) const
  {
    return json.value(_Q(boolTagName.data())).toBool();
  }

  double ModelBaseJsonSerializer::ParseDouble(const QJsonObject& json, std::string_view tagName) const
  {
    return json.value(tagName.data()).toDouble();
  }

  std::string ModelBaseJsonSerializer::ParseString(const QJsonObject& json, std::string_view stringTagName) const
  {
    return _S(json.value(stringTagName.data()).toString());
  }

  size_t ModelBaseJsonSerializer::ParseUnsignedInt(const QJsonObject& json, std::string_view uintTagName) const
  {
    return static_cast<size_t>(json.value(uintTagName.data()).toInt());
  }

  StringList ModelBaseJsonSerializer::ParseStringList(const QJsonObject& json, std::string_view stringListTagName) const
  {
    const auto jsonStringArray = json.value(stringListTagName.data()).toArray();
    StringList stringList;
    std::transform(jsonStringArray.begin(), jsonStringArray.end(), std::back_inserter(stringList),
#ifdef QT_6_4_0
      [](QJsonValueConstRef value)
#else
      [](QJsonValueRef value)
#endif
      { return _S(value.toString()); });

    return stringList;
  }

  StringList2D ModelBaseJsonSerializer::ParseStringList2D(const QJsonObject& json, std::string_view stringListTagName) const
  {
    StringList2D string2dArray;
    const auto jsonStringArray = json.value(stringListTagName.data()).toArray();
    std::transform(jsonStringArray.begin(), jsonStringArray.end(), std::back_inserter(string2dArray),
#ifdef QT_6_4_0
      [](QJsonValueConstRef value)
#else
      [](QJsonValueRef value)
#endif
      {
        const auto list = value.toArray();
        StringList stringList;
        std::transform(list.begin(), list.end(), std::back_inserter(stringList),
#ifdef QT_6_4_0
          [&](QJsonValueConstRef value)
#else
          [&](QJsonValueRef value)
#endif
          { return _S(value.toString()); });
        return stringList;
      });
    return string2dArray;
  }

  JsonPair ModelBaseJsonSerializer::ComponentsToJsonPair(const IdentityModelPtr model,
    std::string_view componetsName) const
  {
    const auto components = model->Get<PlsComponents>(componetsName.data());
    return { JsonTagNames::Components, ComponetsToJsonArray(components) };
  }

  JsonPair ModelBaseJsonSerializer::TTitleToJsonPair(const IdentityModelPtr model,
    std::string_view ttitleName) const
  {
    const auto metaInfoList = model->Get<AnalysisMetaInfoList>(ttitleName.data());
    QJsonArray titleArray;
    std::transform(metaInfoList.begin(), metaInfoList.end(), std::back_inserter(titleArray),
      [&](const AnalysisMetaInfo& info) { return AnalysisMetaInfoToJson(info); });
    return { JsonTagNames::TTitle, titleArray };
  }

  JsonPair ModelBaseJsonSerializer::ByteDataToJsonPair(const IdentityModelPtr model,
    std::string_view fitName, std::string_view jsonTagName) const
  {
    const auto bytes = model->Get<ByteArray>(fitName.data());
    QByteArray aimodeldata;
    std::transform(bytes.cbegin(), bytes.cend(), std::back_inserter(aimodeldata),
      [](const std::byte& c) { return static_cast<char>(c); });
    return { _Q(jsonTagName.data()), QString(aimodeldata) };
  }

  JsonPair ModelBaseJsonSerializer::BoolToJsonPair(const IdentityModelPtr model, std::string_view boolModelName, std::string_view boolJsonTagName) const
  {
    return { boolJsonTagName.data(), model->Get<bool>(boolModelName.data()) };
  }

  JsonPair ModelBaseJsonSerializer::StringToJsonPair(const IdentityModelPtr model, std::string_view stringModelName, std::string_view stringJsonTagName) const
  {
    return { stringJsonTagName.data(), _Q(model->Get<std::string>(stringModelName.data())) };
  }

  JsonPair ModelBaseJsonSerializer::UnsignedIntToJsonPair(const IdentityModelPtr model, std::string_view uintModelName, std::string_view uintJsonTagName) const
  {
    return { uintJsonTagName.data(), static_cast<int>(model->Get<size_t>(uintModelName.data())) };
  }

  JsonPair ModelBaseJsonSerializer::IntToJsonPair(const IdentityModelPtr model, std::string_view uintModelName, std::string_view uintJsonTagName) const
  {
    return { uintJsonTagName.data(), model->Get<int>(uintModelName.data()) };
  }

  JsonPair ModelBaseJsonSerializer::StringListToJsonPair(const IdentityModelPtr model, std::string_view stringListModelName, std::string_view stringListJsonTagName) const
  {
    const auto stringList = model->Get<StringList>(stringListModelName.data());
    return { stringListJsonTagName.data(), StringListToJsonArray(stringList) };
  }

  JsonPair ModelBaseJsonSerializer::StringList2DToJsonPair(const IdentityModelPtr model, std::string_view stringListModelName, std::string_view stringListJsonTagName) const
  {
    const auto stringList2d = model->Get<StringList2D>(stringListModelName.data());
    QJsonArray jsonArray;
    std::transform(stringList2d.begin(), stringList2d.end(), std::back_inserter(jsonArray),
      [&](const StringList& list) { return StringListToJsonArray(list); });
    return { stringListJsonTagName.data(), {} };
  }

  QJsonArray ModelBaseJsonSerializer::StringListToJsonArray(const StringList& list)
  {
    QJsonArray arrayResult;
    std::transform(list.begin(), list.end(), std::back_inserter(arrayResult),
      [](const std::string& line) { return _Q(line); });
    return arrayResult;
  }

  QJsonObject ModelBaseJsonSerializer::MetaInfoToJsonObject(const MetaInfo& info) const
  {
    return
    {
      { JsonTagNames::Date,  _Q(info.Date) },
      { JsonTagNames::Field,  _Q(info.Field) },
      { JsonTagNames::FileId, static_cast<int>(info.FileId) },
      { JsonTagNames::SampleId, static_cast<int>(info.SampleId) },
      { JsonTagNames::FileName, _Q(info.FileName) },
      { JsonTagNames::Layer, _Q(info.Layer) },
      { JsonTagNames::Well, _Q(info.Well) },
      { JsonTagNames::WellCluster, _Q(info.WellCluster) },
    };
  }

  QJsonObject ModelBaseJsonSerializer::StatisticToJson(const IdentityModelPtr model,
    std::string_view statisticName,
    std::string_view provName,
    std::string_view gradName) const
  {
    const auto statistic = model->Get<Statistic>(statisticName.data());
    return
    {
      { provName.data(), statistic.Proverka},
      { gradName.data(), statistic.Graduirovka},
    };
  }

  QJsonObject ModelBaseJsonSerializer::DoubleVectorToJson(const IdentityModelPtr model,
    std::string_view vectorName, std::string_view jsonTagName) const
  {
    return
    {
      DoubleVectorToJsonPair(model, vectorName, jsonTagName)
    };
  }

  JsonPair ModelBaseJsonSerializer::DoubleVectorToJsonPair(const IdentityModelPtr model, std::string_view vectorName, std::string_view jsonTagName) const
  {
    return
    {
      jsonTagName.data(),
      DoubleVectorToJsonArray(model->Get<DoubleVector>(vectorName.data()))
    };
  }

  QJsonObject ModelBaseJsonSerializer::PlsComponentToJsonObject(const PlsComponent& component) const
  {
    return
    {
      { JsonTagNames::Number, static_cast<int>(component.Number) },
      { JsonTagNames::YProv, DoubleVectorToJsonArray(component.Y_prov) },
      { JsonTagNames::YGrad, DoubleVectorToJsonArray(component.Y_grad) },
      { JsonTagNames::Coefficients, DoubleVectorToJsonArray(component.Coefficients) },
      { JsonTagNames::YLoadings, DoubleVectorToJsonArray(component.Y_loadings) },
      { JsonTagNames::T, DoubleVectorToJsonArray(component.T) },
      { JsonTagNames::P, DoubleVectorToJsonArray(component.P) },
      { JsonTagNames::RMSEGrad, component.Statistic.RMSE.Graduirovka },
      { JsonTagNames::RMSEProv, component.Statistic.RMSE.Proverka },
      { JsonTagNames::R2Grad, component.Statistic.R2.Graduirovka },
      { JsonTagNames::R2Prov, component.Statistic.R2.Proverka },
      { JsonTagNames::SlopeGrad, component.Statistic.Slope.Graduirovka },
      { JsonTagNames::SlopeProv, component.Statistic.Slope.Proverka },
      { JsonTagNames::OffsetGrad, component.Statistic.Offset.Graduirovka },
      { JsonTagNames::OffsetProv, component.Statistic.Offset.Proverka },
    };
  }

  QJsonArray ModelBaseJsonSerializer::AnalysisMetaInfoListToJson(const AnalysisMetaInfoList& metaList) const
  {
    QJsonArray infoArray;
    std::transform(metaList.begin(), metaList.end(), std::back_inserter(infoArray),
      [&](const AnalysisMetaInfo& metaList) { return AnalysisMetaInfoToJson(metaList); });
    return infoArray;
  }

  QJsonObject ModelBaseJsonSerializer::AnalysisMetaInfoToJson(const AnalysisMetaInfo& metaInfo) const
  {
    auto metaJson = MetaInfoToJsonObject(metaInfo);
    if (metaInfo.YConcentration.has_value())
    {
      JSonCombiner combiner;
      metaJson = combiner.Combine(metaJson,
        {
          { JsonTagNames::YConcentration, metaInfo.YConcentration.value()} ,
        }
      );
    }
    return metaJson;
  }

  QJsonObject ModelBaseJsonSerializer::DoubleVector2DToJson(const Double2DVector& vec2d, std::string_view tagName) const
  {
    QJsonArray vec2dArray;
    std::transform(vec2d.begin(), vec2d.end(), std::back_inserter(vec2dArray),
      [&](const DoubleVector& vec) { return DoubleVectorToJsonArray(vec); });
    return { { tagName.data(), vec2dArray } };
  }

  QJsonArray ModelBaseJsonSerializer::StringListToJsonArray(const StringList& list) const
  {
    QJsonArray stringArray;
    std::transform(list.begin(), list.end(), std::back_inserter(stringArray),
      [&](const std::string& str) { return _Q(str); });
    return stringArray;
  }

  QJsonArray ModelBaseJsonSerializer::ComponetsToJsonArray(const PlsComponents& components) const
  {
    QJsonArray componentsArray;
    std::transform(components.begin(), components.end(), std::back_inserter(componentsArray),
      [&](const PlsComponent& comp) { return PlsComponentToJsonObject(comp); });
    return componentsArray;
  }

  QJsonObject ModelBaseJsonSerializer::AnalysisMetaInfoWithPeaksToJson(const AnalysisMetaInfoWithPeaks& data) const
  {
    JSonCombiner combiner;
    auto result = combiner.Combine(
      MetaInfoToJsonObject(data),
      {
        { JsonTagNames::Peaks, PeaksToJson(data.Peaks) }
      }
    );

    if (data.YConcentration.has_value())
      result = combiner.Combine(result, { { JsonTagNames::YConcentration, data.YConcentration.value()} });
    return result;
  }

  QJsonArray ModelBaseJsonSerializer::AnalysisMetaInfoWithPeaksListToJson(const AnalysisMetaInfoWithPeaksList& list) const
  {
    QJsonArray jsonArray;
    std::transform(list.begin(), list.end(), std::back_inserter(jsonArray),
      [&](const AnalysisMetaInfoWithPeaks& data) { return AnalysisMetaInfoWithPeaksToJson(data); });
    return jsonArray;
  }

  QJsonArray ModelBaseJsonSerializer::AnalysisMetaInfoWithPeaksListToJson(const IdentityModelPtr& model,
    std::string_view dataTag) const
  {
    return AnalysisMetaInfoWithPeaksListToJson(model->Get<AnalysisMetaInfoWithPeaksList>(dataTag.data()));
  }

  QJsonArray ModelBaseJsonSerializer::PeaksToJson(const MetaAnalysisPeaks& peaks) const
  {
    QJsonArray peakArray;
    std::transform(peaks.begin(), peaks.end(), std::back_inserter(peakArray),
      [&](const MetaAnalysisPeak& peak) { return PeakToJson(peak); });
    return peakArray;
  }

  QJsonObject ModelBaseJsonSerializer::PeakToJson(const MetaAnalysisPeak& peak) const
  {
    return
    {
      { JsonTagNames::Area, peak.Area },
      { JsonTagNames::Height, peak.Height },
      { JsonTagNames::Custom, peak.Custom },
      { JsonTagNames::Id, peak.Id },
      { JsonTagNames::IsMarker, peak.IsMarker },
      { JsonTagNames::Title, _Q(peak.Title)},
    };
  }

  QJsonArray ModelBaseJsonSerializer::DoubleVectorToJsonArray(const DoubleVector& vec) const
  {
    return QJsonArray::fromVariantList({ vec.begin(), vec.end() });
  }

  JsonPair ModelBaseJsonSerializer::DoubleVector2DToJson(const IdentityModelPtr model, std::string_view modelTagName, std::string_view jsonTagName) const
  {
    QJsonArray result;
    const auto input = model->Get<Double2DVector>(modelTagName.data());
    std::transform(input.begin(), input.end(), std::back_inserter(result),
      [&](const DoubleVector& vec) { return DoubleVectorToJsonArray(vec); });
    return { jsonTagName.data(), result };
  }

  QJsonArray ModelBaseJsonSerializer::DoubleOptionalVectorToJsonArray(const OptionalDoubleVector& vec) const
  {
    QJsonArray arrayResult;
    std::transform(vec.begin(), vec.end(), std::back_inserter(arrayResult),
      [](std::optional<double> value) -> QJsonValue
      {
        if (value.has_value())
          return value.value();
        return QJsonValue::Null;
      });
    return arrayResult;
  }

  JsonPair ModelBaseJsonSerializer::DoubleOptionalVectorToJsonPair(const IdentityModelPtr model, std::string_view modelTagName, std::string_view jsonTagName) const
  {
    return { jsonTagName.data(), DoubleOptionalVectorToJsonArray(model->Get<OptionalDoubleVector>(modelTagName.data())) };
  }

  JsonPair ModelBaseJsonSerializer::DoubleOptionalVector2DToJsonPair(const IdentityModelPtr model, std::string_view modelTagName, std::string_view jsonTagName) const
  {
    QJsonArray arrayResult;
    const auto opt2dvec = model->Get<OptionalDouble2DVector>(modelTagName.data());
    std::transform(opt2dvec.begin(), opt2dvec.end(), std::back_inserter(arrayResult),
      [&](const OptionalDoubleVector& vec) { return DoubleOptionalVectorToJsonArray(vec); });
    return { jsonTagName.data(), arrayResult };
  }

  QJsonArray ModelBaseJsonSerializer::DoubleOptionalVector2DToJsonArray(const OptionalDouble2DVector& vec) const
  {
    QJsonArray arrayResult;
    std::transform(vec.begin(), vec.end(), std::back_inserter(arrayResult),
      [&](const OptionalDoubleVector& vec) { return DoubleOptionalVectorToJsonArray(vec); });
    return arrayResult;
  }

  QJsonArray ModelBaseJsonSerializer::IntVectorToJsonArray(const IntVector& vec) const
  {
    return QJsonArray::fromVariantList({ vec.begin(), vec.end() });
  }

  JsonPair ModelBaseJsonSerializer::IntVectorToJsonPair(const IdentityModelPtr model, std::string_view modelTagName, std::string_view jsonTagName) const
  {
    return { jsonTagName.data(), IntVectorToJsonArray(model->Get<IntVector>(modelTagName.data())) };
  }

  ByteArray ModelBaseJsonSerializer::ParseByteData(const QJsonObject& json,
    std::string_view byteDataName) const
  {
    const auto data = json.value(byteDataName.data()).toString().toUtf8();
    ByteArray aimodel;
    std::transform(data.cbegin(), data.cend(), std::back_inserter(aimodel),
      [](const char& c) { return static_cast<std::byte>(c); });
    return aimodel;
  }
}
