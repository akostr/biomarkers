#pragma once
#include "analysis_identity_model_json_serializer.h"
#include "logic/structures/analysis_model_structures.h"

class AnalysisEntityModel;
namespace Service
{

using namespace Structures;

class EntityModelJsonSerializer : public AnalysisIdentityModelJsonSerializer
{
public:
  QJsonObject ToJson(const IdentityModelPtr model) const override;
  IdentityModelPtr ToModel(const QJsonObject& json) const override;
  IdentityModelPtr GenerateFromRequest(const QJsonObject& json, int libraryGroupIdForX, int libraryGroubIdForY);

protected:
  QPointer<AnalysisEntityModel> ParseAnalysisEntityModelFromRequest(const QJsonObject &json, int libraryGroupIdForX, int libraryGroubIdForY) const;
};

}
