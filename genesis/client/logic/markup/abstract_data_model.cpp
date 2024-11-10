#include "abstract_data_model.h"
#include <QJsonObject>
#include <QUuid>
#include <QSharedPointer>
// #include "genesis_markup_enums.h"

AbstractEntityDataModel::AbstractEntityDataModel(QObject *parent)
  : QObject(parent)
{

}

//this is for locking default functions from descendants virtual calls
using _C = AbstractEntityDataModel;
AbstractEntityDataModel::AbstractEntityDataModel(const AbstractEntityDataModel &other)
{
  //create full data (not pointers) copy of other model
  for(auto& key : other.mData.keys())
  {
    auto dataPtr = DataPtr::create(*other.mData[key]);
    mData[key] = dataPtr;
    mTypeToDataCache[dataPtr->getType()].insert(dataPtr);
  }
}

AbstractEntityDataModel::ConstDataPtr AbstractEntityDataModel::getEntity(TEntityUid entityId) const
{
  if(mData.contains(entityId))
    return mData[entityId];
  else
    return nullptr;
}

QList<AbstractEntityDataModel::ConstDataPtr> AbstractEntityDataModel::getEntities(EntityType type) const
{
  if(!mTypeToDataCache.contains(type))
    return {};
  return {mTypeToDataCache[type].begin(), mTypeToDataCache[type].end()};
}

QList<AbstractEntityDataModel::ConstDataPtr> AbstractEntityDataModel::getEntities(const QList<QPair<int, QVariant> > &filters) const
{
  //filter is pair of data role and it's value
  QList<ConstDataPtr> ret;
  for(auto& entity : mData)
  {
    bool pass = true;
    for(auto& filter : filters)
    {
      auto data = entity->getData(filter.first);
      if(!data.isValid() || data.isNull() || data != filter.second)
      {
        pass = false;
        break;
      }
    }
    if(pass)
      ret << entity;
  }
  return ret;
}

QList<AbstractEntityDataModel::ConstDataPtr> AbstractEntityDataModel::getEntities(std::function<bool (ConstDataPtr)> predicate) const
{
  QList<ConstDataPtr> ret;
  for(auto& entity : mData)
  {
    if(predicate(entity))
      ret << entity;
  }
  return ret;
}

QSet<TEntityUid> AbstractEntityDataModel::getEntitiesUidsSet(std::function<bool (ConstDataPtr)> predicate) const
{
  QSet<TEntityUid> ret;
  for(auto it = mData.constKeyValueBegin(); it != mData.constKeyValueEnd(); it++)
  {
    if(predicate(it->second))
      ret << it->first;
  }
  return ret;
}

QList<TEntityUid> AbstractEntityDataModel::getUIdListOfEntities(EntityType type) const
{
  if(!mTypeToDataCache.contains(type))
    return {};
  QList<TEntityUid> list;
  auto entList = mTypeToDataCache[type];
  for(auto ent : entList)
    list << ent->getUid();
  return list;
}

int AbstractEntityDataModel::countEntities(EntityType type) const
{
  return (mTypeToDataCache.value(type).size());
}

int AbstractEntityDataModel::countEntities(const QList<QPair<int, QVariant> > &filters) const
{
  int counter = 0;
  for(auto& entity : mData)
  {
    bool pass = true;
    for(auto& filter : filters)
    {
      auto data = entity->getData(filter.first);
      if(!data.isValid() || data.isNull() || data != filter.second)
      {
        pass = false;
        break;
      }
    }
    if(pass)
      counter++;
  }
  return counter;
}

int AbstractEntityDataModel::countEntities(std::function<bool (ConstDataPtr)> predicate) const
{
  int counter = 0;
  for(auto& entity : mData)
  {
    if(predicate(entity))
      counter++;
  }
  return counter;
}

bool AbstractEntityDataModel::setEntityData(TEntityUid Id, DataRoleType role, const QVariant &value)
{
  //запрещаем менять uid и EntityType существующих сущностей
  if(role == RoleEntityType &&
     role == RoleEntityUid)
    return false;
  if(mData.contains(Id))
  {
    //@TODO: очень хочется здесь сделать удаление из мапы при !value.isValid(), но страшно что-нибудь сломать
    (*mData[Id])[role] = value;
    EntityType type = mData[Id]->constFind(EntityDataRoles::RoleEntityType)->toInt();
    emit entityChanged(type, Id, role, (*mData[Id])[role]);

    return true;
  }
  return false;
}

