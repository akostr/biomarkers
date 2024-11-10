#include "shape_color_grouped_entity_plot_data_model.h"
#include <QColor>

using namespace AnalysisEntity;

ShapeColorGroupedEntityPlotDataModel::ShapeColorGroupedEntityPlotDataModel(QObject *parent)
  : AbstractEntityDataModel(parent)
{

};

QColor ShapeColorGroupedEntityPlotDataModel::getActualColor(TEntityUid uid)
{
  return getGroupValue<QColor>(RoleColor, uid);//if entity not in group, it will be entity value itself
}

QPainterPath ShapeColorGroupedEntityPlotDataModel::getActualShape(TEntityUid uid)
{
  return getGroupValue<QPainterPath>(RoleShape, uid);//if entity not in group, it will be entity value itself
}

int ShapeColorGroupedEntityPlotDataModel::getActualShapeSize(TEntityUid uid)
{
  return getGroupValue<int>(RoleShapePixelSize, uid);//if entity not in group, it will be entity value itself
}

AnalysisEntity::ShapeDescriptor ShapeColorGroupedEntityPlotDataModel::getShapeDescriptor(TEntityUid uid)
{
  auto ownerUid = getCurrentValueGroup(uid);//if entity not in group, it will be entity value itself
  if(ownerUid.isNull())
    return {};
  auto ent = getEntity(ownerUid);
  return {ent->getData(RoleShape).value<TShape>(),
          ent->getData(RoleShapePixelSize).value<TShapePixelSize>()};
}

bool ShapeColorGroupedEntityPlotDataModel::setEntityGroup(TEntityUid entityUid, TEntityUid groupEntityUid)
{
  auto ent = getEntity(entityUid);
  auto groupEnt = getEntity(groupEntityUid);
  if(!ent || !groupEnt || !groupEnt->hasDataAndItsValid(RoleEntityType) ||
      (groupEnt->getType() != TypeShapeGroup
       && groupEnt->getType() != TypeColorGroup
       && groupEnt->getType() != TypeGroup
       && groupEnt->getType() != TypeHottelingGroup))
  {
    return false;
  }
  auto groupsUids = ent->getData(RoleGroupsUidSet).value<TGroupsUidSet>();
  groupsUids << groupEntityUid;

  auto res = AbstractEntityDataModel::setEntityData(entityUid, RoleGroupsUidSet, QVariant::fromValue(groupsUids));

  if(!res)
    return false;

  switch(groupEnt->getType())
  {
  case TypeShapeGroup:
    emit groupShapeChanged({entityUid});
    break;
  case TypeColorGroup:
    emit groupColorChanged({entityUid});
    break;
  case TypeGroup:
    emit groupColorChanged({entityUid});
    emit groupShapeChanged({entityUid});
    break;
  case TypeHottelingGroup:
    emit groupColorChanged({entityUid});
    emit groupShapeChanged({entityUid});
    break;
  default:
    break;
  }

  return true;
}

bool ShapeColorGroupedEntityPlotDataModel::addEntitiesToGroup(QSet<TEntityUid> entityUids, TEntityUid groupEntityUid)
{
  if(groupEntityUid.isNull() || entityUids.isEmpty())
    return false;

  auto groupEnt = getEntity(groupEntityUid);
  if(!groupEnt->hasDataAndItsValid(RoleEntityType) ||
      (groupEnt->getType() != TypeShapeGroup
       && groupEnt->getType() != TypeColorGroup
       && groupEnt->getType() != TypeGroup
       && groupEnt->getType() != TypeHottelingGroup))
  {
    return false;
  }

  for(auto& eId : entityUids)
  {
    auto ent = getEntity(eId);
    auto groupsSet = ent->getData(RoleGroupsUidSet).value<TGroupsUidSet>();
    groupsSet << groupEntityUid;
    setEntityData(eId, RoleGroupsUidSet, QVariant::fromValue(groupsSet));
  }
  switch(groupEnt->getType())
  {
  case TypeShapeGroup:
    emit groupShapeChanged(entityUids);
    break;
  case TypeColorGroup:
    emit groupColorChanged(entityUids);
    break;
  case TypeGroup:
    emit groupColorChanged(entityUids);
    emit groupShapeChanged(entityUids);
    break;
  case TypeHottelingGroup:
    emit groupColorChanged(entityUids);
    emit groupShapeChanged(entityUids);
    break;
  default:
    break;
  }
  return true;
}

