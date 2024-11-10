#include "analysis_entity_model.h"
#include <QMetaEnum>
#include <QJsonDocument>
#include <QJsonArray>
#include <QColor>
using namespace AnalysisEntity;

AnalysisEntityModel::AnalysisEntityModel(QObject *parent)
  : ShapeColorGroupedEntityPlotDataModel(parent)
{
  // qDebug() << "hello world (AnalysisEntityModel)";
}

AnalysisEntityModel::~AnalysisEntityModel()
{
  // qDebug() << "goodbye world (AnalysisEntityModel)";
}

QPointF AnalysisEntityModel::currentEntityCoord(TEntityUid uid)
{
  auto ent = getEntity(uid);
  if(!ent)
    return QPointF();
  int xcomp = currentXComponent();
  int ycomp = currentYComponent();
  auto compData = ent->getData(RoleComponentsData).value<TComponentsData>();
  if(!compData.contains(xcomp) ||
      !compData.contains(ycomp))
    return QPointF();
  return {compData.value(xcomp),
          compData.value(ycomp)};
}

double AnalysisEntityModel::currentEntitySpectralValue(TEntityUid uid)
{
  auto ent = getEntity(uid);
  if(!ent)
    return 0;
  int comp = currentSpectralComponent();
  auto compData = ent->getData(RoleComponentsData).value<TComponentsData>();
  if(!compData.contains(comp))
    return 0;
  return compData.value(comp);
}

void AnalysisEntityModel::setExplVarianceMap(const TComponentsExplVarianceMap &map)
{
  mData[ModelRoleComponentsExplVarianceMap] = QVariant::fromValue(map);
  emit modelDataChanged({{ModelRoleComponentsExplVarianceMap, mData[ModelRoleComponentsExplVarianceMap]}});
}

double AnalysisEntityModel::currentXExplVariance()
{
  if(!mData.contains(ModelRoleComponentsExplVarianceMap) ||
      !mData[ModelRoleComponentsExplVarianceMap]
           .value<TComponentsExplVarianceMap>().contains(currentXComponent()))
    return INVALID_EXPL_VARIANCE;
  return mData[ModelRoleComponentsExplVarianceMap]
      .value<TComponentsExplVarianceMap>()[currentXComponent()];
}

double AnalysisEntityModel::currentYExplVariance()
{
  if(!mData.contains(ModelRoleComponentsExplVarianceMap) ||
      !mData[ModelRoleComponentsExplVarianceMap]
           .value<TComponentsExplVarianceMap>().contains(currentYComponent()))
    return INVALID_EXPL_VARIANCE;
  return mData[ModelRoleComponentsExplVarianceMap]
      .value<TComponentsExplVarianceMap>()[currentYComponent()];
}

void AnalysisEntityModel::setCurrentXComponent(int comp)
{
  if(mData.value(ModelRoleCurrentXComponentNum, INVALID_COMPONENT).toInt() == comp)
    return;
  mData[ModelRoleCurrentXComponentNum] = comp;
  emit modelDataChanged({{ModelRoleCurrentXComponentNum, comp}});
}

void AnalysisEntityModel::setCurrentYComponent(int comp)
{
  if(mData.value(ModelRoleCurrentYComponentNum, INVALID_COMPONENT).toInt() == comp)
    return;
  mData[ModelRoleCurrentYComponentNum] = comp;
  emit modelDataChanged({{ModelRoleCurrentXComponentNum, comp}});
}

void AnalysisEntityModel::setCurrentSpectralComponent(int comp)
{
  if(mData.value(ModelRoleCurrentSpectralComponentNum, INVALID_COMPONENT).toInt() == comp)
    return;
  mData[ModelRoleCurrentSpectralComponentNum] = comp;
  emit modelDataChanged({{ModelRoleCurrentSpectralComponentNum, comp}});
}

void AnalysisEntityModel::setCurrentXYComponent(int compx, int compy)
{
  QHash<int, QVariant> roleDataMap;
  if(mData.value(ModelRoleCurrentXComponentNum, INVALID_COMPONENT).toInt() != compx)
  {
    mData[ModelRoleCurrentXComponentNum] = compx;
    roleDataMap.insert(ModelRoleCurrentXComponentNum, compx);
  }
  if(mData.value(ModelRoleCurrentYComponentNum, INVALID_COMPONENT).toInt() != compy)
  {
    mData[ModelRoleCurrentYComponentNum] = compy;
    roleDataMap.insert(ModelRoleCurrentYComponentNum, compy);
  }
  emit modelDataChanged(roleDataMap);
}

