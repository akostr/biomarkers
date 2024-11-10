#include "chromatogram_passport_model.h"
#include "genesis_markup_enums.h"

namespace GenesisMarkup{

ChromatogramPassportModel::ChromatogramPassportModel()
  : AbstractEntityDataModel()
{

}

ChromatogramPassportModel::ChromatogramPassportModel(const ChromatogramPassportModel &other)
  :AbstractEntityDataModel(other)
{
  mIdToUidMap = other.mIdToUidMap;
}

AbstractEntityDataModel::ConstDataPtr ChromatogramPassportModel::getPassport(int chromatogrammId) const
{
  if(!mIdToUidMap.contains(chromatogrammId))
    return nullptr;
  return getEntity(mIdToUidMap[chromatogrammId]);
}

void ChromatogramPassportModel::load(const QJsonObject &data)
{
}

QJsonObject ChromatogramPassportModel::save() const
{
  return QJsonObject();
}

bool ChromatogramPassportModel::setPassportData(int chromaId, DataRoleType role, const QVariant &value)
{
  if(!mIdToUidMap.contains(chromaId))
    return false;
  const auto& uid = mIdToUidMap[chromaId];
  return setEntityData(uid, role, value);
}

bool ChromatogramPassportModel::resetPassportData(int chromaId, const DataModel &newData)
{
  if(!mIdToUidMap.contains(chromaId))
    return false;
  const auto& uid = mIdToUidMap[chromaId];
  return resetEntityData(uid, newData);
}

void ChromatogramPassportModel::addNewPassport(int chromaId)
{
  auto uid = addNewEntity(DataPassport, nullptr);
  if(uid.isNull())
    return;
  mIdToUidMap[chromaId] = uid;
}

bool ChromatogramPassportModel::removePassport(int chromaId)
{
  if(!mIdToUidMap.contains(chromaId))
    return false;
  const auto& uid = mIdToUidMap[chromaId];
  return removeEntity(uid);
}

QList<int> ChromatogramPassportModel::getIdList() const
{
  return mIdToUidMap.keys();
}


bool ChromatogramPassportModel::setEntityData(TEntityUid Id, DataRoleType role, const QVariant &value)
{
  return AbstractEntityDataModel::setEntityData(Id, role, value);
}

bool ChromatogramPassportModel::resetEntityData(TEntityUid Id, const DataModel &newData)
{
  return AbstractEntityDataModel::resetEntityData(Id, newData);
}

QUuid ChromatogramPassportModel::addNewEntity(EntityType type, const DataModel &data, QUuid uid)
{
  return AbstractEntityDataModel::addNewEntity(type, data, uid);
}

QUuid ChromatogramPassportModel::addNewEntity(EntityType type, DataModel *data, QUuid uid)
{
  return AbstractEntityDataModel::addNewEntity(type, data, uid);
}

bool ChromatogramPassportModel::removeEntity(TEntityUid Id)
{
  return AbstractEntityDataModel::removeEntity(Id);
}

AbstractEntityDataModel::ConstDataPtr ChromatogramPassportModel::getEntity(TEntityUid entityId) const
{
  return AbstractEntityDataModel::getEntity(entityId);
}

//const QHash<AbstractEntityDataModel::EntityUId, AbstractEntityDataModel::DataPtr> &ChromatogramPassportModel::getData() const
//{
//  return AbstractEntityDataModel::getData
//}

//void ChromatogramPassportModel::setData(const QHash<EntityUId, DataPtr> &newData)
//{
//  return AbstractEntityDataModel::setData
//}

//void ChromatogramPassportModel::clearData()
//{
//  return AbstractEntityDataModel::clearData();
//}

} //namespace GenesisMarkup