bool ShapeColorGroupedEntityPlotDataModel::resetEntityGroup(TEntityUid entityUid, AnalysisEntity::EntityTypes groupEntityType)
{
  auto ent = getEntity(entityUid);
  if(!ent || !ent->hasDataAndItsValid(RoleGroupsUidSet))
    return false;
  auto uidsSet = ent->getData(RoleGroupsUidSet).value<TGroupsUidSet>();
  TGroupsUidSet uidsToRemoveSet;
  for(auto& groupUid : uidsSet)
  {
    auto groupEnt = getEntity(groupUid);
    if(!groupEnt->hasDataAndItsValid(RoleEntityType))
      continue;
    auto groupType = groupEnt->getType();
    if(groupType != groupEntityType)
      continue;
    else
      uidsToRemoveSet << groupUid;
  }
  if(uidsToRemoveSet.isEmpty())
    return false;
  uidsSet.subtract(uidsToRemoveSet);

  //действуем в предположении, что в сущности не может лежать неправильных uid-ов групп
  //таким образом мы сокращаем количество проверок и операций
  auto res = AbstractEntityDataModel::setEntityData(entityUid, RoleGroupsUidSet, QVariant::fromValue(uidsSet));
  if(!res)
    return false;
  switch(groupEntityType)
  {
  case TypeShapeGroup:
    emit groupShapeChanged({entityUid});
    break;
  case TypeColorGroup:
    emit groupColorChanged({entityUid});
    break;
  case TypeGroup:
    emit groupColorChanged({entityUid});
    emit groupShapeChanged({entityUid});
    break;
  case TypeHottelingGroup:
    emit groupColorChanged({entityUid});
    emit groupShapeChanged({entityUid});
    break;
  default:
    break;
  }
  return true;
}

bool ShapeColorGroupedEntityPlotDataModel::resetEntityGroup(TEntityUid entityUid, TEntityUid groupEntityUid)
{
  auto ent = getEntity(entityUid);
  auto groupEntType = getEntity(groupEntityUid)->getType();

  if(!ent || !ent->hasDataAndItsValid(RoleGroupsUidSet))
    return false;

  auto uidsSet = ent->getData(RoleGroupsUidSet).value<TGroupsUidSet>();

  if(!uidsSet.remove(groupEntityUid))
    return false;

  //действуем в предположении, что в сущности не может лежать неправильных uid-ов групп
  //таким образом мы сокращаем количество проверок и операций
  auto res = AbstractEntityDataModel::setEntityData(entityUid, RoleGroupsUidSet, QVariant::fromValue(uidsSet));
  if(!res)
    return false;

  switch(groupEntType)
  {
  case TypeShapeGroup:
    emit groupShapeChanged({entityUid});
    break;
  case TypeColorGroup:
    emit groupColorChanged({entityUid});
    break;
  case TypeGroup:
    emit groupColorChanged({entityUid});
    emit groupShapeChanged({entityUid});
    break;
  case TypeHottelingGroup:
    emit groupColorChanged({entityUid});
    emit groupShapeChanged({entityUid});
    break;
  default:
    break;
  }

  return true;
}

bool ShapeColorGroupedEntityPlotDataModel::setEntityData(TEntityUid Id, DataRoleType role, const QVariant &value)
{
  auto ent = getEntity(Id);
  if(!ent)
    return false;
  switch(role)
  {
  case RoleGroupsUidSet:
  {
    auto uids = value.value<TGroupsUidSet>();
    validateGroupsUidsSet(uids);//remove non-groups uids
    return AbstractEntityDataModel::setEntityData(Id, role, QVariant::fromValue(uids));
  }
  case RoleColor:
  {
    auto entType = ent->getType();
    if(entType == TypeColorGroup || entType == TypeGroup || entType == TypeHottelingGroup)
    {
      auto entUidsList = getEntitiesUidsSet([Id](ConstDataPtr data)->bool
                                            {
                                              return data->getData(RoleGroupsUidSet)
                                                  .value<TGroupsUidSet>().contains(Id);
                                            });

      auto result = AbstractEntityDataModel::setEntityData(Id, role, value);
      if(result)
        emit groupColorChanged(entUidsList);
      return result;
    }
    break;
  }
  case RoleShape:
  {
    auto entType = ent->getType();
    if(entType == TypeShapeGroup || entType == TypeGroup || entType == TypeHottelingGroup)
    {
      auto entUidsList = getEntitiesUidsSet([Id](ConstDataPtr data)->bool
                                            {
                                              return data->getData(RoleGroupsUidSet)
                                                  .value<TGroupsUidSet>().contains(Id);
                                            });

      auto result = AbstractEntityDataModel::setEntityData(Id, role, value);
      if(result)
        emit groupShapeChanged(entUidsList);
      return result;
    }
    break;
  }
  case RoleShapePixelSize:
  {
    auto entType = ent->getType();
    if(entType == TypeShapeGroup || entType == TypeGroup || entType == TypeHottelingGroup)
    {
      auto entUidsList = getEntitiesUidsSet([Id](ConstDataPtr data)->bool
                                            {
                                              return data->getData(RoleGroupsUidSet)
                                                  .value<TGroupsUidSet>().contains(Id);
                                            });

      auto result = AbstractEntityDataModel::setEntityData(Id, role, value);
      if(result)
        emit groupShapeChanged(entUidsList);
      return result;
    }
  }
  default:
    break;
  }
  return AbstractEntityDataModel::setEntityData(Id, role, value);
}