void AnalysisEntityModel::setColorModelFilter(const TPassportFilter &colorFilter)
{
  mData[ModelRoleColorFilter] = QVariant::fromValue(colorFilter);
  emit modelDataChanged({{ModelRoleColorFilter, mData[ModelRoleColorFilter] }});
}

void AnalysisEntityModel::setShapeModelFilter(const TPassportFilter &shapeFilter)
{
  mData[ModelRoleShapeFilter] = QVariant::fromValue(shapeFilter);
  emit modelDataChanged({{ModelRoleShapeFilter, mData[ModelRoleShapeFilter]}});
}

void AnalysisEntityModel::setCurrentModelFilters(const TPassportFilter &colorFilter, const TPassportFilter &shapeFilter)
{
  mData[ModelRoleColorFilter] = QVariant::fromValue(colorFilter);
  mData[ModelRoleShapeFilter] = QVariant::fromValue(shapeFilter);
  emit modelDataChanged({{ModelRoleColorFilter, mData[ModelRoleColorFilter] },
                         {ModelRoleShapeFilter, mData[ModelRoleShapeFilter]}});
}

int AnalysisEntityModel::currentXComponent()
{
  return mData.value(ModelRoleCurrentXComponentNum, INVALID_COMPONENT).toInt();
}

int AnalysisEntityModel::currentYComponent()
{
  return mData.value(ModelRoleCurrentYComponentNum, INVALID_COMPONENT).toInt();
}

int AnalysisEntityModel::currentSpectralComponent()
{
  return mData.value(ModelRoleCurrentSpectralComponentNum, INVALID_COMPONENT).toInt();
}

void AnalysisEntityModel::regroup(TPassportFilter colorFilter,
  TPassportFilter shapeFilter,
  const QList<TLegendGroup>& legendGroups)
{
  removeEntitiesIf([](AnalysisEntityModel::ConstDataPtr item)
    {
      return item->getType() == TypeGroup && !item->getData(RoleGroupUserCreated).toBool();
    });

  std::map<TEntityUid, TPassportFilter> newGroups;
  for (const auto& group : legendGroups)
  {
    auto groupUid = addNewEntity(TypeGroup);
    setEntityData(groupUid, RoleTitle, group.title);
    setEntityData(groupUid, RoleGroupTooltip, group.tooltip);
    setEntityData(groupUid, RoleColor, group.color);
    setEntityData(groupUid, RoleShape, QVariant::fromValue(group.shape.path));
    setEntityData(groupUid, RoleShapePixelSize, group.shape.pixelSize);
    setEntityGroup(group.uid, groupUid);
    newGroups.emplace(groupUid, group.filter);
  }

  if(!colorFilter.isEmpty() || !shapeFilter.isEmpty())
  {
    auto nonUserGroupedSamples = getEntities(
      [&](AbstractEntityDataModel::ConstDataPtr ent)->bool
      {
        const auto groupUids = ent->getData(RoleGroupsUidSet).value<TGroupsUidSet>();
        for (auto& uid : groupUids)
          if (getEntity(uid)->getData(RoleGroupUserCreated).toBool())
            return false;
        return ent->getType() == TypeSample;
      });

    for (const auto& [groupUid, filter] : newGroups)
    {
      setEntityData(groupUid, RolePassportFilter, QVariant::fromValue(filter));
      for (auto& sample : nonUserGroupedSamples)
      {
        if (filter.match(sample->getData(RolePassport).value<TPassport>()))
          setEntityGroup(sample->getUid(), groupUid);
      }
    }
  }
  setCurrentModelFilters(colorFilter, shapeFilter);
}

void AnalysisEntityModel::cancelExclude(EntityType type)
{
  auto excludedItems = getEntities({{RoleEntityType, type},
                                    {RoleExcluded, true}});
  if(excludedItems.isEmpty())
    return;
  beginResetModel();
  for(auto& ent : excludedItems)
    setEntityData(ent->getUid(), RoleExcluded, false);
  endResetModel();
}

const QHash<int, QVariant> &AnalysisEntityModel::modelData()
{
  return mData;
}

void AnalysisEntityModel::setModelData(const QHash<int, QVariant> &modelData)
{
  beginResetModel();
  mData = modelData;
  endResetModel();
}

