#include "markup_data_model.h"
#include "chromatogram_data_model.h"
#include "logic/markup/genesis_markup_enums.h"
#include "genesis_markup_enums.h"

using namespace GenesisMarkup;

QHash<ChromaId, CurveDataModelPtr> MarkupDataModel::mCurves = QHash<ChromaId, CurveDataModelPtr>();
MarkupDataModel::MarkupDataModel(MarkupStepController& controller, QObject *parent)
  : QObject(parent),
    mController(&controller)
//  , mCurrentMasterId(-1)
{

}

MarkupDataModel::MarkupDataModel(MarkupStepController& controller, const MarkupDataModel &other, QObject *newParent)
  : QObject(newParent),
    mController(&controller)
//  , mCurrentMasterId(other.mCurrentMasterId)
{
  mData = other.mData;
  mCurves = other.mCurves;
  for(auto& pair : other.getChromatogrammsList())
  {
    addChromatogramm(pair.first, *pair.second);
  }
}

void MarkupDataModel::clearCurves()
{
  mCurves.clear();
}

QJsonObject MarkupDataModel::print(const QList<int> &ignoreRoles, const QList<int> &ignoreTypes)
{
  QJsonObject root;
  QJsonObject jchromatogramms;
  for(auto& pair : getChromatogrammsList())
    jchromatogramms[QString::number(pair.first)] = pair.second->print(ignoreRoles, ignoreTypes);
  root["chromatogramms"] = jchromatogramms;

  QJsonObject jsettings;
  {

  }
  root["settings"] = jsettings;

  root["masterId"] = mData[MarkupMasterIdRole].toInt();
  return root;
}

void GenesisMarkup::MarkupDataModel::load(const QJsonObject &data)
{
  if(data.contains("chromatogramms"))
  {
    auto jchromatogramms = data["chromatogramms"].toObject();
    auto ids = jchromatogramms.keys();
    for(auto& id : ids)
    {
      auto jchromatogramm = jchromatogramms[id].toObject();
      auto chromatogramm = ChromatogrammModelPtr::create(id.toInt());
      chromatogramm->load(jchromatogramm);
      addChromatogramm(id.toInt(), chromatogramm);
    }
  }
  if(data.contains("settings"))
  {

  }
  if(data.contains("masterId"))
  {
    setData(MarkupMasterIdRole, data["masterId"].toInt());
  }
  if(data.contains("MarkupIdentificationData"))
  {
    setData(MarkupIdentificationData, QVariant::fromValue(data["MarkupIdentificationData"].toObject()));
  }
}

QJsonObject GenesisMarkup::MarkupDataModel::save() const
{
  QJsonObject root;
  QJsonObject jchromatogramms;
  for(auto& pair : getChromatogrammsList())
    jchromatogramms[QString::number(pair.first)] = pair.second->save();
  root["chromatogramms"] = jchromatogramms;

  QJsonObject jsettings;
  {

  }
  root["settings"] = jsettings;
  if(mData[MarkupIdentificationData].isValid())
    root["MarkupIdentificationData"] = mData[MarkupIdentificationData].value<QJsonObject>();

  root["masterId"] = mData[MarkupMasterIdRole].toInt();
  return root;
}

MarkupStepController *MarkupDataModel::controller() const
{
  return mController;
}

ChromatogrammModelPtr MarkupDataModel::getChromatogramm(ChromaId id)
{
  if(mChromatogramms.contains(id))
    return mChromatogramms[id];
  else
  {
    if(hasMaster() &&
       id == getMaster()->id() &&
       mData.contains(MarkupExternalMasterPtr) &&
       mData[MarkupExternalMasterPtr].isValid() &&
       !mData[MarkupExternalMasterPtr].isNull())
    {
      return mData[MarkupExternalMasterPtr].value<ChromatogrammModelPtr>();
    }
    return nullptr;
  }
}

ChromatogrammModelPtr MarkupDataModel::takeChromatogramm(ChromaId id)
{
  if(!mChromatogramms.contains(id))
    return nullptr;
  auto chroma = mChromatogramms[id];
  mChromatogramms.remove(id);
  return chroma;
}

