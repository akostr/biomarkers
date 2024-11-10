#include "pls_analysis_list_json_serializer.h"

#include "logic/models/pls_analysis_list_model.h"
#include "logic/known_json_tag_names.h"
#include "logic/enums.h"

#include <QJsonArray>

using namespace Constants;
using namespace Model;

#define _Q(str) QString::fromStdString(str)
#define _S(str) str.toStdString()

namespace Service
{
  QJsonObject PlsAnalysisListSerializer::ToJson(const IdentityModelPtr model) const
  {
    QJsonArray jsonArray;
    model->ForEach<PlsModels>([&](const PlsModels& models)
      {
        for (const auto& item : models)
        {
          jsonArray.append(PlsModelToJsonArray(item));
        }
      });
    return { { JsonTagNames::Children, jsonArray} };
  }

  IdentityModelPtr PlsAnalysisListSerializer::ToModel(const QJsonObject& json) const
  {
    const auto container = std::make_shared<PlsAnalysisListModel>();
    PlsModels models;
    PlsModels predicts;
    const auto tagByIndex = ParseColumnNames(json);
    const auto dataRows = json.value(JsonTagNames::Data).toArray();
    for (const auto& row : dataRows)
    {
      const auto jsonArray = row.toArray();
      if (const auto model = ParsePlsModel(jsonArray, tagByIndex);
        model.ParentId)
        predicts.emplace_back(model);
      else
        models.emplace_back(model);
    }

    container->Set<PlsModels>(PlsAnalysisListModel::PlsModels, models);
    container->Set<PlsModels>(PlsAnalysisListModel::PlsPredicts, predicts);

    return container;
  }

  StringMap PlsAnalysisListSerializer::ParseColumnNames(const QJsonObject& jsonObject) const
  {
    const auto columnArray = jsonObject.value(JsonTagNames::Columns).toArray();
    StringMap tags;
    for (qsizetype index = 0; index < columnArray.size(); index++)
    {
      tags.emplace(ParseString(columnArray[index].toObject(), _S(JsonTagNames::Name)), index);
    }
    return tags;
  }

  PlsModel PlsAnalysisListSerializer::ParsePlsModel(const QJsonArray& json, const StringMap& columnList) const
  {
    const auto parentID = ParseColumnValues(json, columnList, _S(JsonTagNames::ParentId));
    const auto parameters = ParseColumnValues(json, columnList, _S(JsonTagNames::_analysis_parameters)).toJsonObject();
    return
    {
      static_cast<size_t>(ParseColumnValues(json, columnList, _S(JsonTagNames::_analysis_id)).toInt()),
      static_cast<size_t>(ParseColumnValues(json, columnList, _S(JsonTagNames::AnalysisTableId)).toInt()),
      static_cast<size_t>(parameters.value(JsonTagNames::chosen_pc_count).toInt()),
      static_cast<size_t>(ParseColumnValues(json, columnList, _S(JsonTagNames::AnalysisPeaksCount)).toInt()),
      static_cast<size_t>(ParseColumnValues(json, columnList, _S(JsonTagNames::AnalysisSamplesCount)).toInt()),
      static_cast<size_t>(parentID.isNull() ? 0 : parentID.toInt()),
      static_cast<size_t>(ParseColumnValues(json, columnList, _S(JsonTagNames::AnalysisVersionId)).toInt()),
      static_cast<AnalysisType>(ParseColumnValues(json, columnList, _S(JsonTagNames::AnalysisTypeId)).toInt()),
      _S(ParseColumnValues(json, columnList, _S(JsonTagNames::AnalysisType)).toString()),
      _S(ParseColumnValues(json, columnList, _S(JsonTagNames::AnalysisTitle)).toString()),
      _S(ParseColumnValues(json, columnList, _S(JsonTagNames::AnalysisTableTitle)).toString()),
      _S(ParseColumnValues(json, columnList, _S(JsonTagNames::AnalysisComment)).toString()),
      _S(ParseColumnValues(json, columnList, _S(JsonTagNames::AnalysisUser)).toString()),
      _S(ParseColumnValues(json, columnList, _S(JsonTagNames::AnalysisCreateDate)).toString()),
      _S(ParseColumnValues(json, columnList, _S(JsonTagNames::AnalysisVersionTitle)).toString()),
      _S(ParseColumnValues(json, columnList, _S(JsonTagNames::ParentTitle)).toString()),
    };
  }