void AnalysisEntityModel::setModelData(int role, const QVariant &value)
{
  switch(role)
  {
    case ModelRoleCurrentXComponentNum:
      setCurrentXComponent(value.toInt());
      break;
    case ModelRoleCurrentYComponentNum:
      setCurrentYComponent(value.toInt());
      break;
    case ModelRoleComponentsExplVarianceMap:
      setExplVarianceMap(value.value<TComponentsExplVarianceMap>());
      break;
    case ModelRoleColorFilter:
      setColorModelFilter(value.value<TPassportFilter>());
      break;
    case ModelRoleShapeFilter:
      setShapeModelFilter(value.value<TPassportFilter>());
      break;
    case ModelRoleIsSpectral:
      if(mData[ModelRoleIsSpectral] != value)
      {
        mData[ModelRoleIsSpectral] = value;
        emit modelDataChanged({{ModelRoleIsSpectral, mData[ModelRoleIsSpectral]}});
      }
      break;
    case ModelRoleSpectralRenderType:
      if(mData[ModelRoleSpectralRenderType] != value)
      {
        mData[ModelRoleSpectralRenderType] = value;
        emit modelDataChanged({{ModelRoleSpectralRenderType, mData[ModelRoleSpectralRenderType]}});
      }
      break;
    case ModelRoleLast:
      break;
    default:
      if(mData[role] != value)
      {
        mData[role] = value;
        emit modelDataChanged({{role, mData[role]}});
      }
      break;
  }
}

QVariant AnalysisEntityModel::modelData(int role)
{
  return mData.value(role, QVariant());
}