CurveDataModelPtr MarkupDataModel::getCurve(ChromaId id)
{
  return mCurves[id];
}

bool MarkupDataModel::hasCurve(ChromaId id)
{
  return mCurves.contains(id);
}

bool MarkupDataModel::setCurve(ChromaId id, CurveDataModelPtr ptr)
{
  mCurves[id] = ptr;
  return true;
}

bool MarkupDataModel::addChromatogramm(ChromaId id, ChromatogrammModelPtr ptr)
{
  if(mChromatogramms.contains(id))
    return false;
  if(!ptr)
    ptr.reset(new ChromatogramDataModel(id));
  ptr->setMarkupModel(this);
  mChromatogramms[id] = ptr;
  connectChroma(ptr);
  emit chromatogrammAdded(id, mChromatogramms[id]);
  return true;
}

bool MarkupDataModel::addChromatogramm(ChromaId id, const GenesisMarkup::ChromatogramDataModel &data)
{
  return addChromatogramm(id, QSharedPointer<ChromatogramDataModel>::create(data));
}

bool MarkupDataModel::swapChromatogramm(ChromaId id, ChromatogrammModelPtr newChromaPtr)
{
  if(!newChromaPtr || !mChromatogramms.contains(id))
    return false;
  newChromaPtr->setMarkupModel(this);
  disconnectChroma(mChromatogramms[id]);
  mChromatogramms[id] = newChromaPtr;
  connectChroma(mChromatogramms[id]);
  emit chromatogrammSwapped(id, mChromatogramms[id]);
  return true;
}

bool MarkupDataModel::removeChromatogramm(ChromaId id)
{
  if(!mChromatogramms.contains(id))
    return false;
  disconnectChroma(mChromatogramms[id]);
  mChromatogramms.remove(id);
  emit chromatogrammRemoved(id);
  return true;
}

void MarkupDataModel::clearChromatogramms()
{
  for(auto& chroma : mChromatogramms)
    disconnectChroma(chroma);
  mChromatogramms.clear();
  emit chromatogrammsCleared();
}

int MarkupDataModel::getChromatogrammsCount() const
{
  return mChromatogramms.size();
}

QList<ChromaId> MarkupDataModel::getChromatogrammsIdList() const
{
  return mChromatogramms.keys();
}

QList<QPair<ChromaId, ChromatogrammModelPtr> > MarkupDataModel::getChromatogrammsList() const
{
  QList<QPair<ChromaId, ChromatogrammModelPtr> > ret;
  for(auto& key : mChromatogramms.keys())
    ret.append({key, mChromatogramms[key]});
  return ret;
}

QList<ChromatogrammModelPtr> MarkupDataModel::getChromatogrammsListModels() const
{
  QList<ChromatogrammModelPtr> models;
  for(auto& ids : mChromatogramms.keys()){
    models << mChromatogramms.value(ids);
  }
  return models;
}

bool MarkupDataModel::hasChromatogram(int Id) const
{
  return mChromatogramms.contains(Id);
}

void MarkupDataModel::setData(const QHash<uint, QVariant> &data)
{
  mData = data;
}

const QHash<uint, QVariant> &MarkupDataModel::data() const
{
  return mData;
}

void MarkupDataModel::setData(uint role, QVariant data)
{
  if(mData.contains(role) && mData[role] == data)
      return;
  mData[role] = data;
  emit dataChanged(role, mData[role]);
}

QVariant MarkupDataModel::getData(uint role) const
{
  if(!mData.contains(role))
    return QVariant();
  return mData[role];
}

void MarkupDataModel::resetData(QHash<uint, QVariant> newData)
{
  mData = newData;
  emit dataResetted(mData);
}

void MarkupDataModel::setMaster(ChromaId newId)
{
  if(!mChromatogramms.contains(newId))
    return;
  setData(MarkupMasterIdRole, newId);
//  mCurrentMasterId = newId;
//  emit dataChanged()
}

void MarkupDataModel::resetMaster()
{
  setData(MarkupMasterIdRole, -1);
//  mCurrentMasterId = -1;
}

