#include "entity_model_json_serializer.h"


#include "logic/models/analysis_identity_model.h"
#include "logic/models/analysis_entity_model.h"
#include "logic/known_json_tag_names.h"
#include "logic/serializers/json_combiner.h"
#include "ui/plots/gp_items/gpshape_item.h"
#include <string>

#include <QJsonArray>
#include <QJsonDocument>
#include <QMetaEnum>
#include <QColor>
#include <QPointer>

#define _Q(str) QString::fromStdString(str)
#define _S(str) str.toStdString()

namespace Service
{

QJsonObject EntityModelJsonSerializer::ToJson(const IdentityModelPtr model) const
{
  auto entityModel = model->Get<QPointer<AnalysisEntityModel>>(Service::AnalysisIdentityModel::EntityModel);
  return AnalysisEntityModelToJson(entityModel);
}

IdentityModelPtr EntityModelJsonSerializer::ToModel(const QJsonObject& json) const
{
  const auto model = std::make_shared<AnalysisIdentityModel>();

  model->Set(AnalysisIdentityModel::EntityModel, ParseAnalysisEntityModel(json));
  return model;
}

IdentityModelPtr EntityModelJsonSerializer::GenerateFromRequest(const QJsonObject& json, int libraryGroupIdForX, int libraryGroubIdForY)
{
  const auto model = std::make_shared<AnalysisIdentityModel>();

  model->Set(AnalysisIdentityModel::EntityModel,
             ParseAnalysisEntityModelFromRequest(json,
                                                 libraryGroupIdForX,
                                                 libraryGroubIdForY));
  return model;
}

QPointer<AnalysisEntityModel> EntityModelJsonSerializer::ParseAnalysisEntityModelFromRequest
  (const QJsonObject &json, int libraryGroupIdForX, int libraryGroupIdForY) const
{
  using namespace AnalysisEntity;
  if(json.isEmpty() && json.contains("result"))
    return nullptr;

  TPassportFilter defaultColorFilter;//by layer
  defaultColorFilter[PassportTags::layer] = "grouping by";//exact value can be any non-empty
  TPassportFilter defaultShapeFilter;//no filter

  QList<TLegendGroup> groupsInfo;
  int currentShapeInd = 0;
  int currentColorInd = 0;
  auto ensureGroupEnlisted = [&defaultColorFilter, &defaultShapeFilter, &groupsInfo, &currentShapeInd, &currentColorInd](const TPassport& passport)
  {
    auto title = AnalysisEntityModel::genericGroupName(defaultColorFilter, defaultShapeFilter, passport);
    auto iter = std::find_if(groupsInfo.begin(), groupsInfo.end(), [title](const TLegendGroup& groupInfo)->bool
                             {
                               return groupInfo.title == title;
                             });
    if(iter == groupsInfo.end())
    {
      TPassportFilter filter;
      filter[PassportTags::layer] = passport.value(PassportTags::layer);
      groupsInfo << TLegendGroup
        {
          title, title,
          GPShapeItem::GetCycledColor(currentColorInd++),
          { GPShape::GetPath(GPShapeItem::GetCycledShape(currentShapeInd++)), 10 },
          filter
        };
    }
  };

  QJsonObject result = json["result"].toObject();
  QPointer<AnalysisEntityModel> analysisModel = new AnalysisEntityModel();

  QMap<int, QList<double>> valuesForLibraryIds;
  QMap<int, QList<double>> normedValuesForLibraryIds;
  QJsonArray components = result["components"].toArray();
  for (auto item: components)
  {
    QJsonObject component = item.toObject();
    int id = component["library_element_id"].toInt();
    auto values = component["column_values"].toArray();
    for(auto value: values)
    {
      valuesForLibraryIds[id].push_back(value.toDouble());
    }
    auto normedValues = component["column_values_normed"].toArray();
    for (auto value : normedValues)
    {
      normedValuesForLibraryIds[id].push_back(value.toDouble());
    }
  }
  QStringList pk, ph;
  auto jpk = result[JsonTagNames::PassportKeys].toArray();
  auto jph = result[JsonTagNames::PassportHeaders].toArray();
  for(int i = 0; i < jpk.size(); i++)
    pk << jpk[i].toString();
  for(int i = 0; i < jph.size(); i++)
    ph << jph[i].toString();


  QVector <AnalysisMetaInfo> infoList;
  QJsonArray Ttitle = result["T_title"].toArray();
  for (int i = 0; i < Ttitle.count(); i++)
  {
    auto tTitle = Ttitle[i].toObject();

    AnalysisMetaInfo t_title = ParseMetaInfo(tTitle);
    double x = valuesForLibraryIds[libraryGroupIdForX][i];
    double y = valuesForLibraryIds[libraryGroupIdForY][i];
    TPassport passport;
    for (auto key: tTitle.keys())
    {
      passport[key] = tTitle[key].toVariant();
    }
    auto title = AnalysisEntityModel::passportToSampleTitle(passport);
    DataModel entityData;
    entityData[RolePassport] = QVariant::fromValue(passport);
    entityData[RoleTitle] = title;
    entityData[RoleGroupTooltip] = title;
    entityData[RoleIntId] = t_title.SampleId;
    entityData[RoleExcluded] = false;
    entityData[RoleOrderNum] = i;
    TComponentsData components;
    components[libraryGroupIdForX] = x;
    components[libraryGroupIdForY] = y;
    if (!normedValuesForLibraryIds.isEmpty())
    {
      TNormedComponentsData normedComponents;
      double normedX = normedValuesForLibraryIds[libraryGroupIdForX][i];
      double normedY = normedValuesForLibraryIds[libraryGroupIdForY][i];
      normedComponents[libraryGroupIdForX] = normedX;
      normedComponents[libraryGroupIdForY] = normedY;
      entityData[RoleNormedComponentData] = QVariant::fromValue(normedComponents);
    }
    entityData[RoleComponentsData] = QVariant::fromValue(components);
    entityData[RoleShape] = QVariant::fromValue(GPShape::GetPath(GPShape::Circle));
    entityData[RoleShapePixelSize] = 8;
    entityData[RoleColor] = GPShapeItem::ToColor(GPShapeItem::Blue);
    ensureGroupEnlisted(passport);
    analysisModel->addNewEntity(TypeSample, entityData);
  }

  analysisModel->setModelData(ModelRolePassportKeys, pk);
  analysisModel->setModelData(ModelRolePassportHeaders, ph);
  analysisModel->setCurrentXYComponent(libraryGroupIdForX,libraryGroupIdForY);
  analysisModel->regroup(defaultColorFilter, defaultShapeFilter, groupsInfo);
  return analysisModel;
}

}
