#include "hotteling_response_json_serializer.h"

#include "logic/models/hotteling_response_model.h"
#include "logic/known_json_tag_names.h"

using namespace Model;

namespace Service
{
  QJsonObject HottelingResponseJsonSerializer::ToJson(const IdentityModelPtr model) const
  {
    return
    {
      StringListToJsonPair(model, HottelingResponseModel::Headers, JsonTagNames::Headers.toStdString()),
      StringList2DToJsonPair(model, HottelingResponseModel::Values, JsonTagNames::Values.toStdString()),
      StringList2DToJsonPair(model, HottelingResponseModel::Colors, JsonTagNames::Colors.toStdString()),
    };
  }

  IdentityModelPtr HottelingResponseJsonSerializer::ToModel(const QJsonObject& json) const
  {
    auto model = std::make_shared<HottelingResponseModel>();
    model->Set(HottelingResponseModel::Headers,
      ParseStringList(json, JsonTagNames::ValuesHeaders.toStdString()));

    model->Set(HottelingResponseModel::Values, ParseStringList2D(json, JsonTagNames::Values.toStdString()));
    model->Set(HottelingResponseModel::Colors, ParseStringList2D(json, JsonTagNames::Colors.toStdString()));
    return model;
  }
}