bool AbstractEntityDataModel::resetEntityData(TEntityUid Id, const DataModel &newData)
{
  if(mData.contains(Id))
  {
    auto oldType = mData[Id]->getData(RoleEntityType);
    auto oldUid = mData[Id]->getData(RoleEntityUid);
    (*mData[Id]) = newData;
    (*mData[Id])[RoleEntityType] = oldType;
    (*mData[Id])[RoleEntityUid] = oldUid;
    EntityType type = mData[Id]->constFind(EntityDataRoles::RoleEntityType)->toInt();
    emit entityResetted(type, Id, mData[Id]);

    return true;
  }
  return false;
}

TEntityUid AbstractEntityDataModel::addNewEntity(EntityType type, const DataModel &data, TEntityUid uid)
{
  TEntityUid newId;
  if(uid.isNull())
    newId = TEntityUid::createUuid();
  else
    newId = uid;

  mData.insert(newId, DataPtr::create(data));
  mData[newId]->insert(EntityDataRoles::RoleEntityType, type);
  mTypeToDataCache[type] << mData[newId];
  mData[newId]->insert(EntityDataRoles::RoleEntityUid, newId);
  emit entityAdded(type, newId, mData[newId]);
  return newId;
}

TEntityUid AbstractEntityDataModel::addNewEntity(EntityType type, DataModel *data, TEntityUid uid)
{
  if(data)
    return _C::addNewEntity(type, *data, uid);
  else
    return _C::addNewEntity(type, DataModel(), uid);
}

bool AbstractEntityDataModel::removeEntity(TEntityUid Id)
{
  if(mData.contains(Id))
  {
    EntityType type = mData[Id]->constFind(EntityDataRoles::RoleEntityType)->toInt();
    emit entityAboutToRemove(type, Id);

    if(mTypeToDataCache.contains(type))
    {
      mTypeToDataCache[type].remove(mData[Id]);
      if(mTypeToDataCache[type].isEmpty())
        mTypeToDataCache.remove(type);
    }

    mData.remove(Id);
    return true;
  }
  return false;
}

const QHash<TEntityUid, AbstractEntityDataModel::DataPtr> &AbstractEntityDataModel::getData() const
{
  return mData;
}

void AbstractEntityDataModel::setData(const QHash<TEntityUid, DataPtr> &newData)
{
  emit modelAboutToReset();
  mData = newData;
  mTypeToDataCache.clear();

  for(auto& data : mData)
    mTypeToDataCache[data->getType()] << data;

  emit modelReset();
}

void AbstractEntityDataModel::clearData()
{
  emit modelAboutToReset();
  mData.clear();
  mTypeToDataCache.clear();
  emit modelReset();
}

void AbstractEntityDataModel::clearEntities(EntityType type)
{
  emit modelAboutToReset();
  mTypeToDataCache.remove(type);
  for(auto& uid : getUIdListOfEntities(type))
    _C::removeEntity(uid);
  emit modelReset();
}

void AbstractEntityDataModel::removeEntities(const QList<QPair<int, QVariant> > &filters)
{
  //filter is pair of data role and it's value
  QList<TEntityUid> toRemove;
  for(auto& entity : mData)
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
    _C::removeEntity(id);
  }
}

void AbstractEntityDataModel::removeEntitiesIf(std::function<bool (ConstDataPtr)> predicate)
{
  QList<TEntityUid> toRemove;
  for(auto& entity : mData)
  {
    if(predicate(entity))
      toRemove << entity->getData(RoleEntityUid).toUuid();
  }
  for(auto& id : toRemove)
  {
    _C::removeEntity(id);
  }
}

void AbstractEntityDataModel::copyEntitiesIf(AbstractEntityDataModel *from, AbstractEntityDataModel *to, std::function<bool (ConstDataPtr)> predicate)
{
  if(!from || !to)
    return;

  QList<QPair<TEntityUid, DataPtr>> toCopy;
  for(auto& entity : from->mData)
  {
    if(predicate(entity))
      toCopy.append({entity->getData(RoleEntityUid).toUuid(), entity});
  }
  for(auto& pair : toCopy)
  {
    auto dataPtr = DataPtr(new DataModel(*pair.second));
    to->mData[pair.first] = dataPtr;
    to->mTypeToDataCache[pair.second->getType()] << dataPtr;
  }
}

void AbstractEntityDataModel::beginResetModel()
{
  emit modelAboutToReset();
}

void AbstractEntityDataModel::endResetModel()
{
  emit modelReset();
}


EntityType DataModel::getType() const
{
  return getData(AbstractEntityDataModel::RoleEntityType).value<EntityType>();
}

TEntityUid DataModel::getUid() const
{
  return getData(AbstractEntityDataModel::RoleEntityUid).value<TEntityUid>();
}