TEntityUid ShapeColorGroupedEntityPlotDataModel::addNewEntity(EntityType type, const DataModel &data, QUuid uid)
{
  //better is to not add entity with alredy created groups list to avoid this double-copying
  if(data.contains(RoleGroupsUidSet) && data[RoleGroupsUidSet].isValid())
  {
    DataModel copy = data;
    auto uidsSet = copy[RoleGroupsUidSet].value<TGroupsUidSet>();
    validateGroupsUidsSet(uidsSet);
    return AbstractEntityDataModel::addNewEntity(type, copy, uid);
  }
  return AbstractEntityDataModel::addNewEntity(type, data, uid);
}

TEntityUid ShapeColorGroupedEntityPlotDataModel::addNewEntity(EntityType type, DataModel *data, QUuid uid)
{
  //redefinition of default methods because there is used _C::
  //(base functions is guarded from virtual calls) but we need to use virtual funcs
  if(data)
    return addNewEntity(type, *data, uid);
  else
    return addNewEntity(type, DataModel(), uid);
}

bool ShapeColorGroupedEntityPlotDataModel::removeEntity(TEntityUid Id)
{
  auto depList = getEntities([Id](ConstDataPtr data)->bool
                             {
                               return data->hasDataAndItsValid(RoleGroupsUidSet) && data->getData(RoleGroupsUidSet).value<TGroupsUidSet>().contains(Id);
                             });
  for(auto& ent : depList)
  {
    auto uid = ent->getData(RoleEntityUid).value<TEntityUid>();
    resetEntityGroup(uid, Id);
  }
  return AbstractEntityDataModel::removeEntity(Id);
}

void ShapeColorGroupedEntityPlotDataModel::removeEntities(const QList<QPair<int, QVariant> > &filters)
{
  //redefinition of default methods because there is used _C::
  //(base functions is guarded from virtual calls) but we need to use virtual funcs

  //filter is pair of data role and it's value
  QList<TEntityUid> toRemove;
  for(auto& entity : getData())
  {
    bool pass = true;
    for(auto& filter : filters)
    {
      if(!entity->hasDataAndItsValid(filter.first) ||
          entity->getData(filter.first) != filter.second)
      {
        pass = false;
        break;
      }
    }
    if(pass)
      toRemove << entity->getData(RoleEntityUid).toUuid();
  }
  for(auto& id : toRemove)
  {
    removeEntity(id);
  }
}

void ShapeColorGroupedEntityPlotDataModel::removeEntitiesIf(std::function<bool (ConstDataPtr)> predicate)
{
  //redefinition of default methods because there is used _C::
  //(base functions is guarded from virtual calls) but we need to use virtual funcs
  QList<TEntityUid> toRemove;
  for(auto& entity : getData())
  {
    if(predicate(entity))
      toRemove << entity->getData(RoleEntityUid).toUuid();
  }
  for(auto& id : toRemove)
  {
    removeEntity(id);
  }
}

void ShapeColorGroupedEntityPlotDataModel::validateGroupsUidsSet(TGroupsUidSet &set)
{
  TGroupsUidSet uidsToRemove;
  for(auto& uid : set)
  {
    auto ent = getEntity(uid);
    if(!ent || !ent->hasDataAndItsValid(RoleEntityType))
    {
      uidsToRemove << uid;
      continue;
    }
    auto entType = getEntity(uid)->getType();
    if(entType != TypeColorGroup
        && entType != TypeShapeGroup
        && entType != TypeGroup
        && entType != TypeHottelingGroup)
      uidsToRemove << uid;
  }
  set.subtract(uidsToRemove);
}

TEntityUid ShapeColorGroupedEntityPlotDataModel::getCurrentValueGroup(TEntityUid uid)
{
  return getGroupValue<TEntityUid>(RoleEntityUid, uid);
}
