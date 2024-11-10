#include "hotteling_request_json_serializer.h"

#include "logic/models/hotteling_request_model.h"
#include "logic/known_json_tag_names.h"
#include "logic/serializers/json_combiner.h"

using namespace Model;
using namespace Structures;

namespace Service
{
  QJsonObject HottelingRequestJsonSerialzier::ToJson(const IdentityModelPtr model) const
  {
    return
    {
      UnsignedIntToJsonPair(model, HottelingRequestModel::Count, JsonTagNames::Count.toStdString()),
      HottelingDataToJson(model)
    };
  }

  IdentityModelPtr HottelingRequestJsonSerialzier::ToModel(const QJsonObject& json) const
  {
    auto model = std::make_shared<HottelingRequestModel>();
    model->Set(HottelingRequestModel::Count, ParseUnsignedInt(json, JsonTagNames::Count.toStdString()));
    model->Set(HottelingRequestModel::T, ParseHottelingData(json, JsonTagNames::T.toStdString()));
    return model;
  }

  HottelingData HottelingRequestJsonSerialzier::ParseHottelingData(const QJsonObject& json, std::string_view tag) const
  {
    HottelingData data;
    for (const auto& key : json.keys())
    {
      const auto stdKey = key.toStdString();
      data.emplace(stdKey, ParseDouble2DVector(json, stdKey));
    }
    return data;
  }

  JsonPair HottelingRequestJsonSerialzier::HottelingDataToJson(const IdentityModelPtr model) const
  {
    JsonPair result;
    model->Exec<HottelingData>(HottelingRequestModel::T,
      [&](const HottelingData& data)
      {
        JSonCombiner comb;
        QJsonObject groups;
        for (const auto& [key, values] : data)
        {
          groups = comb.Combine(groups, DoubleVector2DToJson(values, key));
        }
        result = { JsonTagNames::T, groups };
      });
    return result;
  }
}