ChromatogrammModelPtr MarkupDataModel::getMaster()
{
  auto masterId = mData[MarkupMasterIdRole].toInt();
  if(masterId == -1 ||
     !mChromatogramms.contains(masterId))
  {
    if(mData.contains(MarkupExternalMasterPtr) &&
       mData[MarkupExternalMasterPtr].isValid() &&
       !mData[MarkupExternalMasterPtr].isNull())
    {
      return mData[MarkupExternalMasterPtr].value<ChromatogrammModelPtr>();
    }
    return nullptr;
  }
  return mChromatogramms[masterId];
}

bool MarkupDataModel::hasMaster()
{
  bool hasMaster = mData.contains(MarkupMasterIdRole) &&
                   mData[MarkupMasterIdRole].isValid() &&
                   !mData[MarkupMasterIdRole].isNull() &&
                   mData[MarkupMasterIdRole].toInt() > 0 ;
  Q_ASSERT(hasMaster == !getMaster().isNull()); //have master but have not pointer to it
  return hasMaster;
}

bool MarkupDataModel::hasInternalMaster()
{
  return hasMaster() && mChromatogramms.contains(getMaster()->id());
}

void MarkupDataModel::updateValuablePeaksSortedIdList()
{
  if(!hasMaster())
    return;
  using Entity = AbstractEntityDataModel::ConstDataPtr;
  auto peaks = getMaster()->getEntities(TypePeak);
  QList<Entity> valuablePeaks;
  for(auto& peak : peaks)
  {
    auto peakType = peak->getData(PeakType);
    if(peakType == PTMarker || peakType == PTInterMarker)
      valuablePeaks << peak;
  }
  std::sort(valuablePeaks.begin(), valuablePeaks.end(), [](const Entity& a, const Entity& b)->bool
  {
    return a->getData(PeakRetentionTime).toDouble() < b->getData(PeakRetentionTime).toDouble();
  });
  QList<int> orderedIdList;
  for(auto& peak : valuablePeaks)
  {
    orderedIdList << peak->getData(PeakId).toInt();
  }
  setData(MarkupValuablePeaksSortedList, QVariant::fromValue(orderedIdList));
}

QList<int> MarkupDataModel::getValuablePeaksSortedIdList()
{
  auto list = getData(MarkupValuablePeaksSortedList);
  if(list.isValid() && !list.isNull())
     return list.value<QList<int>>();
  return QList<int>();
}

void MarkupDataModel::updateTitlesDisplayMode()
{
  for(auto& chroma : mChromatogramms)
    chroma->updateTitlesDisplayMode();
}

void MarkupDataModel::connectChroma(ChromatogrammModelPtr chroma)
{
  connect(chroma.get(), &ChromatogramDataModel::entityChanged, this, [this, chroma](
          EntityType type,
          TEntityUid eId,
          DataRoleType role,
          const QVariant& value)
  {
    emit chromatogramEntityChanged(chroma->id(), type, eId, role, value);
  });
  connect(chroma.get(), &ChromatogramDataModel::entityResetted, this, [this, chroma](
          EntityType type,
          TEntityUid eId,
          AbstractEntityDataModel::ConstDataPtr data)
  {
    emit chromatogramEntityResetted(chroma->id(), type, eId, data);
  });
  connect(chroma.get(), &ChromatogramDataModel::entityAdded, this, [this, chroma](
          EntityType type,
          TEntityUid eId,
          AbstractEntityDataModel::ConstDataPtr data)
  {
    emit chromatogramEntityAdded(chroma->id(), type, eId, data);
  });
  connect(chroma.get(), &ChromatogramDataModel::entityAboutToRemove, this, [this, chroma](
          EntityType type,
          TEntityUid eId)
  {
    emit chromatogramEntityAboutToRemove(chroma->id(), type, eId);
  });
  connect(chroma.get(), &ChromatogramDataModel::modelAboutToReset, this, [this, chroma]()
  {
    emit chromatogramModelAboutToReset(chroma->id());
  });
  connect(chroma.get(), &ChromatogramDataModel::modelReset, this, [this, chroma]()
  {
    emit chromatogramModelReset(chroma->id());
  });
}

void MarkupDataModel::disconnectChroma(ChromatogrammModelPtr chroma)
{
  disconnect(chroma.get(), nullptr, this, nullptr);
}