  QVariant PlsAnalysisListSerializer::ParseColumnValues(const QJsonArray& json,
    const StringMap& columnList, const std::string& tag) const
  {
    if (const auto columnIterator = columnList.find(tag);
      columnIterator != columnList.end())
      return json[columnIterator->second];

    return QVariant();
  }

  /*
  * sequence should be the same as PlsModelColumnList
  */
  QJsonArray PlsAnalysisListSerializer::PlsModelToJsonArray(const PlsModel& item) const
  {
    return
    {
      /*_Q(KnownJsonTagNames::AnalysisComment),*/ _Q(item.Comments),
      /*_Q(KnownJsonTagNames::AnalysisId),*/ static_cast<int>(item.Id),
      /*_Q(KnownJsonTagNames::AnalysisTitle),*/ _Q(item.Name),
      /*_Q(KnownJsonTagNames::AnalysisType),*/ _Q(item.TypeName),
      /*_Q(KnownJsonTagNames::AnalysisTypeId),*/ static_cast<int>(item.Type),
      /*_Q(KnownJsonTagNames::AnalysisCreateDate),*/ _Q(item.CreateDate),
      /*_Q(KnownJsonTagNames::AnalysisMarkupId),*/ static_cast<int>(item.TableId),
      /*_Q(KnownJsonTagNames::AnalysisMarkupTitle), */_Q(item.TableTitle),
      /*_Q(KnownJsonTagNames::AnalysisParentId), */static_cast<int>(item.ParentId),
      /*_Q(KnownJsonTagNames::AnalysisPeaksCount),*/ static_cast<int>(item.PeaksCount),
      /*_Q(KnownJsonTagNames::AnalysisSamplesCount),*/ static_cast<int>(item.SamplesCount),
      /*_Q(KnownJsonTagNames::AnalysisUser),*/ _Q(item.Author),
      /*_Q(KnownJsonTagNames::AnalysisTableId),*/ static_cast<int>(item.VersionId),
      /*_Q(KnownJsonTagNames::AnalysisVersionTitle),*/ _Q(item.VersionTitle),
      /*_Q(KnownJsonTagNames::ParentTitle),*/ _Q(item.ParentTitle),
    };
  }

  /*
  * sequence should be the same as PlsModelToJson
  */
  QJsonArray PlsAnalysisListSerializer::PlsModelColumnList() const
  {
    return
    {
      ColumnObject(_S(JsonTagNames::AnalysisComment), "text"),
      ColumnObject(_S(JsonTagNames::_analysis_id), "int4"),
      ColumnObject(_S(JsonTagNames::AnalysisTitle), "text"),
      ColumnObject(_S(JsonTagNames::AnalysisType), "text"),
      ColumnObject(_S(JsonTagNames::AnalysisTypeId), "int4"),
      ColumnObject(_S(JsonTagNames::AnalysisCreateDate), "timestamp"),
      ColumnObject(_S(JsonTagNames::AnalysisTableId), "int4"),
      ColumnObject(_S(JsonTagNames::AnalysisTableTitle), "text"),
      ColumnObject(_S(JsonTagNames::AnalysisParentId), "int4"),
      ColumnObject(_S(JsonTagNames::AnalysisPeaksCount), "int4"),
      ColumnObject(_S(JsonTagNames::AnalysisSamplesCount), "int4"),
      ColumnObject(_S(JsonTagNames::AnalysisUser), "text"),
      ColumnObject(_S(JsonTagNames::AnalysisVersionId), "int4"),
      ColumnObject(_S(JsonTagNames::AnalysisVersionTitle), "text"),
    };
  }

  QJsonObject PlsAnalysisListSerializer::ColumnObject(std::string_view tagName,
    const std::string& type) const
  {
    return
    {
      { JsonTagNames::Name, tagName.data() },
      { JsonTagNames::TypName, _Q(type) }
    };
  }
}