#include "markup_reference_model_json_serializer.h"

#include <QJsonArray>

#include "logic/known_json_tag_names.h"
#include "logic/models/markup_reference_model.h"

using namespace Structures;
using namespace Model;

#define _Q(str) QString::fromStdString(str)
#define _S(str) str.toStdString()

namespace Service
{
  QJsonObject MarkupReferenceModelJsonSerializer::ToJson(const IdentityModelPtr model) const
  {
    QJsonArray jsonArray;
    model->ForEach<ReferenceList>([&](const ReferenceList& models)
      {
        for (const auto& item : models)
        {
          jsonArray.append(ReferenceToJsonArray(item));
        }
      });
    return { { JsonTagNames::Children, jsonArray} };
  }

  IdentityModelPtr MarkupReferenceModelJsonSerializer::ToModel(const QJsonObject& json) const
  {
    const auto model = std::make_shared<MarkupReferenceModel>();
    const auto tagByIndex = ParseColumnNames(json);
    const auto dataRows = json.value(JsonTagNames::Data).toArray();
    ReferenceList list;
    std::transform(dataRows.begin(), dataRows.end(), std::back_inserter(list),
#ifdef QT_6_4_0
      [&](QJsonValueConstRef value)
#else
      [&](QJsonValueRef value)
#endif
    { return ParseReference(value.toArray(), tagByIndex); });
    model->Set<ReferenceList>(MarkupReferenceModel::ReferenceList, list);
    return model;
  }

  QJsonArray MarkupReferenceModelJsonSerializer::ReferenceToJsonArray(const Reference& reference) const
  {
    return
    {
      static_cast<int>(reference.Id),
      static_cast<int>(reference.ProjectFileId),
      static_cast<int>(reference.CreatedMarkupId),
      _Q(reference.Title),
      _Q(reference.Comment),
      _Q(reference.MarkupTitle),
      _Q(reference.Changed),
      _Q(reference.Author),
    };
  }

  StringMap MarkupReferenceModelJsonSerializer::ParseColumnNames(const QJsonObject& jsonObject) const
  {
    const auto columnArray = jsonObject.value(JsonTagNames::Columns).toArray();
    StringMap tags;
    for (qsizetype index = 0; index < columnArray.size(); index++)
    {
      tags.emplace(ParseString(columnArray[index].toObject(), _S(JsonTagNames::Name)), index);
    }
    return tags;
  }

  Reference MarkupReferenceModelJsonSerializer::ParseReference(const QJsonArray& json, const StringMap& columnList) const
  {
    return
    {
      static_cast<size_t>(ParseColumnValues(json, columnList, _S(JsonTagNames::_id)).toInt()),
      static_cast<size_t>(ParseColumnValues(json, columnList, _S(JsonTagNames::_project_file_id)).toInt()),
      static_cast<size_t>(ParseColumnValues(json, columnList, _S(JsonTagNames::_created_markup_id)).toInt()),
      _S(ParseColumnValues(json,columnList, _S(JsonTagNames::_title)).toString()),
      _S(ParseColumnValues(json,columnList, _S(JsonTagNames::_comment)).toString()),
      _S(ParseColumnValues(json,columnList, _S(JsonTagNames::AnalysisMarkupTitle)).toString()),
      _S(ParseColumnValues(json,columnList, _S(JsonTagNames::_changed)).toString()),
      _S(ParseColumnValues(json,columnList, _S(JsonTagNames::Author)).toString()),
    };
  }

  QVariant MarkupReferenceModelJsonSerializer::ParseColumnValues(const QJsonArray& json, const StringMap& columnList, const std::string& tag) const
  {
    if (const auto columnIterator = columnList.find(tag);
      columnIterator != columnList.end())
      return json[columnIterator->second];

    return QVariant();
  }
}
