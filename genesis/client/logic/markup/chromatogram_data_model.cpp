#include "chromatogram_data_model.h"
#include "genesis_markup_enums.h"
#include "qjsonobject.h"
#include <QVariant>
#include <QJsonArray>
#include <QPointF>
#include <QMetaEnum>
#include <QColor>
#include "markup_data_model.h"
//debug purpose
#include <logic/markup/curve_data_model.h>
#include <logic/markup/baseline_data_model.h>
#include <logic/context_root.h>
#include <logic/known_context_tag_names.h>
#include <QJsonDocument>

namespace GenesisMarkup{

QList<QColor> ChromatogramDataModel::DefaultInterMarkerColors = { {228, 105, 109, 200},
                                                                 {251, 189, 86, 200},
                                                                 {240, 219, 86, 200},
                                                                 {105, 192, 151, 200},
                                                                 { 94, 197, 255, 200},
                                                                 { 54, 160, 245, 200},
                                                                 {119, 98, 235, 200}};
QColor ChromatogramDataModel::DefaultMarkerColor = QColor(0, 32, 51, 150);
QColor ChromatogramDataModel::DefaultPeakColor = QColor(0, 65, 102, 50);
QString ChromatogramDataModel::DefaultMarkerPrefix = "M";

ChromatogramDataModel::ChromatogramDataModel(ChromaId id)
  : AbstractEntityDataModel(),
  mInterMarkerColors(DefaultInterMarkerColors),
  mMarkerColor(DefaultMarkerColor),
  mPeakColor(DefaultPeakColor),
  mMarkerPrefix(DefaultMarkerPrefix)
{
  mData[ChromatogrammIdRole]                         = id;
  mData[ChromatogrammSettingsBaseLineSmoothLvl]      = Defaults::kChromatogrammSettingsBaseLineSmoothLvlDefault;
  mData[ChromatogrammSettingsMinimalPeakHeight]      = Defaults::kChromatogrammSettingsMinimalPeakHeightDefault;
  mData[ChromatogrammSettingsNoisy]                  = Defaults::kChromatogrammSettingsNoisyDefault;
  mData[ChromatogrammSettingsMedianFilterWindowSize] = Defaults::kChromatogrammSettingsMedianFilterWindowSizeDefault;
  mData[ChromatogrammSettingsSignalToMedianRatio]    = Defaults::kChromatogrammSettingsSignalToMedianRatioDefault;
  mData[ChromatogrammSettingsDegreeOfApproximation]  = Defaults::kChromatogrammSettingsDegreeOfApproximationDefault;
  mData[ChromatogrammSettingsBoundsByMedian]         = Defaults::kChromatogrammSettingsBoundsByMedianDefault;
  mData[ChromatogrammSettingsCoelution]              = Defaults::kChromatogrammSettingsCoelutionDefault;
  mData[ChromatogrammSettingsViewSmooth]             = Defaults::kChromatogrammSettingsViewSmoothDefault;
  mData[ChromatogrammMode]                           = Defaults::kChromatogrammModeDefault;

  connect(this, &AbstractEntityDataModel::entityChanged, this, &ChromatogramDataModel::onEntityChanged);
  connect(this, &AbstractEntityDataModel::entityResetted, this, &ChromatogramDataModel::onEntityResetted);
  connect(this, &AbstractEntityDataModel::entityAdded, this, &ChromatogramDataModel::onEntityAdded);
  connect(this, &AbstractEntityDataModel::entityAboutToRemove, this, &ChromatogramDataModel::onEntityAboutToRemove);
  connect(this, &AbstractEntityDataModel::modelAboutToReset, this, &ChromatogramDataModel::onModelAboutToReset);
  connect(this, &AbstractEntityDataModel::modelReset, this, &ChromatogramDataModel::onModelReset);
}

ChromatogramDataModel::ChromatogramDataModel(const ChromatogramDataModel &other)
  : AbstractEntityDataModel(other),
  mPeaksOrderingMap(other.mPeaksOrderingMap),
  mInterMarkerColors(DefaultInterMarkerColors),
  mMarkerColor(DefaultMarkerColor),
  mPeakColor(DefaultPeakColor),
  mMarkerPrefix(DefaultMarkerPrefix)
{
  mData = other.mData;
  connect(this, &AbstractEntityDataModel::entityChanged, this, &ChromatogramDataModel::onEntityChanged);
  connect(this, &AbstractEntityDataModel::entityResetted, this, &ChromatogramDataModel::onEntityResetted);
  connect(this, &AbstractEntityDataModel::entityAdded, this, &ChromatogramDataModel::onEntityAdded);
  connect(this, &AbstractEntityDataModel::entityAboutToRemove, this, &ChromatogramDataModel::onEntityAboutToRemove);
  connect(this, &AbstractEntityDataModel::modelAboutToReset, this, &ChromatogramDataModel::onModelAboutToReset);
  connect(this, &AbstractEntityDataModel::modelReset, this, &ChromatogramDataModel::onModelReset);
}

void ChromatogramDataModel::setId(ChromaId id)
{
  if(mData.contains(ChromatogrammIdRole))
    if(mData[ChromatogrammIdRole] == id)
      return;
  mData[ChromatogrammIdRole] = id;
  emit chromatogramDataChanged(ChromatogrammIdRole, mData[ChromatogrammIdRole]);
}

ChromaId ChromatogramDataModel::id() const
{
  return mData[ChromatogrammIdRole].value<ChromaId>();
}

void ChromatogramDataModel::setCutPoint(double x)
{
  if(mData.contains(ChromatogrammCutPoint))
    if(mData[ChromatogrammCutPoint] == x)
      return;
  mData[ChromatogrammCutPoint] = x;
  emit chromatogramDataChanged(ChromatogrammCutPoint, mData[ChromatogrammCutPoint]);
}

void ChromatogramDataModel::resetCutPoint()
{
  if(mData.contains(ChromatogrammCutPoint))
    if(mData[ChromatogrammCutPoint] == 0)
      return;
  mData[ChromatogrammCutPoint] = 0;
  emit chromatogramDataChanged(ChromatogrammCutPoint, mData[ChromatogrammCutPoint]);
}

double ChromatogramDataModel::cutPoint()
{
  if(!mData.contains(ChromatogrammCutPoint))
    return 0;
  return mData[ChromatogrammCutPoint].toDouble();
}

bool ChromatogramDataModel::hasCutPoint()
{
  if(!mData.contains(ChromatogrammCutPoint) ||
     mData[ChromatogrammCutPoint].toDouble() == 0)
    return false;
  return true;
}

void ChromatogramDataModel::updateTitlesDisplayMode()
{
  emit modelAboutToReset();
  blockSignals(true);
  auto titleMode = Core::GenesisContextRoot::Get()->GetContextMarkup()->GetData(Names::MarkupContextTags::kPeakTitleDisplayMode).value<TChromatogrammTitlesDisplayMode>();
  for(auto& peak : getEntities(TypePeak))
    resolvePeakNaming(peak->getData(RoleEntityUid).toUuid(), titleMode);
  blockSignals(false);
  emit modelReset();
}

QPair<QUuid, QUuid> ChromatogramDataModel::getNeighbourPeaksUids(const QUuid &uid) const
{
  QPair<QUuid, QUuid> ret;
  auto entity = getEntity(uid);

  if(mPeaksOrderingMap.size() < 2 ||
     getEntity(uid)->getType() != TypePeak)
    return ret;

  auto peak = std::find_if(mPeaksOrderingMap.begin(), mPeaksOrderingMap.end(),
                           [uid](const QUuid& v)->bool{return v == uid;});
  if(peak == mPeaksOrderingMap.end())
  {
    return ret;
  }
  else if(peak.key() != entity->getData(PeakStart).toPointF().x())
  {
    Q_ASSERT(false); //mPeaksOrderingMap is invalid
    return ret;
  }

  if(peak == mPeaksOrderingMap.begin())
  {
    ret.second = *std::next(peak);
    return ret;
  }
  else if(std::next(peak) == mPeaksOrderingMap.end())
  {
    ret.first = *std::prev(peak);
    return ret;
  }
  else
  {
    ret.first = *std::prev(peak);
    ret.second = *std::next(peak);
    return ret;
  }
}

QPair<QUuid, QUuid> ChromatogramDataModel::getNeighbourIntervalsUids(const QUuid &uid) const
{
  QPair<QUuid, QUuid> ret;
  auto entity = getEntity(uid);

  if(mIntervalsOrderingMap.size() < 2 ||
     getEntity(uid)->getType() != IntervalType)
    return ret;

  auto peak = std::find_if(mIntervalsOrderingMap.begin(), mIntervalsOrderingMap.end(),
                           [uid](const QUuid& v)->bool{return v == uid;});
  if(peak == mIntervalsOrderingMap.end())
  {
    return ret;
  }
  else if(peak.key() != entity->getData(IntervalLeft).value<TIntervalLeft>())
  {
    Q_ASSERT(false); //mPeaksOrderingMap is invalid
    return ret;
  }

  if(peak == mIntervalsOrderingMap.begin())
  {
    ret.second = *std::next(peak);
    return ret;
  }
  else if(std::next(peak) == mIntervalsOrderingMap.end())
  {
    ret.first = *std::prev(peak);
    return ret;
  }
  else
  {
    ret.first = *std::prev(peak);
    ret.second = *std::next(peak);
    return ret;
  }
}

QUuid ChromatogramDataModel::getLeftPeakUid(const QUuid &uid) const
{
  auto entity = getEntity(uid);

  if(mPeaksOrderingMap.size() < 2 ||
     getEntity(uid)->getType() != TypePeak)
    return QUuid();

  auto peak = std::find_if(mPeaksOrderingMap.begin(), mPeaksOrderingMap.end(), [uid](const QUuid& v)->bool{return v == uid;});
  if(peak == mPeaksOrderingMap.end())
  {
    return QUuid();
  }
  else if(peak.key() != entity->getData(PeakStart).toPointF().x())
  {
    Q_ASSERT(false); //mPeaksOrderingMap is invalid
    return QUuid();
  }

  if(peak == mPeaksOrderingMap.begin())
    return QUuid();
  return *std::prev(peak);
}

QUuid ChromatogramDataModel::getRightPeakUid(const QUuid &uid) const
{
  auto entity = getEntity(uid);

  if(mPeaksOrderingMap.size() < 2 ||
     getEntity(uid)->getType() != TypePeak)
    return QUuid();

  auto peak = std::find_if(mPeaksOrderingMap.begin(), mPeaksOrderingMap.end(), [uid](const QUuid& v)->bool{return v == uid;});
  if(peak == mPeaksOrderingMap.end())
  {
    return QUuid();
  }
  else if(peak.key() != entity->getData(PeakStart).toPointF().x())
  {
    Q_ASSERT(false); //mPeaksOrderingMap is invalid
    return QUuid();
  }

  if(std::next(peak) == mPeaksOrderingMap.end())
    return QUuid();
  return *std::next(peak);
}

QUuid ChromatogramDataModel::getLeftIntervalUid(const QUuid &uid) const
{
  auto entity = getEntity(uid);

  if(mIntervalsOrderingMap.size() < 2 ||
     getEntity(uid)->getType() != TypeInterval)
    return QUuid();

  auto peak = std::find_if(mIntervalsOrderingMap.begin(), mIntervalsOrderingMap.end(), [uid](const QUuid& v)->bool{return v == uid;});
  if(peak == mIntervalsOrderingMap.end())
  {
    return QUuid();
  }
  else if(peak.key() != entity->getData(IntervalLeft).value<TIntervalLeft>())
  {
    Q_ASSERT(false); //mPeaksOrderingMap is invalid
    return QUuid();
  }

  if(peak == mIntervalsOrderingMap.begin())
    return QUuid();
  return *std::prev(peak);
}

QUuid ChromatogramDataModel::getRightIntervalUid(const QUuid &uid) const
{
  auto entity = getEntity(uid);

  if(mIntervalsOrderingMap.size() < 2 ||
     getEntity(uid)->getType() != TypeInterval)
    return QUuid();

  auto peak = std::find_if(mIntervalsOrderingMap.begin(), mIntervalsOrderingMap.end(), [uid](const QUuid& v)->bool{return v == uid;});
  if(peak == mIntervalsOrderingMap.end())
  {
    return QUuid();
  }
  else if(peak.key() != entity->getData(IntervalLeft).value<TIntervalLeft>())
  {
    Q_ASSERT(false); //mPeaksOrderingMap is invalid
    return QUuid();
  }

  if(std::next(peak) == mIntervalsOrderingMap.end())
    return QUuid();
  return *std::next(peak);
}

QUuid ChromatogramDataModel::getPeakUidFromX(double x)
{
  auto iter = mPeaksOrderingMap.lowerBound(x);
  if(iter == mPeaksOrderingMap.end())
  {
    iter = std::prev(iter);
    auto lastUid = iter.value();
    auto start = iter.key();
    auto end = getEntity(lastUid)->getData(PeakEnd).toPointF().x();
    if(start <= x && x <= end)
      return lastUid;
    return QUuid();
  }
  //iter now should point to peak, that has Start.x exactly on x or Start.x > x,
  //in wich case previous iterator should have Start.x < x.
  auto peakXStart = iter.key();
  if(peakXStart == x)
  {
    return iter.value();
  }
  else //x is expected to be less than PeakStart.x
  {
    Q_ASSERT(x < peakXStart);
    if(iter == mPeaksOrderingMap.begin())
      return QUuid();
    iter = std::prev(iter);
    peakXStart = iter.key();
    auto peakXEnd = getEntity(iter.value())->getData(PeakEnd).toPointF().x();
    if(peakXStart <= x && x <= peakXEnd)
      return iter.value();
    else
      return QUuid(); //miss to free space
  }
}

QList<QUuid> ChromatogramDataModel::getOrderedPeaks()
{
  return mPeaksOrderingMap.values();
}

ChromaSettings ChromatogramDataModel::getChromaSettings() const
{
  ChromaSettings ret;
  ret.BLineSmoothLvl  = mData[ChromatogrammSettingsBaseLineSmoothLvl].toInt();
  ret.View_smooth     = mData[ChromatogrammSettingsViewSmooth].toInt();
  ret.Coel            = mData[ChromatogrammSettingsCoelution].toInt();
  ret.Doug_peuck      = mData[ChromatogrammSettingsDegreeOfApproximation].toInt();
  ret.Med_bounds      = mData[ChromatogrammSettingsBoundsByMedian].toInt();
  ret.Min_h           = mData[ChromatogrammSettingsMinimalPeakHeight].toDouble();
  ret.Noisy           = mData[ChromatogrammSettingsNoisy].toInt();
  ret.Sign_to_med     = mData[ChromatogrammSettingsSignalToMedianRatio].toDouble();
  ret.Window_size     = mData[ChromatogrammSettingsMedianFilterWindowSize].toInt();
  return ret;
}

void ChromatogramDataModel::setChromaSettings(const ChromaSettings &settings)
{
  setChromatogramValue(ChromatogrammSettingsBaseLineSmoothLvl, settings.BLineSmoothLvl);
  setChromatogramValue(ChromatogrammSettingsViewSmooth, settings.View_smooth);
  setChromatogramValue(ChromatogrammSettingsCoelution, settings.Coel);
  setChromatogramValue(ChromatogrammSettingsDegreeOfApproximation, settings.Doug_peuck);
  setChromatogramValue(ChromatogrammSettingsBoundsByMedian, settings.Med_bounds);
  setChromatogramValue(ChromatogrammSettingsMinimalPeakHeight, settings.Min_h);
  setChromatogramValue(ChromatogrammSettingsNoisy, settings.Noisy);
  setChromatogramValue(ChromatogrammSettingsSignalToMedianRatio, settings.Sign_to_med);
  setChromatogramValue(ChromatogrammSettingsMedianFilterWindowSize, settings.Window_size);
}

BaseLineDataModelPtr ChromatogramDataModel::getBaseLine()
{
  auto blines = getEntities(TypeBaseline);
  if(blines.isEmpty())
    return nullptr;
  return blines.first()->getData(BaseLineData).value<TBaseLineDataModel>();
}

void ChromatogramDataModel::load(const QJsonObject &data)
{
  QMetaEnum entityEnum = QMetaEnum::fromType<MarkupEntityTypes>();
  QMetaEnum dataRoleEnum = QMetaEnum::fromType<ChromatogrammEntityDataRoles>();
  QMetaEnum entityDataRoleEnum = QMetaEnum::fromType<EntityDataRoles>();
  QMetaEnum privateRolesEnum = QMetaEnum::fromType<ChromatogrammDataRoles>();
  QHash<TEntityUid, DataPtr> newData;
  auto chromaData = data["chromatogrammData"].toObject();
  auto jchromaEntities = chromaData["entities"].toArray();
  auto modelData = data["modelData"].toObject();
  //  clearData(); //don't need to be here, because we have "setData()" at the end, wich will overwrite all of old data
  mData.clear();
  for(auto& stringKey : modelData.keys())
  {
    int key = privateRolesEnum.keyToValue(stringKey.toStdString().c_str());
    if(key == -1)
      continue;
    ChromatogrammDataRoles role = (ChromatogrammDataRoles)key;
    mData[role] = modelData[stringKey].toVariant();
  }
  QString typeKey = entityDataRoleEnum.valueToKey(RoleEntityType);
  for(int i = 0; i < jchromaEntities.size(); i++)
  {
    auto jentity = jchromaEntities[i].toObject();
    auto uid = QUuid::fromString(jentity["uid"].toString());
    TEntityType type(-1);
    //backward compatibility
    if(jentity.contains("entityTypeData"))
      type = entityEnum.keyToValue(jentity["entityTypeData"].toString().toStdString().c_str());
    else if(jentity.contains(typeKey))
      type = entityEnum.keyToValue(jentity[typeKey].toString().toStdString().c_str());

    DataPtr dataPtr = DataPtr::create();
    newData[uid] = dataPtr;
    auto& dataRef = *dataPtr;
    dataRef[RoleEntityType] = type;
    dataRef[RoleEntityUid] = uid;
    for(auto& stringDataRole : jentity.keys())
    {
      auto dataRoleValue = dataRoleEnum.keyToValue(stringDataRole.toStdString().c_str());
      if(dataRoleValue == -1)
        continue;
      ChromatogrammEntityDataRoles dataRole = (ChromatogrammEntityDataRoles)dataRoleValue;
      switch(dataRole)
      {
      case PeakStart:
      case PeakEnd:
      case PeakTop:
      case PeakBottom:
      {
        QJsonObject jpt = jentity[stringDataRole].toObject();
        QPointF pt;
        pt.setX(jpt["x"].toDouble());
        pt.setY(jpt["y"].toDouble());
        dataRef[dataRole] = pt;
      }
        break;
      case IntervalLeft:
      case IntervalRight:
      {
        QJsonObject jpt = jentity[stringDataRole].toObject();
        QPointF pt;
        pt.setX(jpt["x"].toDouble());
        pt.setY(jpt["y"].toDouble());
        dataRef[dataRole] = pt.x();
        break;
      }
      case PeakRetentionTime:
      case PeakCovatsIndex:
      case PeakMarkerWindow:
      case PeakArea:
      case PeakHeight:
        dataRef[dataRole] = jentity[stringDataRole].toDouble();
        break;
      case PeakFormFactor:
      case PeakTitle:
      case PeakCompoundTitle:
        dataRef[dataRole] = jentity[stringDataRole].toString();
        break;
        //      case PassportDate:
        //        dataRef[dataRole] = QDateTime::fromString(jentity[stringDataRole].toString());
        //        break;
      case IntervalSettings:
        break;
      case IntervalType:
      {
        auto e = QMetaEnum::fromType<IntervalTypes>();
        dataRef[dataRole] = e.keyToValue(jentity[stringDataRole].toString().toStdString().c_str());
      }
        break;
      case PeakType:
      {
        auto e = QMetaEnum::fromType<PeakTypes>();
        dataRef[dataRole] = e.keyToValue(jentity[stringDataRole].toString().toStdString().c_str());
      }
        break;
      case PeakColor:
        dataRef[dataRole] = QColor(jentity[stringDataRole].toString());
        break;
      case PeakId:
      case PeakLibraryGroupId:
        //      case PassportFileId:
        dataRef[dataRole] = jentity[stringDataRole].toInt();
        break;
      case BaseLineData:
      {
        TBaseLineDataModel baseLineModel(new BaseLineDataModel());
        QJsonObject jlines = jentity[stringDataRole].toObject();
        for(auto& key : jlines.keys())
        {
          uint smoothFactor = key.toInt();
          QVector<double> keys, values;
          QJsonArray jpoints = jlines[key].toArray();
          for(int i = 0; i < jpoints.size(); i++)
          {
            QJsonObject pt = jpoints[i].toObject();
            keys << pt["key"].toDouble();
            values << pt["value"].toDouble();
          }
          baseLineModel->addBaseLine(smoothFactor, keys, values);
        }
        baseLineModel->setBaseLine(jlines["current_smooth_lvl"].toInt());
        dataRef[dataRole] = QVariant::fromValue<TBaseLineDataModel>(baseLineModel);
      }
      case CurveData://curve data loads not from here. it loads separate by special API
      default:
        break;
      }
      if(dataRef.getData(PeakType).toInt() == PTMarker)
        validatePeakRequiredValues(dataRef);
    }
//    if(!dataRef.hasDataAndItsValid(PeakType))
//      dataRef[PeakType] = PTNone;
//    else
//      resolvePeakType(uid, dataRef[PeakType].toInt());
  }
//  resolvePeaksOrderingAndColoring({PTMarker, PTInterMarker});
  AbstractEntityDataModel::setData(newData);
}

QJsonObject ChromatogramDataModel::save() const
{
  QMetaEnum entityEnum = QMetaEnum::fromType<MarkupEntityTypes>();
  QMetaEnum dataRoleEnum = QMetaEnum::fromType<ChromatogrammEntityDataRoles>();
  QMetaEnum entityDataRoleEnum = QMetaEnum::fromType<EntityDataRoles>();
  QMetaEnum privateRolesEnum = QMetaEnum::fromType<ChromatogrammDataRoles>();
  QJsonObject root;
  QJsonObject jchromaData;
  QJsonArray jchromaEntities;
  QJsonObject jmodelData;
  auto data = getData();
  for(auto& entityUid : data.keys())
  {
    auto variantMap = data[entityUid];
    if(variantMap->constFind(RoleEntityType) == variantMap->constEnd() ||
       variantMap->constFind(RoleEntityType)->toInt() == TypeCurve)
      continue;
    QJsonObject jentity;
    jentity["uid"] = entityUid.toString();
    QJsonObject jvariantMap;
    for(auto& dataRole : variantMap->keys())
    {
      const auto& map = (*variantMap);
      QString stringKey;
      if(dataRole == RoleEntityType)
      {
        //backward compatibility
        jentity["entityTypeData"] = entityEnum.valueToKey(map[dataRole].toInt());
      }
      if(QString stringKey = entityDataRoleEnum.valueToKey(dataRole); !stringKey.isEmpty())
      {//here we handle entity base data
        switch((EntityDataRoles)dataRole)
        {
        case EntityDataRoles::RoleEntityType:
        {
          jentity[stringKey] = entityEnum.valueToKey(map[dataRole].toInt());
        }
          break;
        case EntityDataRoles::RoleEntityUid://skip
        default:
          break;
        }
      }
      else if(QString stringKey = dataRoleEnum.valueToKey(dataRole); !stringKey.isEmpty())
      {
        switch((ChromatogrammEntityDataRoles)dataRole)
        {
        case BaseLineData:
        {
          QJsonObject jlines;
          auto baseLineModel = map[dataRole].value<TBaseLineDataModel>();
          if(baseLineModel)
          {
            const auto& map = baseLineModel->data();
//            for(auto iter = map.constBegin(); iter != map.constEnd(); iter++)
//            {
//              const auto& line = *iter;
//              QJsonArray jpoints;
//              for(int i = 0; i < line.first.size(); i++)
//              {
//                QJsonObject jpt;
//                jpt["key"] = line.first[i];
//                jpt["value"] = line.second[i];
//                jpoints << jpt;
//              }
//              jlines[QString::number(iter.key())] = jpoints;
//            }
            int smooth_lvl = baseLineModel->getSmoothFactor();

            for(auto iter = map.constBegin(); iter != map.constEnd(); iter++)
            {
                // Add only current baseline
                if(iter.key() == smooth_lvl)
                {
                    const auto& line = *iter;
                    QJsonArray jpoints;
                    for(int i = 0; i < line.first.size(); i++)
                    {
                        QJsonObject jpt;

                        jpt["key"] = line.first[i];
                        jpt["value"] = line.second[i];
                        jpoints << jpt;

                    }
                    jlines[QString::number(iter.key())] = jpoints;
                }
            }
            jlines["current_smooth_lvl"] = smooth_lvl;
            jentity[stringKey] = jlines;
          }
          break;
        }

        case PeakStart:
        case PeakEnd:
        case PeakTop:
        case PeakBottom:
        {
          QPointF pt = map[dataRole].toPointF();
          QJsonObject jpt;
          jpt["x"] = pt.x();
          jpt["y"] = pt.y();
          jentity[stringKey] = jpt;
        }
          break;
        case IntervalLeft:
        case IntervalRight:
        {
          double x = map[dataRole].toDouble();
          QJsonObject jpt;
          jpt["x"] = x;
          jpt["y"] = 0;
          jentity[stringKey] = jpt;
          break;
        }
        case PeakRetentionTime:
        case PeakCovatsIndex:
        case PeakMarkerWindow:
        case PeakArea:
        case PeakHeight:
        case PeakFormFactor:
          jentity[stringKey] = map[dataRole].toDouble();
          break;
        case PeakTitle:
        case PeakCompoundTitle:
          //        case PassportTitle:
          //        case PassportFilename:
          //        case PassportWell:
          //        case PassportField:
          //        case PassportLayer:
          jentity[stringKey] = map[dataRole].toString();
          break;
          //        case PassportDate:
          //          jentity[stringKey] = map[dataRole].toDateTime().toString();
          //          break;
        case IntervalSettings:
          //TODO: do something gere
          break;
        case IntervalType:
          {
            auto e = QMetaEnum::fromType<IntervalTypes>();
            jentity[stringKey] = e.valueToKey(map[dataRole].toInt());
          }
          break;
        case PeakType:
        {
          auto e = QMetaEnum::fromType<PeakTypes>();
          jentity[stringKey] = e.valueToKey(map[dataRole].toInt());
        }
          break;
        case PeakColor:
          jentity[stringKey] = map[dataRole].value<QColor>().name(QColor::HexArgb);
          break;
        case PeakId:
        case PeakLibraryGroupId:
          //        case PassportFileId:
          if(map[dataRole].isValid())
            jentity[stringKey] = map[dataRole].toInt();

          break;
        case CurveData://curve data not saves
        default:
          break;
        }
      }
    }
    jchromaEntities << jentity;
  }
  jchromaData["entities"] = jchromaEntities;
  root["chromatogrammData"] = jchromaData;

  for(auto& key : mData.keys())
  {
    auto stringKey = privateRolesEnum.valueToKey(key);
    jmodelData[stringKey] = QJsonValue::fromVariant(mData[key]);
  }
  root["modelData"] = jmodelData;
  return root;
}

QJsonArray ChromatogramDataModel::getJPeaksForStepCalculations() const
{
  QJsonArray ret;
  auto getData = [](ConstDataPtr peak, ChromatogrammEntityDataRoles role)->QVariant
  {
    auto constIter = peak->constFind(role);
    if(constIter == peak->constEnd())
      return QVariant();
    else
      return constIter.value();
  };
  for(auto& peak : getEntities(MarkupEntityTypes::TypePeak))
  {
    QJsonObject jpeak;
    jpeak["x_start"       ] = getData(peak, ChromatogrammEntityDataRoles::PeakStart).toPointF().x();
    jpeak["x_end"         ] = getData(peak, ChromatogrammEntityDataRoles::PeakEnd).toPointF().x();
    jpeak["ret_time"			] = getData(peak, ChromatogrammEntityDataRoles::PeakRetentionTime).toDouble();
    jpeak["form_factor"		] = getData(peak, ChromatogrammEntityDataRoles::PeakFormFactor).toDouble();
    jpeak["is_fake"				] = getData(peak, ChromatogrammEntityDataRoles::PeakType).toInt() == PTFake ? true : false;
    jpeak["title"					] = getData(peak, ChromatogrammEntityDataRoles::PeakTitle).toString();
    jpeak["is_marker"			] = getData(peak, ChromatogrammEntityDataRoles::PeakType).toInt() == PTMarker ? true : false;
    jpeak["y_start"				] = getData(peak, ChromatogrammEntityDataRoles::PeakStart).toPointF().y();
    jpeak["y_end"					] = getData(peak, ChromatogrammEntityDataRoles::PeakEnd).toPointF().y();
    jpeak["x_top"					] = getData(peak, ChromatogrammEntityDataRoles::PeakTop).toPointF().x();
    jpeak["y_top"					] = getData(peak, ChromatogrammEntityDataRoles::PeakTop).toPointF().y();
    jpeak["red"						] = getData(peak, ChromatogrammEntityDataRoles::PeakColor).value<QColor>().red();
    jpeak["green"					] = getData(peak, ChromatogrammEntityDataRoles::PeakColor).value<QColor>().green();
    jpeak["blue"					] = getData(peak, ChromatogrammEntityDataRoles::PeakColor).value<QColor>().blue();
    jpeak["alpha"					] = getData(peak, ChromatogrammEntityDataRoles::PeakColor).value<QColor>().alpha();
    jpeak["index"					] = getData(peak, ChromatogrammEntityDataRoles::PeakCovatsIndex).toDouble();
    jpeak["id"						] = getData(peak, ChromatogrammEntityDataRoles::PeakId).toInt();
    jpeak["is_intermarker"] = getData(peak, ChromatogrammEntityDataRoles::PeakType).toInt() == PTInterMarker ? true : false;
    jpeak["marker_window"	] = getData(peak, ChromatogrammEntityDataRoles::PeakMarkerWindow).toDouble();
    jpeak["area"					] = getData(peak, ChromatogrammEntityDataRoles::PeakArea).toDouble();
    jpeak["height"				] = getData(peak, ChromatogrammEntityDataRoles::PeakHeight).toDouble();
    if(peak->hasDataAndItsValid(ChromatogrammEntityDataRoles::PeakCompoundTitle))
      jpeak["PeakCompoundTitle"] = peak->getData(ChromatogrammEntityDataRoles::PeakCompoundTitle).toString();
    if(peak->hasDataAndItsValid(ChromatogrammEntityDataRoles::PeakLibraryGroupId))
      jpeak["PeakLibraryGroupId"] = peak->getData(ChromatogrammEntityDataRoles::PeakLibraryGroupId).toInt();
    ret << jpeak;
  }
  return ret;
}

QJsonArray ChromatogramDataModel::getJBaseLineForStepCalculations() const
{
  QJsonArray ret;
  const auto& blines = getEntities(MarkupEntityTypes::TypeBaseline);
  if(blines.empty())
    return ret;
  const auto bline = blines.first();
  Q_ASSERT(bline->constFind(BaseLineData) != bline->constEnd());
  const auto& baseLine = bline->constFind(BaseLineData)->value<TBaseLineDataModel>()->getBaseLine();

  for(int i = 0; i < baseLine.first.size(); i++)
  {
    QJsonObject jpt;
    jpt["x"] = baseLine.first[i];
    jpt["y"] = baseLine.second[i];
    ret << jpt;
  }
  return ret;
}

void ChromatogramDataModel::parseJPeaks(const QJsonArray &jpeaks)
{
//  qDebug().noquote() << QJsonDocument(jpeaks).toJson();
  emit modelAboutToReset();
  blockSignals(true);
  {
    clearEntities(TypePeak);
    for(int i = 0; i < jpeaks.size(); i++)
    {
      auto uid = AbstractEntityDataModel::addNewEntity(TypePeak);
      auto jpeak = jpeaks[i].toObject();
      AbstractEntityDataModel::setEntityData(uid, PeakStart, QPointF(jpeak["x_start"].toDouble(), jpeak["y_start"].toDouble()));
      AbstractEntityDataModel::setEntityData(uid, PeakEnd,   QPointF(jpeak["x_end"].toDouble(), jpeak["y_end"].toDouble()));
      AbstractEntityDataModel::setEntityData(uid, PeakTop,   QPointF(jpeak["x_top"].toDouble(), jpeak["y_top"].toDouble()));
      AbstractEntityDataModel::setEntityData(uid, PeakRetentionTime,
                                             (jpeak["ret_time"].toDouble() <= 0
                                             ? (jpeak["x_end"].toDouble() - jpeak["x_start"].toDouble())/2
        : jpeak["ret_time"].toDouble()));
      AbstractEntityDataModel::setEntityData(uid, PeakFormFactor, jpeak["form_factor"].toDouble());
      AbstractEntityDataModel::setEntityData(uid, PeakCovatsIndex, jpeak["index"].toDouble());
      AbstractEntityDataModel::setEntityData(uid, PeakMarkerWindow, jpeak["marker_window"].toDouble());
      AbstractEntityDataModel::setEntityData(uid, PeakArea, jpeak["area"].toDouble());
      AbstractEntityDataModel::setEntityData(uid, PeakHeight, jpeak["height"].toDouble());

      if(jpeak["is_fake"].toBool())
        AbstractEntityDataModel::setEntityData(uid, PeakType, PTFake);
      else if(jpeak["is_marker"].toBool())
        AbstractEntityDataModel::setEntityData(uid, PeakType, PTMarker);
      else if(jpeak["is_intermarker"].toBool())
        AbstractEntityDataModel::setEntityData(uid, PeakType, PTInterMarker);
      else
        AbstractEntityDataModel::setEntityData(uid, PeakType, PTPeak);

      AbstractEntityDataModel::setEntityData(uid, PeakTitle, jpeak["title"].toString());
      if(jpeak.contains("PeakLibraryGroupId"))
        AbstractEntityDataModel::setEntityData(uid, PeakLibraryGroupId, jpeak["PeakLibraryGroupId"].toInt());
      if(jpeak.contains("PeakCompoundTitle"))
        AbstractEntityDataModel::setEntityData(uid, PeakCompoundTitle, jpeak["PeakCompoundTitle"].toString());

      AbstractEntityDataModel::setEntityData(uid, PeakColor, QColor(jpeak["red"].toInt(),
                                             jpeak["green"].toInt(),
          jpeak["blue"].toInt(),
          jpeak["alpha"].toInt()));
      AbstractEntityDataModel::setEntityData(uid, PeakId, jpeak["id"].toInt());
      validatePeakRequiredValues(uid);
    }
    resolveAllPeaksOrderingNamingAndColoring({PTPeak, PTMarker, PTInterMarker});
    //    if(isMaster())
    //    {
    //      resolveInterMarkersIndexing();
    //    }
    //    resolvePeaksNaming();
  }

  blockSignals(false);
  emit modelReset();
}

void ChromatogramDataModel::parseJBaseLine(const QJsonArray &jbline)
{
  auto blinesUids = getUIdListOfEntities(TypeBaseline);
  TBaseLineDataModel baseLineModel;
  if(blinesUids.isEmpty())
  {
    baseLineModel.reset(new BaseLineDataModel());
    auto uid = AbstractEntityDataModel::addNewEntity(TypeBaseline);
    AbstractEntityDataModel::setEntityData(uid, BaseLineData, QVariant::fromValue(baseLineModel));
  }
  else
  {
    auto bline = getEntity(blinesUids.first());
    if(bline->constFind(BaseLineData) == bline->end())
    {
      baseLineModel.reset(new BaseLineDataModel());
      AbstractEntityDataModel::setEntityData(blinesUids.first(), BaseLineData, QVariant::fromValue(baseLineModel));
    }
    else
      baseLineModel = bline->constFind(BaseLineData)->value<TBaseLineDataModel>();
  }
  QVector<double> keys;
  QVector<double> values;
  for(int i = 0; i < jbline.size(); i++)
  {
    auto jpt = jbline[i].toObject();
    keys << jpt["x"].toDouble();
    values << jpt["y"].toDouble();
  }
  baseLineModel->setBaseLine(baseLineModel->getSmoothFactor(), keys, values);
}

void ChromatogramDataModel::parseJBaseLines(const QJsonObject &jblines, int defaultBaseline)
{
  auto blinesUids = getUIdListOfEntities(TypeBaseline);
  TBaseLineDataModel baseLineModel;
  if(blinesUids.isEmpty())
  {
    baseLineModel.reset(new BaseLineDataModel());
    auto uid = AbstractEntityDataModel::addNewEntity(TypeBaseline);
    AbstractEntityDataModel::setEntityData(uid, BaseLineData, QVariant::fromValue(baseLineModel));
  }
  else
  {
    auto bline = getEntity(blinesUids.first());
    if(bline->constFind(BaseLineData) == bline->end())
    {
      baseLineModel.reset(new BaseLineDataModel());
      AbstractEntityDataModel::setEntityData(blinesUids.first(), BaseLineData, QVariant::fromValue(baseLineModel));
    }
    else
      baseLineModel = bline->constFind(BaseLineData)->value<TBaseLineDataModel>();
  }
  for(auto& strKey : jblines.keys())
  {
    int smoothFactor = strKey.toInt();
    QJsonArray jbline = jblines[strKey].toArray();
    QVector<double> keys;
    QVector<double> values;
    for(int i = 0; i < jbline.size(); i++)
    {
      auto jpt = jbline[i].toObject();
      keys << jpt["x"].toDouble();
      values << jpt["y"].toDouble();
    }
    baseLineModel->addBaseLine(smoothFactor, keys, values);
  }
  baseLineModel->setBaseLine(defaultBaseline);
}

void ChromatogramDataModel::parseJIntervals(const QJsonArray &intervals)
{
  emit modelAboutToReset();
  {
    QSignalBlocker sb(this);
    clearEntities(TypeInterval);
    for(int i = 0; i < intervals.size(); i++)
    {
      auto uid = AbstractEntityDataModel::addNewEntity(TypeInterval);
      auto jinterval = intervals[i].toObject();
      AbstractEntityDataModel::setEntityData(uid, IntervalLeft, jinterval["x_start"].toDouble());
      AbstractEntityDataModel::setEntityData(uid, IntervalRight,  jinterval["x_end"].toDouble());
      AbstractEntityDataModel::setEntityData(uid, IntervalType, jinterval["interval_type"].toInt());

    }
  }
  emit modelReset();
}

bool ChromatogramDataModel::isMaster() const
{
  if(!mMarkupModel || !mMarkupModel->hasMaster())
    return false;
  return mMarkupModel->getMaster()->id() == id();
}

const QHash<int, QVariant> &ChromatogramDataModel::internalDataReference() const
{
  return mData;
}

QJsonObject ChromatogramDataModel::print(const QList<int> &ignoreRoles, const QList<int> &ignoreTypes)
{
  QMetaEnum entityEnum = QMetaEnum::fromType<MarkupEntityTypes>();
  QMetaEnum dataRoleEnum = QMetaEnum::fromType<ChromatogrammEntityDataRoles>();
  QMetaEnum entityDataRoleEnum = QMetaEnum::fromType<EntityDataRoles>();
  QMetaEnum privateRolesEnum = QMetaEnum::fromType<ChromatogrammDataRoles>();
  QJsonObject root;
  QJsonObject jchromaData;
  QJsonArray jchromaEntities;
  QJsonObject jmodelData;
  auto data = getData();
  for(auto& entityUid : data.keys())
  {
    auto variantMap = data[entityUid];
    if(auto it = variantMap->constFind(RoleEntityType); it == variantMap->constEnd()
       || ignoreTypes.contains(it.value().toInt()))
      continue;
    QJsonObject jentity;
    jentity["uid"] = entityUid.toString();
    QJsonObject jvariantMap;
    for(auto& dataRole : variantMap->keys())
    {
      if(ignoreRoles.contains(dataRole))
        continue;
      const auto& map = (*variantMap);
      if(QString stringKey = entityDataRoleEnum.valueToKey(dataRole); !stringKey.isEmpty())
      {//here we handle entity base data
        switch((EntityDataRoles)dataRole)
        {
        case EntityDataRoles::RoleEntityType:
        {
          jentity[stringKey] = entityEnum.valueToKey(map[dataRole].toInt());
        }
          break;
        default:
          break;
        }
      }
      else if(QString stringKey = dataRoleEnum.valueToKey(dataRole); !stringKey.isEmpty())
      {
        switch((ChromatogrammEntityDataRoles)dataRole)
        {
        case BaseLineData:
        {
          QJsonObject jlines;
          auto baseLineModel = map[dataRole].value<TBaseLineDataModel>();
          if(baseLineModel)
          {
            const auto& map = baseLineModel->data();
            for(auto iter = map.constBegin(); iter != map.constEnd(); iter++)
            {
              const auto& line = *iter;
              QJsonArray jpoints;
              for(int i = 0; i < line.first.size(); i++)
              {
                QJsonObject jpt;
                jpt["key"] = line.first[i];
                jpt["value"] = line.second[i];
                jpoints << jpt;
              }
              jlines[QString::number(iter.key())] = jpoints;
            }
            jentity[stringKey] = jlines;
          }
          break;
        }
        case PeakStart:
        case PeakEnd:
        case PeakTop:
        case PeakBottom:
        {
          QPointF pt = map[dataRole].toPointF();
          QJsonObject jpt;
          jpt["x"] = pt.x();
          jpt["y"] = pt.y();
          jentity[stringKey] = jpt;
        }
          break;
        case PeakRetentionTime:
        case PeakCovatsIndex:
        case PeakMarkerWindow:
        case PeakArea:
        case PeakHeight:
        case IntervalLeft:
        case IntervalRight:
        case PeakFormFactor:
          jentity[stringKey] = map[dataRole].toDouble();
          break;
        case PeakTitle:
          //        case PassportTitle:
          //        case PassportFilename:
          //        case PassportWell:
          //        case PassportField:
          //        case PassportLayer:
          jentity[stringKey] = map[dataRole].toString();
          break;
          //        case PassportDate:
          //          jentity[stringKey] = map[dataRole].toDateTime().toString();
          //          break;
        case IntervalSettings:
          //TODO: do something gere
          break;
        case IntervalType:
          //TODO: convert interval type to string
          //          jentity[stringKey] = map[dataRole].toInt();
          break;
        case PeakType:
        {
          auto e = QMetaEnum::fromType<PeakTypes>();
          jentity[stringKey] = e.valueToKey(map[dataRole].toInt());
        }
          break;
        case PeakColor:
          jentity[stringKey] = map[dataRole].value<QColor>().name();
          break;
        case PeakId:
          //        case PassportFileId:
          jentity[stringKey] = map[dataRole].toInt();
          break;
        case CurveData://curve data not saves
        {
          CurveDataModelPtr model = map[dataRole].value<CurveDataModelPtr>();
          QJsonArray keys;
          QJsonArray values;
          for(auto& v : model->keys())
            keys << v;
          for(auto& k : model->values())
            values << k;
          QJsonObject curve;
          curve["keys"] = keys;
          curve["values"] = values;
          jentity[stringKey] = curve;
          break;
        }

        default:
          break;
        }
      }
    }
    jchromaEntities << jentity;
  }
  jchromaData["entities"] = jchromaEntities;
  root["chromatogrammData"] = jchromaData;

  for(auto& key : mData.keys())
  {
    auto stringKey = privateRolesEnum.valueToKey(key);
    jmodelData[stringKey] = QJsonValue::fromVariant(mData[key]);
  }
  root["modelData"] = jmodelData;
  return root;
}

MarkupDataModel* ChromatogramDataModel::markupModel() const
{
  return mMarkupModel;
}

void ChromatogramDataModel::setMarkupModel(MarkupDataModel* newModel)
{
  mMarkupModel = newModel;
}

void ChromatogramDataModel::setChromatogramValue(int DataRole, QVariant newValue)
{
  mData[DataRole] = newValue;
  if(DataRole == ChromatogrammSettingsBaseLineSmoothLvl)
  {
    auto blines = getEntities(TypeBaseline);
    if(!blines.isEmpty())
      if(auto bline = blines.first()->getData(BaseLineData).value<TBaseLineDataModel>())
        bline->setBaseLine(newValue.toInt());
  }
  emit chromatogramDataChanged(DataRole, newValue);
}

QVariant ChromatogramDataModel::getChromatogramValue(int DataRole)
{
  if(!mData.contains(DataRole))
    return QVariant();
  return mData[DataRole];
}

ChromatogrammModelPtr ChromatogramDataModel::getCopyWithFilteredEntities(std::function<bool (ConstDataPtr)> filter)
{
  ChromatogrammModelPtr newChroma(new ChromatogramDataModel(id()));
  newChroma->mData = mData;
  newChroma->mMarkupModel = mMarkupModel;
  AbstractEntityDataModel::copyEntitiesIf(this, newChroma.get(), [](ConstDataPtr entity)->bool
  {
    auto eType = entity->getType();
    return eType == TypeBaseline || eType == TypeCurve;
  });
  return newChroma;
}

QString ChromatogramDataModel::markerPrefix() const
{
  return mMarkerPrefix;
}

void ChromatogramDataModel::setMarkerPrefix(const QString &newMarkerPrefix)
{
  if (mMarkerPrefix == newMarkerPrefix)
    return;
  mMarkerPrefix = newMarkerPrefix;
  emit markerPrefixChanged();
}

void ChromatogramDataModel::resetMarkerPrefix()
{
  setMarkerPrefix("M"); // TODO: Adapt to use your actual default value
}

QColor ChromatogramDataModel::peakColor() const
{
  return mPeakColor;
}

void ChromatogramDataModel::setPeakColor(const QColor &newPeakColor)
{
  if (mPeakColor == newPeakColor)
    return;
  mPeakColor = newPeakColor;
  emit peakColorChanged();
}

void ChromatogramDataModel::resetPeakColor()
{
  setPeakColor(DefaultPeakColor); // TODO: Adapt to use your actual default value
}

QColor ChromatogramDataModel::markerColor() const
{
  return mMarkerColor;
}

void ChromatogramDataModel::setMarkerColor(const QColor &newMarkerColor)
{
  if (mMarkerColor == newMarkerColor)
    return;
  mMarkerColor = newMarkerColor;
  emit markerColorChanged();
}

void ChromatogramDataModel::resetMarkerColor()
{
  setMarkerColor(DefaultMarkerColor); // TODO: Adapt to use your actual default value
}

QList<QColor> ChromatogramDataModel::interMarkerColors() const
{
  return mInterMarkerColors;
}

void ChromatogramDataModel::setInterMarkerColors(const QList<QColor> &newInterMarkerColors)
{
  if (mInterMarkerColors == newInterMarkerColors)
    return;
  mInterMarkerColors = newInterMarkerColors;
  emit interMarkerColorsChanged();
}

void ChromatogramDataModel::resetInterMarkerColors()
{
  setInterMarkerColors(DefaultInterMarkerColors); // TODO: Adapt to use your actual default value
}

QHash<int, QVariant> ChromatogramDataModel::data() const
{
  return mData;
}

void ChromatogramDataModel::setData(const QHash<int, QVariant> &newData)
{
  mData = newData;
}


bool ChromatogramDataModel::setEntityData(TEntityUid Id, DataRoleType role, const QVariant &value)
{
  auto oldValue = getEntity(Id)->getData(role);
  if(oldValue == value)
    return true;

  auto entity = getEntity(Id);
  // if(entity->getData(entityTypeData).toInt() == TypeKeySelector &&
  //     role == KeySelectorKey)
  // {
  //   auto curve = getEntities(TypeCurve).first();
  //   auto curveData = curve->getData(CurveData).value<TCurveData>();
  //   auto keys = curveData->keys();
  //   if(!keys.contains(value.toDouble()))
  //   {
  //     QVariant newValue = value;
  //     double dval = value.toDouble();
  //     auto it = std::lower_bound(keys.begin(), keys.end(), dval);
  //     if(it == keys.end())
  //       newValue = keys.last();
  //     else if(it == keys.begin())
  //       newValue = keys.first();

  //     if(*it - dval < *std::prev(it) - dval)
  //       newValue = *it;
  //     else
  //       newValue = *std::prev(it);
  //     return AbstractEntityDataModel::setEntityData(Id, role, newValue);
  //   }
  //   else
  //     return AbstractEntityDataModel::setEntityData(Id, role, value);
  // }
  if(entity->getType() == TypePeak)
  {
    if(role == PeakType)
    {
      if((oldValue.toInt() == PTMarker || oldValue.toInt() == PTInterMarker) ||
         (value.toInt() == PTMarker || value.toInt() == PTInterMarker))
      {
        blockSignals(true);
        bool result;
        if((result = AbstractEntityDataModel::setEntityData(Id, role, value)))
        {
          blockSignals(false);
          emit modelAboutToReset();
          blockSignals(true);
          resolveAllPeaksOrderingNamingAndColoring({oldValue.toInt(), value.toInt()});
          if(value.toInt() == PTMarker)
            validatePeakRequiredValues(Id);
          blockSignals(false);
          emit modelReset();
        }
        else
        {
          blockSignals(false);
        }
        return result;
      }
      else if(value.toInt() == PTPeak)
      {
        AbstractEntityDataModel::setEntityData(Id, PeakColor, mPeakColor);
        AbstractEntityDataModel::setEntityData(Id, PeakTitle, "X");
        AbstractEntityDataModel::setEntityData(Id, PeakId, 0);
        return AbstractEntityDataModel::setEntityData(Id, role, value);
      }
      else
      {
        return AbstractEntityDataModel::setEntityData(Id, role, value);
      }
    }
  }

  if(auto result = AbstractEntityDataModel::setEntityData(Id, role, value))
  {
    if(getEntity(Id)->getType() == TypePeak)
    {
      switch((ChromatogrammEntityDataRoles)role)
      {
      case PeakId:
      case PeakCompoundTitle:
        resolvePeakNaming(Id, Core::GenesisContextRoot::Get()->GetContextMarkup()->GetData(Names::MarkupContextTags::kPeakTitleDisplayMode).value<TChromatogrammTitlesDisplayMode>());
        break;
      default:
        break;
      }
    }
    return result;
  }
  return false;
}

bool ChromatogramDataModel::resetEntityData(TEntityUid Id, const DataModel &newData)
{
  if(newData.getType() == TypePeak)
  {
    int oldType = getEntity(Id)->getData(PeakType).toInt();//return QVariant() if no such role, which converts to zero int
    if(newData.getData(PeakType).toInt() == PTMarker)
    {
      if(!newData.hasDataAndItsValid(PeakMarkerWindow))
        const_cast<DataModel&>(newData)[PeakMarkerWindow] = 2.0;
      if(!newData.hasDataAndItsValid(PeakCovatsIndex))
        const_cast<DataModel&>(newData)[PeakCovatsIndex] = 0.0;
    }

    if(newData.contains(PeakType))
    {
      auto newType = newData.getData(PeakType).toInt();
      if(newType != PTMarker && newType != PTInterMarker)
      {
        if(newType == PTPeak)
        {
          const_cast<DataModel&>(newData)[PeakColor] = mPeakColor;
          const_cast<DataModel&>(newData)[PeakTitle] = "X";
          const_cast<DataModel&>(newData)[PeakId] = 0;
        }
        if(oldType == PTPeak)
        {
          return AbstractEntityDataModel::resetEntityData(Id, newData);
        }
      }
      blockSignals(true);
      if(auto result = AbstractEntityDataModel::resetEntityData(Id, newData))
      {
        blockSignals(false);
        emit modelAboutToReset();
        blockSignals(true);

        resolveAllPeaksOrderingNamingAndColoring({oldType, newType});

        blockSignals(false);
        emit modelReset();
        return true;
      }
      else
      {
        blockSignals(false);
        return false;
      }
    }
  }

  return AbstractEntityDataModel::resetEntityData(Id, newData);
//  if(!result)
//    return result;

//  if(getEntity(Id)->getData(entityTypeData).toInt() == TypePeak)
//  {
//    int newType = getEntity(Id)->getData(PeakType).toInt();
//    resolveAllPeaksOrderingNamingAndColoring({oldType, newType});
////    resolvePeakColoring(Id, newType);
////    resolvePeakOrderingAndColoring(Id, newType, {oldType, newType});
////    resolvePeakNaming(Id);
//  }
//  return result;
}

QUuid ChromatogramDataModel::addNewEntity(EntityType type, const DataModel &data, QUuid uid)
{
  if(data.getType() == TypeBaseline)
  {
    if(auto blinePtr = data.getData(BaseLineData).value<TBaseLineDataModel>())
    {
    }
  }
  if(data.getType() == TypePeak)
  {
    if(data.hasDataAndItsValid(PeakType))
    {
      auto newType = data.getData(PeakType).toInt();
      if(newType != PTMarker && newType != PTInterMarker)
      {
        if(newType == PTPeak)
        {
          const_cast<DataModel&>(data)[PeakColor] = mPeakColor;
          const_cast<DataModel&>(data)[PeakTitle] = "X";
          const_cast<DataModel&>(data)[PeakId] = 0;
        }
        return AbstractEntityDataModel::addNewEntity(type, data, uid);
      }

      if(newType == PTMarker)
      {
        if(!data.hasDataAndItsValid(PeakMarkerWindow))
          const_cast<DataModel&>(data)[PeakMarkerWindow] = 2.0;
        if(!data.hasDataAndItsValid(PeakCovatsIndex))
          const_cast<DataModel&>(data)[PeakCovatsIndex] = 0.0;
      }

      blockSignals(true);
      QUuid result = AbstractEntityDataModel::addNewEntity(type, data, uid);
      if(!result.isNull())
      {
        blockSignals(false);
        emit modelAboutToReset();
        blockSignals(true);

        resolveAllPeaksOrderingNamingAndColoring({newType});

        blockSignals(false);
        emit modelReset();
      }
      else
      {
        blockSignals(false);
      }
      return result;
    }
  }

  return AbstractEntityDataModel::addNewEntity(type, data, uid);
}

QUuid ChromatogramDataModel::addNewEntity(EntityType type, DataModel *data, QUuid uid)
{
  if(!data)
    data = new DataModel();
  if(data->getType() == TypePeak)
  {
    if(data->hasDataAndItsValid(PeakType))
    {
      auto newType = data->getData(PeakType).toInt();
      if(newType != PTMarker && newType != PTInterMarker)
      {
        if(newType == PTPeak)
        {
          (*data)[PeakColor] = mPeakColor;
          (*data)[PeakTitle] = "X";
          (*data)[PeakId] = 0;
        }
        return AbstractEntityDataModel::addNewEntity(type, data, uid);
      }

      if(newType == PTMarker)
      {
        if(!data->hasDataAndItsValid(PeakMarkerWindow))
          (*data)[PeakMarkerWindow] = 2.0;
        if(!data->hasDataAndItsValid(PeakCovatsIndex))
          (*data)[PeakCovatsIndex] = 0.0;
      }

      blockSignals(true);
      QUuid result;
      if(!(result = AbstractEntityDataModel::addNewEntity(type, data, uid)).isNull())
      {
        blockSignals(false);
        emit modelAboutToReset();
        blockSignals(true);

        resolveAllPeaksOrderingNamingAndColoring({newType});

        blockSignals(false);
        emit modelReset();
      }
      else
      {
        blockSignals(false);
      }
      return result;
    }
  }
  return AbstractEntityDataModel::addNewEntity(type, data, uid);
}

bool ChromatogramDataModel::removeEntity(TEntityUid Id)
{
  auto entity = getEntity(Id);
  if(entity->getType() == TypePeak)
  {
    auto peakType = entity->getData(PeakType).toInt();
    if(peakType != PTMarker && peakType != PTInterMarker)
      return AbstractEntityDataModel::removeEntity(Id);

    blockSignals(true);
    if(auto result = AbstractEntityDataModel::removeEntity(Id))
    {
      blockSignals(false);
      emit modelAboutToReset();
      blockSignals(true);

      resolveAllPeaksOrderingNamingAndColoring({peakType});

      blockSignals(false);
      emit modelReset();
      return result;
    }
    else
    {
      blockSignals(false);
      return result;
    }
  }

  return AbstractEntityDataModel::removeEntity(Id);
}

void ChromatogramDataModel::resolveAllPeaksOrderingNamingAndColoring(QList<int> affectedPeaksTypes)
{
  if(mPeaksOrderingMap.isEmpty())
  {
    recalcPeaksOrderMap();
  }
  auto orderedPeaksUids = getOrderedPeaks();
  int interInd = 1;//startInd
  int interColorInd = 0;
  int markerInd = -1;//startInd
  const int peakInd = 0;//always zero
  const int markerStep = -1;
  const int interStep = 1;
  bool affectedPeaks = affectedPeaksTypes.contains(PTPeak);
  bool affectedMarkers = affectedPeaksTypes.contains(PTMarker);
  bool affectedInterMarkers = affectedPeaksTypes.contains(PTInterMarker);
  auto titleMode = Core::GenesisContextRoot::Get()->GetContextMarkup()->GetData(Names::MarkupContextTags::kPeakTitleDisplayMode).value<TChromatogrammTitlesDisplayMode>();
  for(auto& uid : orderedPeaksUids)
  {
    auto peak = getEntity(uid);
    switch((PeakTypes)(peak->getData(PeakType).toInt()))
    {
    case PTPeak:
      if(affectedPeaks)
      {
        if(peak->getData(PeakId).toInt() != peakInd)
          AbstractEntityDataModel::setEntityData(uid, PeakId, peakInd);
        if(peak->getData(PeakColor).value<QColor>() != mPeakColor)
        {
          AbstractEntityDataModel::setEntityData(uid, PeakColor, mPeakColor);
        }
      }
      break;
    case PTMarker:
      interColorInd = 0;
      if(affectedMarkers)
      {
        if(peak->getData(PeakId).toInt() != markerInd)
          AbstractEntityDataModel::setEntityData(uid, PeakId, markerInd);
        if(peak->getData(PeakColor).value<QColor>() != mMarkerColor)
          AbstractEntityDataModel::setEntityData(uid, PeakColor, mMarkerColor);
        markerInd += markerStep;
      }
      break;
    case PTInterMarker:
      if(affectedInterMarkers)
      {
        if(isMaster())
        {
          if(peak->getData(PeakId).toInt() != interInd)
            AbstractEntityDataModel::setEntityData(uid, PeakId, interInd);
        }
        else
        {
          auto interTitle = QString::number(peak->getData(PeakId).toInt());
        }
        auto interColor = mInterMarkerColors[(interColorInd) % mInterMarkerColors.size()];
        if(peak->getData(PeakColor).value<QColor>() != interColor)
          AbstractEntityDataModel::setEntityData(uid, PeakColor, interColor);
        interInd += interStep;
        interColorInd += interStep;
      }
      break;
    default:
      break;
    }
    resolvePeakNaming(uid, titleMode);
  }

}

void ChromatogramDataModel::resolvePeakNaming(TEntityUid Id, ChromatogrammTitlesDisplayModes mode)
{
  auto ent = getEntity(Id);

  switch(ent->getData(PeakType).toInt())
  {
  case PTPeak:
  {
    switch(mode)
    {
    case TitleModeRetentionTime:
      if(ent->hasDataAndItsValid(PeakRetentionTime))
        AbstractEntityDataModel::setEntityData(Id, PeakTitle, QString::number(ent->getData(PeakRetentionTime).toDouble()));
      else
        AbstractEntityDataModel::setEntityData(Id, PeakTitle, QString(""));
      break;
    case TitleModeCompoundTitle:
    case TitleModeIndex:
    case TitleModeOrder:
    case TitleModeDefault:
    default:
      AbstractEntityDataModel::setEntityData(Id, PeakTitle, QString("X"));
      break;
    }
    break;
  }
  case PTMarker:
  {
    switch(mode)
    {
    case TitleModeCompoundTitle:
      //empty string if fail
      AbstractEntityDataModel::setEntityData(Id, PeakTitle, ent->getData(PeakCompoundTitle).toString());
      break;
    case TitleModeIndex:
      if(ent->hasDataAndItsValid(PeakCovatsIndex))
        AbstractEntityDataModel::setEntityData(Id, PeakTitle, QString::number(ent->getData(PeakCovatsIndex).toDouble()));
      else
        AbstractEntityDataModel::setEntityData(Id, PeakTitle, QString(""));
      break;
    case TitleModeRetentionTime:
      if(ent->hasDataAndItsValid(PeakRetentionTime))
        AbstractEntityDataModel::setEntityData(Id, PeakTitle, QString::number(ent->getData(PeakRetentionTime).toDouble()));
      else
        AbstractEntityDataModel::setEntityData(Id, PeakTitle, QString(""));
      break;
    case TitleModeOrder:
    case TitleModeDefault:
      AbstractEntityDataModel::setEntityData(Id, PeakTitle, QString("%1%2").arg(mMarkerPrefix).arg(( -getEntity(Id)->getData(PeakId).toInt())));
      break;
    default:
      break;
    }

    // if(ent->hasData(PeakCompoundTitle) && !ent->getData(PeakCompoundTitle).toString().isEmpty())
    //   AbstractEntityDataModel::setEntityData(Id, PeakTitle, ent->getData(PeakCompoundTitle).toString());
    // else
    //   AbstractEntityDataModel::setEntityData(Id, PeakTitle, QString("%1%2").arg(mMarkerPrefix).arg(( -getEntity(Id)->getData(PeakId).toInt())));
    break;
  }
  case PTInterMarker:
  {
    switch(mode)
    {
    case TitleModeCompoundTitle:
      //empty string if fail
      AbstractEntityDataModel::setEntityData(Id, PeakTitle, ent->getData(PeakCompoundTitle).toString());
      break;
    case TitleModeIndex:
      if(ent->hasDataAndItsValid(PeakCovatsIndex))
        AbstractEntityDataModel::setEntityData(Id, PeakTitle, QString::number(ent->getData(PeakCovatsIndex).toDouble()));
      else
        AbstractEntityDataModel::setEntityData(Id, PeakTitle, QString(""));
      break;
    case TitleModeRetentionTime:
      if(ent->hasDataAndItsValid(PeakRetentionTime))
        AbstractEntityDataModel::setEntityData(Id, PeakTitle, QString::number(ent->getData(PeakRetentionTime).toDouble()));
      else
        AbstractEntityDataModel::setEntityData(Id, PeakTitle, QString(""));
      break;
    case TitleModeOrder:
    case TitleModeDefault:
      AbstractEntityDataModel::setEntityData(Id, PeakTitle, QString("%1").arg((getEntity(Id)->getData(PeakId).toInt())));
    default:
      break;
    }
    break;
  }
  default:
    break;
  }
}

//void ChromatogramDataModel::resolvePeaksNaming()
//{
//  for(auto& peak : getEntities(TypePeak))
//  {
//    auto id = peak->getData(entityUid).toUuid();
//    switch(peak->getData(PeakType).toInt())
//    {
//    case PTMarker:
//      AbstractEntityDataModel::setEntityData(id, PeakTitle, QString("M%1").arg(( -getEntity(id)->getData(PeakId).toInt())));
//      break;
//    case PTInterMarker:
//      AbstractEntityDataModel::setEntityData(id, PeakTitle, QString("%1").arg((getEntity(id)->getData(PeakId).toInt())));
//      break;
//    case PTPeak:
//      AbstractEntityDataModel::setEntityData(id, PeakTitle, QString("X"));
//      break;
//    default:
//      break;
//    }
//  }
//}

//void ChromatogramDataModel::resolvePeakColoring(EntityUId Id, int newType)
//{
//  //resolve changed peak coloring
//  if(newType == PTMarker)
//    AbstractEntityDataModel::setEntityData(Id, PeakColor, DefaultMarkerColor);
//  else if(newType == PTPeak)
//    AbstractEntityDataModel::setEntityData(Id, PeakColor, DefaultPeakColor);
//  //
//}

//void ChromatogramDataModel::resolveInterMarkersIndexing()
//{
//  QList<ConstDataPtr> interMarkers;
//  for(auto& peak : getEntities(TypePeak))
//    if(peak->getData(PeakType).toInt() == PTInterMarker)
//      interMarkers << peak;

//  std::sort(interMarkers.begin(), interMarkers.end(), [](const AbstractEntityDataModel::ConstDataPtr& a,
//            const AbstractEntityDataModel::ConstDataPtr& b)->bool
//  {
//    return a->getData(PeakRetentionTime).toDouble() < b->getData(PeakRetentionTime).toDouble();
//  });
//  for(int i = 0; i < interMarkers.size(); ++i)
//    setEntityData(interMarkers[i]->getData(entityUid).toUuid(), PeakId, i+1);
//}

void ChromatogramDataModel::validatePeakRequiredValues(TEntityUid peakId)
{
  auto entity = getEntity(peakId);
  if(!entity->hasDataAndItsValid(PeakMarkerWindow) || entity->getData(PeakMarkerWindow).toInt() <= 0.1)
    AbstractEntityDataModel::setEntityData(peakId, PeakMarkerWindow, 2.0);
}

void ChromatogramDataModel::validatePeakRequiredValues(DataModel peakData)
{
  if(peakData[PeakMarkerWindow].toInt() <= 0.1)
    peakData[PeakMarkerWindow] = 2;
}

void ChromatogramDataModel::onEntityChanged(EntityType type, TEntityUid eId, DataRoleType role, const QVariant &value)
{
  if(type == TypePeak && role == PeakStart)
  {
    mPeaksOrderingMap.removeIf([eId](std::pair<const double&, const QUuid&> v)->bool{return v.second == eId;});
    if(value.isValid() && !value.isNull())
      mPeaksOrderingMap.insert(value.toPointF().x(), eId);
  }
  if(type == TypeInterval && role == IntervalLeft)
  {
    mIntervalsOrderingMap.removeIf([eId](std::pair<const double&, const QUuid&> v)->bool{return v.second == eId;});
    if(value.isValid() && !value.isNull())
      mIntervalsOrderingMap.insert(value.value<TIntervalLeft>(), eId);
  }
}

void ChromatogramDataModel::onEntityResetted(EntityType type, TEntityUid eId, ConstDataPtr data)
{
  if(type == TypePeak)
  {
    mPeaksOrderingMap.removeIf([eId](std::pair<const double&, const QUuid&> v)->bool{return v.second == eId;});
    if(data->hasDataAndItsValid(PeakStart) && !data->getData(PeakStart).isNull())
      mPeaksOrderingMap.insert(data->getData(PeakStart).toPointF().x(), eId);
  }
  if(type == TypeInterval)
  {
    mIntervalsOrderingMap.removeIf([eId](std::pair<const double&, const QUuid&> v)->bool{return v.second == eId;});
    if(data->hasDataAndItsValid(IntervalLeft) && !data->getData(IntervalLeft).isNull())
      mIntervalsOrderingMap.insert(data->getData(IntervalLeft).value<TIntervalLeft>(), eId);
  }
}

void ChromatogramDataModel::onEntityAdded(EntityType type, TEntityUid eId, ConstDataPtr data)
{
  if(type == TypePeak)
  {
//    mPeaksOrderingMap.removeIf([eId](std::pair<const double&, const QUuid&> v)->bool{return v.second == eId;}); not needed if mPeaksOrderingMap is valid
    if(data->hasDataAndItsValid(PeakStart) && !data->getData(PeakStart).isNull())
      mPeaksOrderingMap.insert(data->getData(PeakStart).toPointF().x(), eId);
  }
  if(type == TypeInterval)
  {
    if(data->hasDataAndItsValid(IntervalLeft) && !data->getData(IntervalLeft).isNull())
      mIntervalsOrderingMap.insert(data->getData(IntervalLeft).value<TIntervalLeft>(), eId);
  }
}

void ChromatogramDataModel::onEntityAboutToRemove(EntityType type, TEntityUid uid)
{
  if(type == TypePeak)
  {
    mPeaksOrderingMap.removeIf([uid](std::pair<const double&, const QUuid&> v)->bool{return v.second == uid;});
  }

  if(type == TypeInterval)
  {
    mIntervalsOrderingMap.removeIf([uid](std::pair<const double&, const QUuid&> v)->bool{return v.second == uid;});
  }
}

void ChromatogramDataModel::onModelAboutToReset()
{
  mPeaksOrderingMap.clear();
  mIntervalsOrderingMap.clear();
}

void ChromatogramDataModel::onModelReset()
{
  recalcPeaksOrderMap();
}

void ChromatogramDataModel::recalcPeaksOrderMap()
{
  if(!mPeaksOrderingMap.empty())// just for safety
    mPeaksOrderingMap.clear();
  for(auto& peak : getEntities(TypePeak))
  {
    if(peak->hasDataAndItsValid(PeakStart) && !peak->getData(PeakStart).isNull())
      mPeaksOrderingMap[peak->getData(PeakStart).toPointF().x()] = peak->getData(RoleEntityUid).toUuid();
  }

  if(!mIntervalsOrderingMap.empty())// just for safety
    mIntervalsOrderingMap.clear();
  for(auto& interval : getEntities(TypeInterval))
  {
    if(interval->hasDataAndItsValid(IntervalLeft) && !interval->getData(IntervalLeft).isNull())
      mIntervalsOrderingMap[interval->getData(IntervalLeft).value<TIntervalLeft>()] = interval->getData(RoleEntityUid).toUuid();
  }
}



} //namespace GenesisMarkup
