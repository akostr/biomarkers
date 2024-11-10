#include "pls_predict_parent_analysis_list_json_serializer.h"

#include "logic/models/pls_predict_parent_analysis_list_model.h"
#include "logic/known_json_tag_names.h"

#include <QJsonArray>

#define _Q(str) QString::fromStdString(str)
#define _S(str) str.toStdString()

namespace Service
{
  QJsonObject PlsPredictParentAnalysisListJsonSerializer::ToJson(const IdentityModelPtr model) const
  {
    return
    {
      { "pls_predict_parent_analysis_list", ParentAnalysisIdsToArray(model) },
    };
  }

  IdentityModelPtr PlsPredictParentAnalysisListJsonSerializer::ToModel(const QJsonObject& json) const
  {
    const auto model = std::make_shared<PlsPredictParentAnalysisListModel>();
    model->Set<ParentAnalysisIds>(PlsPredictParentAnalysisListModel::ParentList, ParseParentAnalysisIds(json));
    return model;
  }

  ParentAnalysisIds PlsPredictParentAnalysisListJsonSerializer::ParseParentAnalysisIds(const QJsonObject& json) const
  {
    ParentAnalysisIds list;
    const auto dataArray = json.value(JsonTagNames::Result).toArray();
    std::transform(dataArray.begin(), dataArray.end(), std::back_inserter(list),
#ifdef QT_6_4_0
      [&](QJsonValueConstRef value)
#else
      [&](QJsonValueRef value)
#endif
    { return ParseParentAnalysisId(value.toObject()); });
    return list;
  }

  ParentAnalysisId PlsPredictParentAnalysisListJsonSerializer::ParseParentAnalysisId(const QJsonObject& json) const
  {
    return
    {
      ParseUnsignedInt(json, _S(JsonTagNames::Id)),
      ParseString(json, _S(JsonTagNames::Title)),
      ParseString(json, _S(JsonTagNames::Layer)),
      ParseIntVector(json.value(JsonTagNames::CheckedMarkersIds).toArray()),
    };
  }

  QJsonArray PlsPredictParentAnalysisListJsonSerializer::ParentAnalysisIdsToArray(const IdentityModelPtr model) const
  {
    QJsonArray result;
    model->Exec<ParentAnalysisIds>(PlsPredictParentAnalysisListModel::ParentList,
      [&](const ParentAnalysisIds& list)
      {
        std::transform(list.begin(), list.end(), std::back_inserter(result),
        [&](const ParentAnalysisId& item)
          {
            return QJsonObject
            {
              { JsonTagNames::Id, static_cast<int>(item.Id) },
              { JsonTagNames::Title, _Q(item.Title) },
              { JsonTagNames::Layer, _Q(item.LayerName) },
              { JsonTagNames::CheckedMarkersIds, IntVectorToJsonArray(item.CheckedMarkerIds)},
            };
          });
      });
    return result;
  }
}