void AnalysisEntityModel::load(const QJsonObject &json)
{
  if(json.isEmpty())
    return;
  QMetaEnum modelDataRoleEnum = QMetaEnum::fromType<AnalysisModelDataRoles>();
  QMetaEnum entityRootRolesEnum = QMetaEnum::fromType<AbstractEntityDataModel::EntityDataRoles>();
  QMetaEnum entityDataRoleEnum = QMetaEnum::fromType<AnalysisEntity::EntityDataRoles>();
  QMetaEnum entityTypes = QMetaEnum::fromType<EntityTypes>();
  QHash<QString, int> entityDataRolesMap;
  for(int i = 0; i < entityRootRolesEnum.keyCount(); i++)
    entityDataRolesMap[entityRootRolesEnum.key(i)] = entityRootRolesEnum.value(i);
  for(int i = 0; i < entityDataRoleEnum.keyCount(); i++)
    entityDataRolesMap[entityDataRoleEnum.key(i)] = entityDataRoleEnum.value(i);

  QHash<QString, int> entityTypeMap;
  for(int i = 0; i < entityTypes.keyCount(); i++)
    entityTypeMap[entityTypes.key(i)] = entityTypes.value(i);

  QHash<TEntityUid, AbstractEntityDataModel::DataPtr> newData;
  auto chromaData = json["chromatogrammData"].toObject();
  auto jchromaEntities = chromaData["entities"].toArray();
  auto jmodelData = json["modelData"].toObject();
  QHash<int, QVariant> modelData;
  for(auto& stringKey : jmodelData.keys())
  {
    int key = modelDataRoleEnum.keyToValue(stringKey.toStdString().c_str());
    if(key == -1)
      continue;
    AnalysisModelDataRoles role = (AnalysisModelDataRoles)key;
    switch(role)
    {
    case ModelRoleIsSpectral:
      modelData[role] = jmodelData[stringKey].toBool();
      break;
    case ModelRoleSpectralRenderType:
      modelData[role] = (RenderType)(jmodelData[stringKey].toInt());
      break;
    case ModelRoleCurrentXComponentNum:
    case ModelRoleCurrentYComponentNum:
    case ModelRoleCurrentSpectralComponentNum:
      modelData[role] = jmodelData[stringKey].toInt();
      break;
    case ModelRoleComponentsExplVarianceMap:
    {
      TComponentsExplVarianceMap map;
      QJsonArray cevArr = jmodelData[stringKey].toArray();
      for(int i = 0; i < cevArr.size(); i++)
      {
        QJsonObject cevElem = cevArr[i].toObject();
        auto key = cevElem["key"].toInt();
        auto value = cevElem["value"].toDouble();
        map[key] = value;
      }
      modelData[role] = QVariant::fromValue(map);
      break;
    }
    case ModelRoleColorFilter:
    case ModelRoleShapeFilter:
    {
      auto jfilter = jmodelData[stringKey].toObject();
      TPassportFilter filter{jfilter.toVariantMap()};
      modelData[role] = QVariant::fromValue(filter);
      break;
    }
    case ModelRoleLast:
    default:
      break;
    }
  }

  QString typeKey = entityRootRolesEnum.valueToKey(AbstractEntityDataModel::RoleEntityType);
  QString uidKey = entityRootRolesEnum.valueToKey(AbstractEntityDataModel::RoleEntityUid);
  for(int i = 0; i < jchromaEntities.size(); i++)
  {
    auto jentity = jchromaEntities[i].toObject();
    auto uid = QUuid::fromString(jentity["uid"].toString());

    AbstractEntityDataModel::DataPtr dataPtr = AbstractEntityDataModel::DataPtr::create();
    newData[uid] = dataPtr;
    auto& dataRef = *dataPtr;
    auto entityTypeStr = jentity[typeKey].toString().toStdString();

    if(entityTypeMap.contains(entityTypeStr.c_str()))
      dataRef[AbstractEntityDataModel::RoleEntityType] = entityTypeMap[entityTypeStr.c_str()];
    else
      dataRef[AbstractEntityDataModel::RoleEntityType] = TypeLast;

    dataRef[AbstractEntityDataModel::RoleEntityUid] = uid;
    for(auto& stringDataRole : jentity.keys())
    {
      auto enumKeyStr = stringDataRole.toStdString();
      int dataRole = entityDataRolesMap.value(enumKeyStr.c_str(), -1);
      if(dataRole == -1)
        continue;
      switch(dataRole)
      {
      case RoleShape:
      {
        auto path_bin = QByteArray::fromBase64(jentity[stringDataRole].toString().toLatin1());
        QDataStream stream(path_bin);
        QPainterPath path;
        stream >> path;
        dataRef[dataRole] = QVariant::fromValue(path);
        break;
      }
      case RoleShapePixelSize:
      case RoleIntId:
      case RoleOrderNum:
      {
        dataRef[dataRole] = jentity[stringDataRole].toInt();
        break;
      }
      case RoleForceShowName:
      case RoleForceShowPassport:
      case RoleGroupUserCreated:
      case RoleExcluded:
      {
        dataRef[dataRole] = jentity[stringDataRole].toBool();
        break;
      }
      case RoleTitle:
      {
        dataRef[dataRole] = jentity[stringDataRole].toString();
        break;
      }
      case RoleColor:
      {
        auto jobj = jentity[stringDataRole].toObject();
        auto red = jobj["red"].toInt();
        auto green = jobj["green"].toInt();
        auto blue = jobj["blue"].toInt();
        auto alpha = jobj["alpha"].toInt();
        dataRef[dataRole] = QColor(red, green, blue, alpha);
        break;
      }
      case RoleGroupsUidSet:
      {
        TGroupsUidSet set;
        auto jarr = jentity[stringDataRole].toArray();
        for(int i = 0; i < jarr.size(); i++)
          set << TEntityUid::fromString(jarr[i].toString());
        dataRef[dataRole] = QVariant::fromValue(set);
        break;
      }
      case RolePassport:
      {
        auto jpassport = jentity[stringDataRole].toObject();
        TPassport passport{jpassport.toVariantMap()};
        dataRef[dataRole] = QVariant::fromValue(passport);
        break;
      }
      case RolePassportFilter:
      {
        auto jfilter = jentity[stringDataRole].toObject();
        TPassportFilter filter{jfilter.toVariantMap()};
        dataRef[dataRole] = QVariant::fromValue(filter);
        break;
      }
      case RolePeakData:
      {
        auto jpeak = jentity[stringDataRole].toObject();
        TPeakData peakData;
        peakData.title = jpeak["title"].toString();
        peakData.orderNum = jpeak["order_num"].toInt();
        dataRef[dataRole] = QVariant::fromValue(peakData);
        break;
      }
      case RoleComponentsData:
      {
        auto jCompData = jentity[stringDataRole].toArray();
        TComponentsData data;
        for(int i = 0; i < jCompData.size(); i++)
        {
          auto jcomp = jCompData[i].toObject();
          auto key = jcomp["key"].toInt();
          auto value = jcomp["value"].toDouble();
          data[key] = value;
        }
        dataRef[dataRole] = QVariant::fromValue(data);
        break;
      }
      case AbstractEntityDataModel::RoleEntityLast:
      case RoleLast:
      default:
        break;
      }
    }

           //compatibility patch:
    if(dataRef[AbstractEntityDataModel::RoleEntityType] == TypeSample
        && !dataRef.contains(RoleTitle))
    {
      auto passport = dataRef.value(RolePassport).value<TPassport>();
      auto title = AnalysisEntityModel::passportToSampleTitle(passport);
      dataRef[RoleTitle] = title;
    }
    if(dataRef[AbstractEntityDataModel::RoleEntityType] == TypePeak
        && !dataRef.contains(RoleTitle))
    {
      auto peakData = dataRef.value(RolePeakData).value<TPeakData>();
      dataRef[RoleTitle] = peakData.title;
    }
    //end of compat patch
  }
  setData(newData);
  setModelData(modelData);
}

QJsonObject AnalysisEntityModel::save() const
{
  using namespace AnalysisEntity;
  QMetaEnum modelDataRoleEnum = QMetaEnum::fromType<AnalysisModelDataRoles>();
  QMetaEnum entityRootRolesEnum = QMetaEnum::fromType<AbstractEntityDataModel::EntityDataRoles>();
  QMetaEnum entityDataRoleEnum = QMetaEnum::fromType<AnalysisEntity::EntityDataRoles>();
  QMetaEnum entityTypes = QMetaEnum::fromType<EntityTypes>();
  QHash<int, QString> entityDataRolesMap;
  for(int i = 0; i < entityRootRolesEnum.keyCount(); i++)
    entityDataRolesMap[entityRootRolesEnum.value(i)] = entityRootRolesEnum.key(i);
  for(int i = 0; i < entityDataRoleEnum.keyCount(); i++)
    entityDataRolesMap[entityDataRoleEnum.value(i)] = entityDataRoleEnum.key(i);

  QHash<int, QString> entityTypeMap;
  for(int i = 0; i < entityTypes.keyCount(); i++)
    entityTypeMap[entityTypes.value(i)] = entityTypes.key(i);

  QJsonObject root;
  QJsonObject jchromaData;
  QJsonArray jchromaEntities;
  QJsonObject jmodelData;
  const auto &data = AbstractEntityDataModel::getData();
  for(auto entityIter : data.asKeyValueRange())
  {
    const auto& entityUid = entityIter.first;
    const auto& variantMap = entityIter.second;

    QJsonObject jentity;
    jentity["uid"] = entityUid.toString();
    QJsonObject jvariantMap;
    for(auto roleIter : variantMap->asKeyValueRange())
    {
      const auto& dataRole = roleIter.first;
      const auto& value = roleIter.second;
      QString stringKey = entityDataRolesMap.value(dataRole, "");
      if(!stringKey.isEmpty())
      {
        switch(dataRole)
        {
        case AbstractEntityDataModel::RoleEntityType:
          jentity[stringKey] = entityTypeMap.value(value.toInt(), entityTypeMap[RoleLast]);
          break;
        case AbstractEntityDataModel::RoleEntityUid://skip
          break;
        case RoleShape:
        {
          QByteArray path_bin;
          QDataStream stream(&path_bin, QIODeviceBase::WriteOnly);
          stream << value.value<TShape>();
          jentity[stringKey] = QString(path_bin.toBase64());
          break;
        }
        case RoleShapePixelSize:
        case RoleIntId:
        case RoleOrderNum:
          jentity[stringKey] = value.toInt();
          break;
        case RoleForceShowName:
        case RoleForceShowPassport:
        case RoleGroupUserCreated:
        case RoleExcluded:
          jentity[stringKey] = value.toBool();
          break;
        case RoleTitle:
          jentity[stringKey] = value.toString();
          break;
        case RoleColor:
        {
          auto color = value.value<TColor>();
          QJsonObject obj;
          obj["red"] = color.red();
          obj["green"] = color.green();
          obj["blue"] = color.blue();
          obj["alpha"] = color.alpha();
          jentity[stringKey] = obj;
          break;
        }
        case RoleGroupsUidSet:
        {
          auto set = value.value<TGroupsUidSet>();
          QJsonArray arr;
          for(auto& uid : set)
            arr << uid.toString();
          jentity[stringKey] = arr;
          break;
        }
        case RolePassport:
        {
          auto passport = value.value<TPassport>();
          QJsonObject jpassport = QJsonObject::fromVariantMap(passport);
          jentity[stringKey] = jpassport;
          break;
        }
        case RolePassportFilter:
        {
          auto filter = value.value<TPassportFilter>();
          QJsonObject jfilter = QJsonObject::fromVariantMap(filter);
          jentity[stringKey] = jfilter;
          break;
        }
        case RolePeakData:
        {
          auto peakData = value.value<TPeakData>();
          QJsonObject jpeak;
          jpeak["title"] = peakData.title;
          jpeak["order_num"] = peakData.orderNum;
          jentity[stringKey] = jpeak;
          break;
        }
        case RoleComponentsData:
        {
          auto compData = value.value<TComponentsData>();
          QJsonArray jcompArr;
          for(auto iter = compData.constKeyValueBegin(); iter != compData.constKeyValueEnd(); iter++)
          {
            QJsonObject jcomp;
            jcomp["key"] = iter->first;
            jcomp["value"] = iter->second;
            jcompArr << jcomp;
          }
          jentity[stringKey] = jcompArr;
          break;
        }
        case AbstractEntityDataModel::RoleEntityLast:
        case RoleLast:
        default:
          break;
        }
      }
    }
    jchromaEntities << jentity;
  }
  jchromaData["entities"] = jchromaEntities;
  root["chromatogrammData"] = jchromaData;
  auto modelData = mData;
  for(auto& key : modelData.keys())
  {
    auto dataRole = AnalysisModelDataRoles(key);
    auto stringKey = modelDataRoleEnum.valueToKey(key);
    switch(dataRole)
    {
    case ModelRoleIsSpectral:
      jmodelData[stringKey] = modelData[key].toBool();
      break;
    case ModelRoleSpectralRenderType:
      jmodelData[stringKey] = modelData[key].toInt();
      break;
    case ModelRoleCurrentXComponentNum:
    case ModelRoleCurrentYComponentNum:
    case ModelRoleCurrentSpectralComponentNum:
      jmodelData[stringKey] = modelData[key].toInt();
      break;
    case ModelRoleComponentsExplVarianceMap:
    {
      auto cevMap = modelData[dataRole].value<TComponentsExplVarianceMap>();
      QJsonArray cevArr;
      for(auto iter = cevMap.constKeyValueBegin(); iter != cevMap.constKeyValueEnd(); iter++)
      {
        QJsonObject cevElem;
        cevElem["key"] = iter->first;
        cevElem["value"] = iter->second;
        cevArr << cevElem;
      }
      jmodelData[stringKey] = cevArr;
      break;
    }
    case ModelRoleColorFilter:
    case ModelRoleShapeFilter:
    {
      auto filter = modelData[key].value<TPassportFilter>();
      QJsonObject jfilter = QJsonObject::fromVariantMap(filter);
      jmodelData[stringKey] = jfilter;
      break;
    }
    case ModelRoleLast:
    default:
      break;
    }
  }
  root["modelData"] = jmodelData;
  return root;
}

QString AnalysisEntityModel::genericGroupName(const TPassportFilter &colorFilter, const TPassportFilter &shapeFilter, const TPassport &passport)
{
  auto stringifyFilter = [](const TPassportFilter& filter, const TPassport& passport)
  {
    for(auto it = filter.constBegin(); it != filter.constEnd(); it++)
    {
      if(passport.contains(it.key()))
        return passport.value(it.key()).toString();
    }
    return QString();
  };
  if(colorFilter.isEmpty())
  {
    if(shapeFilter.isEmpty())
      return QString();
    else
      return stringifyFilter(shapeFilter, passport);
  }
  else
  {
    if(shapeFilter.isEmpty())
      return stringifyFilter(colorFilter, passport);
    else
      return QString("%1_%2").arg(stringifyFilter(colorFilter, passport)).arg(stringifyFilter(shapeFilter, passport));
  }
}

QString AnalysisEntityModel::genericGroupName(const TPassportFilter &colorFilter, const TPassportFilter &shapeFilter, const TPassportFilter &passportFilter)
{
  TPassport passport{passportFilter};
  return genericGroupName(colorFilter, shapeFilter, passport);
}

QString AnalysisEntityModel::passportToSampleTitle(const TPassport &passport)
{
  auto title = passport.value(PassportTags::filetitle).toString();
  if(title.isEmpty())
  {
    title = passport.value(PassportTags::filename).toString().section('/', -1);
    if(title.contains('\\'))
      title = title.section('\\', -1);
  }
  else
  {
    title = passport.value(PassportTags::filetitle).toString();
  }
  return title;
}
