#include "chromatogram_plot.h"
#include "logic/markup/commands.h"
#include "ui/dialogs/chromatogram_widget_settings_dialog_refactor.h"
#include "ui/plots/gp_items/chromatogram_baseline.h"
#include "ui/plots/gp_items/chromatogram_peak.h"
#include "ui/plots/gp_items/chromatogram_interval.h"
#include <QMouseEvent>
#include <logic/markup/baseline_data_model.h>
#include <logic/markup/curve_data_model.h>
#include <ui/controls/spinbox_action.h>
#include <ui/plots/gp_items/chromatogram_key_selector.h>
#include <logic/context_root.h>
#include <logic/known_context_tag_names.h>

using namespace GenesisMarkup;
using namespace Names;

QPen defaultChromatogramCurvePen = QPen(QColor(0, 32, 51, 150));

ChromatogramPlot::ChromatogramPlot(QWidget* parent) : GraphicsPlot(parent), DraggingObject(nullptr), contextMenuLocked(false), baseLineInteractionsMode(false)
//  : GraphicsPlotExtended(parent)
{
  hide();
  setPlottingHint(GP::phImmediateRefresh, false);
  setInteraction(GP::Interaction::iSelectOther, true);
  setInteraction(GP::Interaction::iSelectPlottables, true);
  setInteraction(GP::Interaction::iMultiSelect, true);
  setMultiSelectModifier(Qt::ShiftModifier);
  setInteraction(GP::Interaction::iSelectAxes, false);
  setInteraction(GP::Interaction::iSelectItems, false);
  setInteraction(GP::Interaction::iSelectLegend, false);

  setInteraction(GP::iRangeDrag, true);
  setInteraction(GP::iRangeZoom, true);

  mMode = GenesisMarkup::CMMarkup;
  xAxis->setPadding(0);
  xAxis->setLabelPadding(0);
  yAxis->setPadding(0);
  yAxis->setLabelPadding(0);
  QColor baseColor = QColor(0, 32, 51, 153);
  xAxis->setBasePen(baseColor);
  yAxis->setBasePen(baseColor);
  xAxis->setTickPen(baseColor);
  yAxis->setTickPen(baseColor);
  xAxis->setTickLabelColor(baseColor);
  yAxis->setTickLabelColor(baseColor);
  for(auto& r : axisRects())
  {
    r->setMinimumMargins(QMargins(0,0,0,0));
    r->setMargins(QMargins(0,0,0,0));
  }

  setupUiSettings();
}

void ChromatogramPlot::setInteractionsLocked(bool locked)
{
  deselectAll();
  if(locked)
  {
    setInteraction(GP::Interaction::iSelectOther, false);
    setInteraction(GP::Interaction::iSelectPlottables, false);
    setInteraction(GP::Interaction::iMultiSelect, false);
    contextMenuLocked = true;
  }
  else
  {
    setInteraction(GP::Interaction::iSelectOther, true);
    setInteraction(GP::Interaction::iSelectPlottables, true);
    setInteraction(GP::Interaction::iMultiSelect, true);
    contextMenuLocked = false;
  }
}

void ChromatogramPlot::setChromatogrammModel(GenesisMarkup::ChromatogrammModelPtr model, const GenesisMarkup::StepInfo& modelStepInteractions)
{
  if (mModel)
    disconnect(mModel.get(), nullptr, this, nullptr);
  onModelAboutToReset();
  mModel = model;
  handleStepInteractions(modelStepInteractions);
  connectModelSignals(mModel);
  onModelReset();
}

QPair<GPRange, GPRange> ChromatogramPlot::boundingRange()
{
  bool rangeFound = false;
  bool firstEntry = true;
  GPRange newXRange;
  GPRange newYRange;
  GPCurve* curve = nullptr;

  for (auto& layerablePtr : mLayerables.getHash())
  {
    // this property validity and existing managed by mLayerables itself
    auto uid = layerablePtr->property("uid").toUuid();
    auto entity = mModel->getEntity(uid);
    ////if there is such entity, if this entity has a type, and if this type is
    /// TypeCurve, we go through
    if (!entity)
      continue;
    auto dataIter = entity->constFind(AbstractEntityDataModel::RoleEntityType);
    if (dataIter == entity->constEnd())
      continue;
    if (dataIter->toInt() != GenesisMarkup::TypeCurve)
      continue;
    ///
    // TODO: fix this when switch to GPGraph instead GPCurve
    ////we know exactly, what type of pointer it is
    curve = static_cast<GPCurve*>(layerablePtr);
    Q_ASSERT(curve != nullptr);
    if (firstEntry)
    {
      firstEntry = false;
      newXRange = curve->getKeyRange(rangeFound);
      Q_ASSERT(rangeFound);
      newYRange = curve->getValueRange(rangeFound, GP::sdBoth, newXRange);
      Q_ASSERT(rangeFound);
    }
    else
    {
      newXRange.expand(curve->getKeyRange(rangeFound));
      newYRange.expand(curve->getValueRange(rangeFound, GP::sdBoth, newXRange));
    }
  }
  return {newXRange, newYRange};
}

void ChromatogramPlot::onEntityChanged(EntityType type, TEntityUid uid, DataRoleType role, const QVariant& value)
{
  Q_ASSERT(mModel != nullptr);
  if (!mLayerables.contains(uid))
  {
    onEntityAdded(type, uid, mModel->getEntity(uid));
    return;
  }
  auto layerable = mLayerables.get(uid);

  switch((GenesisMarkup::MarkupEntityTypes)type)
  {
  case GenesisMarkup::TypeCurve: {
    auto curve = static_cast<GPGraph*>(layerable);
    switch ((GenesisMarkup::ChromatogrammEntityDataRoles)role)
    {
    case GenesisMarkup::CurveData: {
      if (!value.isValid() || value.isNull())
      {
        curve->data()->clear();
        if (curve->property("modelPtr").isValid() && !curve->property("modelPtr").isNull())
        {
          disconnect(curve->property("modelPtr").value<GenesisMarkup::CurveDataModelPtr>().get(), nullptr, curve, nullptr);
          curve->setProperty("modelPtr", QVariant());
        }
      }
      else
      {
        auto curveModelPtr = value.value<GenesisMarkup::CurveDataModelPtr>();
        auto oldCurveModelPtr = curve->property("modelPtr").value<GenesisMarkup::CurveDataModelPtr>();
        if (oldCurveModelPtr)
          disconnect(oldCurveModelPtr.get(), nullptr, curve, nullptr);
        curve->setProperty("modelPtr", QVariant::fromValue(curveModelPtr));
        if (curveModelPtr)
        {
          curve->setData(curveModelPtr->keys(), curveModelPtr->values());
          connect(curveModelPtr.get(), &CurveDataModel::curveDataChanged, curve, [curve, curveModelPtr, this]() {
            curve->setData(curveModelPtr->keys(), curveModelPtr->values());
            this->replotOnVisibleViewPort(rpQueuedReplot);
          });
        }
      }
      replotOnVisibleViewPort();
      break;
    }
    default:
      break;
    }
    break;
  }
  case GenesisMarkup::TypePeak: {
    auto peak = static_cast<ChromatogramPeak*>(layerable);
    switch ((GenesisMarkup::ChromatogrammEntityDataRoles)role)
    {
    case PeakStart:
      peak->setLeftBorder(value.toPointF().x());
      peak->layer()->replot();
      break;
    case PeakEnd:
      peak->setRightBorder(value.toPointF().x());
      peak->layer()->replot();
      break;
    case PeakTitle:
      peak->setLabel(value.toString());
      peak->layer()->replot();
      break;
    case PeakId:
      peak->setPeakId(value.toInt());
      break;
    case PeakType:
      peak->setPeakType((PeakTypes)value.toInt());
      peak->updateInteractivityFlags(this, (PeakTypes)value.toInt());

      break;
    case PeakColor: {
      auto color = value.value<QColor>();
      peak->setBrush(color);
      color = color.darker(120);
      peak->setSelectedBrush(color);
      peak->setPen(color);
      peak->setSelectedPen(color.darker(120));
      break;
    }
    default:
      break;
    }

    break;
  }
  case GenesisMarkup::TypeInterval:
  {
    auto interval = static_cast<ChromatogramInterval*>(layerable);
    switch ((GenesisMarkup::ChromatogrammEntityDataRoles)role)
    {
    case PeakStart:
      interval->setLeftBorder(value.toPointF().x());
      interval->layer()->replot();
      break;
    case PeakEnd:
      interval->setRightBorder(value.toPointF().x());
      interval->layer()->replot();
      break;
    case IntervalLeft:
      interval->setLeftBorder(value.value<TIntervalLeft>());
      interval->layer()->replot();
      break;
    case IntervalRight:
      interval->setRightBorder(value.value<TIntervalRight>());
      interval->layer()->replot();
      break;

      //        case PeakId:
      //          interval->setId(value.toInt());
      //          break;
    case IntervalType:
      interval->setIntervalType((IntervalTypes)value.toInt());
      interval->updateInteractivityFlags(this, (IntervalTypes)value.toInt());

      break;
      ///@todo colors
      //        case PeakColor: {
      //          auto color = value.value<QColor>();
      //          interval->setBrush(color);
      //          color = color.darker(120);
      //          interval->setSelectedBrush(color);
      //          interval->setPen(color);
      //          interval->setSelectedPen(color.darker(120));
      //          break;
      //        }
    default:
      break;
    }
  }
    break;
  case GenesisMarkup::TypeBaseline: {
    auto baseLine = static_cast<ChromatogramBaseline*>(layerable);
    switch ((GenesisMarkup::ChromatogrammEntityDataRoles)role)
    {
    case GenesisMarkup::BaseLineData: {
      if (!value.isValid() || value.isNull())
        baseLine->setBaseLineModel(nullptr);
      else
        baseLine->setBaseLineModel(value.value<GenesisMarkup::TBaseLineDataModel>());
      break;
    }
    default:
      break;
    }
  }
  case GenesisMarkup::TypeKeySelector:
    if(role == KeySelectorKey)
    {
      auto selector = static_cast<ChromatogramKeySelector*>(layerable);
      selector->setXCoord(value.toDouble());
    }
    break;
  default:
    break;
  }
}

void ChromatogramPlot::onEntityResetted(EntityType type, TEntityUid uid, AbstractEntityDataModel::ConstDataPtr data)
{  // when entity has massive data update, we need just fully
  // reload this entity.
  Q_ASSERT(mModel != nullptr);
  if (!mLayerables.contains(uid))
  {
    onEntityAdded(type, uid, mModel->getEntity(uid));
    return;
  }
  auto layerable = mLayerables.get(uid);
  switch ((GenesisMarkup::MarkupEntityTypes)type)
  {
  case GenesisMarkup::TypeCurve:
  {
    auto curve = static_cast<GPCurve*>(layerable);
    auto dataIter = data->constFind(GenesisMarkup::CurveData);
    if(dataIter == data->constEnd() || (!dataIter->isValid() || dataIter->isNull()))
    {
      curve->data()->clear();
      if(curve->property("modelPtr").isValid() && !curve->property("modelPtr").isNull())
      {
        disconnect(curve->property("modelPtr").value<GenesisMarkup::CurveDataModelPtr>().get(), nullptr, curve, nullptr);
        curve->setProperty("modelPtr", QVariant());
      }
    }
    else
    {
      auto curveModelPtr = dataIter->value<GenesisMarkup::CurveDataModelPtr>();
      auto oldCurveModelPtr = curve->property("modelPtr").value<GenesisMarkup::CurveDataModelPtr>();
      if(oldCurveModelPtr)
        disconnect(oldCurveModelPtr.get(), nullptr, curve, nullptr);
      curve->setProperty("modelPtr", QVariant::fromValue(curveModelPtr));
      if(curveModelPtr)
      {
        curve->setData(curveModelPtr->keys(), curveModelPtr->values());
        connect(curveModelPtr.get(), &CurveDataModel::curveDataChanged, curve, [curve, curveModelPtr, this]()
        {
          curve->setData(curveModelPtr->keys(), curveModelPtr->values());
          this->replotOnVisibleViewPort(rpQueuedReplot);
        });
      }
    }
    replotOnVisibleViewPort();
    break;
  }
  case GenesisMarkup::TypePeak:
  {
    auto peak = static_cast<ChromatogramPeak*>(layerable);
    peak->setLeftBorder(data->getData(PeakStart).toPointF().x());
    peak->setRightBorder(data->getData(PeakEnd).toPointF().x());
    peak->setLabel(data->getData(PeakTitle).toString());
    peak->setPeakId(data->getData(PeakId).toInt());

    peak->setPeakType((PeakTypes)data->getData(PeakType).toInt());
    peak->updateInteractivityFlags(this, (PeakTypes)data->getData(PeakType).toInt());

    auto color = data->getData(PeakColor).value<QColor>();
    peak->setBrush(color);
    color = color.darker(120);
    peak->setSelectedBrush(color);
    peak->setPen(color);
    peak->setSelectedPen(color.darker(120));

    replotOnVisibleViewPort();
    break;
  }
  case GenesisMarkup::TypeInterval:
  {
    auto interval = static_cast<ChromatogramInterval*>(layerable);
    interval->setLeftBorder(data->getData(IntervalLeft).value<TIntervalLeft>());
    interval->setRightBorder(data->getData(IntervalRight).value<TIntervalRight>());
    interval->setIntervalType((IntervalTypes)data->getData(IntervalType).toInt());
    interval->updateInteractivityFlags(this, (IntervalTypes)data->getData(IntervalType).toInt());
    ///@todo colors
    //        auto color = data->getData(PeakColor).value<QColor>();
    //        interval->setBrush(color);
    //        color = color.darker(120);
    //        interval->setSelectedBrush(color);
    //        interval->setPen(color);
    //        interval->setSelectedPen(color.darker(120));

    replotOnVisibleViewPort();
  }
    break;
  case GenesisMarkup::TypeBaseline:
  {
    auto baseLine = static_cast<ChromatogramBaseline*>(layerable);
    auto dataIter = data->constFind(GenesisMarkup::BaseLineData);
    if(dataIter == data->constEnd() || (!dataIter->isValid() || dataIter->isNull()))
    {
      baseLine->setBaseLineModel(nullptr);
    }
    else
    {
      auto baseLineModelPtr = dataIter->value<GenesisMarkup::TBaseLineDataModel>();
      baseLine->setBaseLineModel(baseLineModelPtr);
    }
    replotOnVisibleViewPort();
    break;
  }
  default:
    break;
  }
}

void ChromatogramPlot::onEntityAdded(EntityType type, TEntityUid uid, AbstractEntityDataModel::ConstDataPtr data)
{
  Q_ASSERT(mModel != nullptr);
  using namespace GenesisMarkup;
  switch ((MarkupEntityTypes)type)
  {
  case TypeCurve:
  {
    if(!data)
      break;
    auto dataIter = data->constFind(GenesisMarkup::CurveData);
    if(dataIter == data->constEnd() || (!dataIter->isValid() || dataIter->isNull()))
    {
      qDebug() << "NO CURVE DATA";
      break;
    }
    GenesisMarkup::CurveDataModelPtr curveModel = dataIter->value<GenesisMarkup::CurveDataModelPtr>();
    if(!curveModel)
    {
      qDebug() << "CURVE DATA POINTER IS NULL";
      break;
    }
    auto curve = addGraph();
    //defaultChromatogramCurvePen.setWidth(2);
    curve->setPen(defaultChromatogramCurvePen);
    curve->setSelectable(GP::stNone);
    mLayerables.add(uid, curve);
    curve->setData(curveModel->keys(), curveModel->values());
    curve->setProperty("modelPtr", QVariant::fromValue(curveModel));
    connect(curveModel.get(), &GenesisMarkup::CurveDataModel::curveDataChanged, curve, [curve, curveModel, this]()
    {
      curve->setData(curveModel->keys(), curveModel->values());
      this->replotOnVisibleViewPort(rpQueuedReplot);
    });
    connect(curve, &QObject::destroyed, &mLayerables, [this, uid]()
    {
      mLayerables.remove(uid);
    });
    replotOnVisibleViewPort(rpQueuedReplot);
    break;
  }
  case TypePeak:
  {
    auto entityPtr = mModel->getEntity(uid);
    if(!entityPtr)
      break;

    auto curveUuid = mModel->getUIdListOfEntities(GenesisMarkup::TypeCurve);
    if(curveUuid.empty() || curveUuid.first().isNull()){
      Q_ASSERT_X(false, "ChromatogramPlot::onModelReset()", "curveUuid.empty() || curveUuid.first().isNull()");
      return;
    }
    if(!mLayerables.contains(curveUuid.first()))
    {
      Q_ASSERT_X(false, "ChromatogramPlot::onModelReset()", "cant create peak, no curve found");
      return;
    }
    auto chromaCurve = static_cast<GPGraph*>(mLayerables.get(curveUuid.first()));

    auto peak = new ChromatogramPeak(this, *chromaCurve,
                                     entityPtr->hasDataAndItsValid(GenesisMarkup::PeakStart) ? entityPtr->getData(GenesisMarkup::PeakStart).toPointF().x() : 0,
                                     entityPtr->hasDataAndItsValid(GenesisMarkup::PeakEnd) ? entityPtr->getData(GenesisMarkup::PeakEnd).toPointF().x() : 0,
                                     uid);
    mLayerables.add(uid, peak);

    if(entityPtr->hasDataAndItsValid(PeakColor))
    {
      auto color = entityPtr->getData(PeakColor).value<QColor>();
      peak->setBrush(color);
      color = color.darker(120);
      peak->setSelectedBrush(color);
      peak->setPen(color);
      peak->setSelectedPen(color.darker(120));
    }

    connect(peak, &QObject::destroyed, &mLayerables, [this, uid]()
    {
      mLayerables.remove(uid);
    });
    mModel->setEntityData(uid, PeakRetentionTime, peak->peakCenter().x());
    PeakTypes type = GenesisMarkup::PTPeak;
    if(entityPtr->hasDataAndItsValid(GenesisMarkup::PeakType))
      type = (PeakTypes)entityPtr->getData(GenesisMarkup::PeakType).toInt();
    else
      type = PeakTypes::PTNone;
    peak->setPeakType(type);
    connect(peak, &ChromatogramPeak::newCommand, this, &ChromatogramPlot::newCommand, Qt::QueuedConnection);
    auto neighbours = mModel->getNeighbourPeaksUids(uid);
    if(!neighbours.first.isNull())
    {
      Q_ASSERT(mLayerables.contains(neighbours.first));
      auto leftPeak = static_cast<ChromatogramPeak*>(mLayerables.get(neighbours.first));
      leftPeak->setRightPeak(peak);
      peak->setLeftPeak(leftPeak);
    }
    if(!neighbours.second.isNull())
    {
      Q_ASSERT(mLayerables.contains(neighbours.second));
      auto rightPeak = static_cast<ChromatogramPeak*>(mLayerables.get(neighbours.second));
      rightPeak->setLeftPeak(peak);
      peak->setRightPeak(rightPeak);
    }
    QUuid firstBaseLineUid;
    auto uids = mModel->getUIdListOfEntities(GenesisMarkup::TypeBaseline);
    if(!uids.isEmpty())
    {
      auto uidIter = uids.begin();
      while(!mModel->getEntity(*uidIter) && uidIter != uids.end())
        uidIter++;
      if(uidIter != uids.end())
      {
        firstBaseLineUid = *uidIter;
      }
    }
    if(!firstBaseLineUid.isNull() && !entityPtr.isNull())
    {
      peak->setBaseLine((ChromatogramBaseline*)mLayerables.get(firstBaseLineUid));
    }
    break;
  }
  case TypeInterval:
  {
    auto entityPtr = mModel->getEntity(uid);
    if(!entityPtr)
      break;
    auto interval = new ChromatogramInterval(this,
                                             entityPtr->hasDataAndItsValid(GenesisMarkup::IntervalLeft) ? entityPtr->getData(GenesisMarkup::IntervalLeft).value<TIntervalLeft>() : 0,
                                             entityPtr->hasDataAndItsValid(GenesisMarkup::IntervalRight) ? entityPtr->getData(GenesisMarkup::IntervalRight).value<TIntervalRight>() : 0,
                                             uid);
    mLayerables.add(uid, interval);
    IntervalTypes type = GenesisMarkup::ITCustomParameters;
    if(entityPtr->hasDataAndItsValid(GenesisMarkup::IntervalType))
      type = (IntervalTypes)entityPtr->getData(GenesisMarkup::IntervalType).toInt();
    else
      type = IntervalTypes::ITNone;

    interval->setIntervalType(type);
    connect(interval, &ChromatogramInterval::newCommand, this, &ChromatogramPlot::newCommand);
    //    auto neighbours = mModel->getNeighbourIntervalsUids(uid);
    auto left = mModel->getLeftIntervalUid(uid);
    if(!left.isNull())
    {
      Q_ASSERT(mLayerables.contains(left));
      auto leftInterval = static_cast<ChromatogramInterval*>(mLayerables.get(left));
      leftInterval->setRightInterval(interval);
      interval->setLeftInterval(leftInterval);
    }
    auto right = mModel->getRightIntervalUid(uid);
    if(!right.isNull())
    {
      Q_ASSERT(mLayerables.contains(right));
      auto rightInterval = static_cast<ChromatogramInterval*>(mLayerables.get(right));
      rightInterval->setLeftInterval(interval);
      interval->setRightInterval(rightInterval);
    }
    replotOnVisibleViewPort(rpQueuedReplot);
  }
    break;
  case TypeBaseline:
  {
    if(!data)
      break;
    auto variantData = data->getData(GenesisMarkup::BaseLineData);
    if((!variantData.isValid() || variantData.isNull()))
    {
      qDebug() << "NO BASELINE DATA";
      break;
    }
    auto baseLineModel = variantData.value<GenesisMarkup::TBaseLineDataModel>();
    if(!baseLineModel)
    {
      qDebug() << "BASELINE DATA POINTER IS NULL";
      break;
    }
    auto baseLine = new ChromatogramBaseline(this, uid);
    baseLine->setBaseLineModel(baseLineModel);
    connect(baseLine, &ChromatogramBaseline::newCommand, this, &ChromatogramPlot::newCommand);
    mLayerables.add(uid, baseLine);
    connect(baseLine, &QObject::destroyed, &mLayerables, [this, uid]()
    {
      mLayerables.remove(uid);
    });
    for(const auto& l : mLayerables.getHash())
      if(auto peak = qobject_cast<ChromatogramPeak*>(l))
        peak->setBaseLine(baseLine);
    break;
  }
  case TypeKeySelector:
  {
    if(!layer("selector_layer"))
    {
      addLayer("selector_layer");
      layer("selector_layer")->setMode(GPLayer::lmBuffered);
    }
    auto curves = mModel->getEntities(TypeCurve);
    Q_ASSERT(!curves.empty());
    auto curveModelPtr = mModel->getEntities(TypeCurve).first()->getData(CurveData).value<TCurveData>();
    auto selector = new ChromatogramKeySelector(this, curveModelPtr, uid);
    selector->setLayer("selector_layer");
    connect(selector, &ChromatogramKeySelector::xCoordChangedByUser, this,
            [this, uid](double xCoord)
            {
              mModel->setEntityData(uid, KeySelectorKey, xCoord);
            });
    mLayerables.add(uid, selector);
    connect(selector, &QObject::destroyed, &mLayerables, [this, uid]()
            {
              mLayerables.remove(uid);
            });
    break;
  }
  default:
    break;
  }
  replotOnVisibleViewPort(rpQueuedReplot);
}

void ChromatogramPlot::onEntityAboutToRemove(EntityType type, TEntityUid uid)
{
  switch (type)
  {
  case GenesisMarkup::TypePeak: {
    auto peak = (ChromatogramPeak*)mLayerables.get(uid);
    if (auto left = peak->getLeftPeak())
      left->setRightPeak(peak->getRightPeak());

    if (auto right = peak->getRightPeak())
      right->setLeftPeak(peak->getLeftPeak());
    break;
  }
  case GenesisMarkup::TypeInterval: {
    auto interval = (ChromatogramInterval*)mLayerables.get(uid);
    if (auto left = interval->getLeftInterval())
      left->setRightInterval(interval->getRightInterval());

    if (auto right = interval->getRightInterval())
      right->setLeftInterval(interval->getLeftInterval());
    break;
  }
  case GenesisMarkup::TypeBaseline: {
    for (const auto& l : mLayerables.getHash())
      if (auto peak = qobject_cast<ChromatogramPeak*>(l))
        peak->setBaseLine(nullptr);
    removeCurve((GPCurve*)mLayerables.get(uid));
    break;
  }
  default:
    break;
  }

  mLayerables.removeAndDelete(uid);
  replotOnVisibleViewPort(rpQueuedReplot);
}

void ChromatogramPlot::onMarkupDataChanged(uint role, const QVariant& data)
{
  if (!mModel)
    return;
  if (role == MarkupMasterIdRole)
  {
    for (auto& l : mLayerables.getHash())
      if (auto peak = dynamic_cast<ChromatogramPeak*>(l))
        peak->updateInteractivityFlags(this);
    handleStepInteractions();
  }
}

void ChromatogramPlot::onModelAboutToReset()
{
  clearLayerables();
}

void ChromatogramPlot::onModelReset()
{
  if (mModel)
  {
    //    qDebug().noquote() << QJsonDocument(mModel->print({},
    //    {GenesisMarkup::TypePeak})).toJson();
    ////CURVE
    QUuid firstCurveModelUid;
    for (auto& uid : mModel->getUIdListOfEntities(GenesisMarkup::TypeCurve))
    {
      auto entityPtr = mModel->getEntity(uid);
      if (!entityPtr)
        continue;
      if (firstCurveModelUid.isNull())
        firstCurveModelUid = uid;
      auto dataIter = entityPtr->constFind(GenesisMarkup::CurveData);
      if (dataIter == entityPtr->constEnd())
      {
        qDebug() << "NO CURVE DATA";
        continue;
      }
      GenesisMarkup::CurveDataModelPtr curveModel = dataIter->value<GenesisMarkup::CurveDataModelPtr>();

      if (!curveModel)
      {
        qDebug() << "CURVE DATA POINTER IS NULL";
        continue;
      }
      //      auto curve = addCurve();

      auto curve = addGraph();
      curve->setPen(defaultChromatogramCurvePen);
      curve->setSelectable(GP::SelectionType::stNone);
      mLayerables.add(uid, curve);
      connect(curve, &GPGraph::destroyed, &mLayerables, [this, uid]()
      {
        mLayerables.remove(uid);
      });
      curve->setData(curveModel->keys(), curveModel->values());
      curve->setProperty("modelPtr", QVariant::fromValue(curveModel));
      connect(curveModel.get(), &GenesisMarkup::CurveDataModel::curveDataChanged, curve, [curve, curveModel, this]() {
        curve->setData(curveModel->keys(), curveModel->values());
        this->replotOnVisibleViewPort(rpQueuedReplot);
      });
    }
    ////END CURVE
    ////BASELINE
    QUuid firstBaseLineUid;
    {
      auto uids = mModel->getUIdListOfEntities(GenesisMarkup::TypeBaseline);
      if (!uids.isEmpty())
      {
        auto uidIter = uids.begin();
        while (!mModel->getEntity(*uidIter) && uidIter != uids.end())
          uidIter++;
        if (uidIter != uids.end())
        {
          firstBaseLineUid = *uidIter;
        }
      }
      if (!firstBaseLineUid.isNull())
      {
        auto entityPtr = mModel->getEntity(firstBaseLineUid);  // should be not null anyway
        TBaseLineDataModel baseLineModel =
            entityPtr->constFind(BaseLineData) == entityPtr->constEnd() ? nullptr : entityPtr->constFind(BaseLineData).value().value<TBaseLineDataModel>();
        if (baseLineModel)
        {
          auto baseLine = new ChromatogramBaseline(this, firstBaseLineUid);
          baseLine->setBaseLineModel(baseLineModel);
          mLayerables.add(firstBaseLineUid, baseLine);
          connect(baseLine, &ChromatogramBaseline::newCommand, this, &ChromatogramPlot::newCommand);
          connect(baseLine, &GPCurve::destroyed, &mLayerables, [this, firstBaseLineUid]()
          {
            mLayerables.remove(firstBaseLineUid);
          });
        }
      }
    }
    ////END BASELINE
    ////PEAKS
    {
      QList<QUuid> uids = mModel->getUIdListOfEntities(GenesisMarkup::TypePeak);
      std::sort(uids.begin(), uids.end(), [this](const QUuid& a, const QUuid& b) -> bool {
        auto aData = mModel->getEntity(a);
        auto bData = mModel->getEntity(b);
        auto aIter = aData->constFind(GenesisMarkup::PeakStart);
        auto bIter = bData->constFind(GenesisMarkup::PeakStart);
        if (aIter == aData->constEnd() || !aIter.value().isValid())
        {
          return false;  // a >= b
        }
        else
        {
          if (bIter == bData->constEnd() || !bIter.value().isValid())
            return true;  // a < b
          else
          {
            return aIter.value().toPointF().x() < bIter.value().toPointF().x();
          }
        }
      });
      ChromatogramPeak* lastPeak = nullptr;
      for (int i = 0; i < uids.size(); i++)  // uids - is peaks uids sorted by left border
      {
        auto& uid = uids[i];
        auto entityPtr = mModel->getEntity(uid);
        if (!entityPtr)
          continue;

        Q_ASSERT_X(!firstCurveModelUid.isNull(), "ChromatogramPlot::onModelReset()", "lastCurveModelUid is NULL");
        Q_ASSERT_X(mLayerables.contains(firstCurveModelUid), "ChromatogramPlot::onModelReset()", "No curve in mLayerables");
        auto chromaCurve = static_cast<GPGraph*>(mLayerables.get(firstCurveModelUid));

        auto peak = new ChromatogramPeak(this, *chromaCurve, entityPtr->getData(GenesisMarkup::PeakStart).toPointF().x(),
                                         entityPtr->getData(GenesisMarkup::PeakEnd).toPointF().x(), uid);
        if (!firstBaseLineUid.isNull())
          peak->setBaseLine((ChromatogramBaseline*)mLayerables.get(firstBaseLineUid));

        PeakTypes type = GenesisMarkup::PTPeak;
        if (entityPtr->constFind(GenesisMarkup::PeakType) != entityPtr->constEnd())
          type = (PeakTypes)entityPtr->constFind(GenesisMarkup::PeakType).value().toInt();
        peak->setPeakType(type);

        auto color = entityPtr->getData(PeakColor).value<QColor>();
        peak->setBrush(color);
        color = color.darker(120);
        peak->setPen(QPen(color));
        peak->setSelectedBrush(color);
        color = color.darker(120);
        peak->setSelectedPen(QPen(color));
        connect(peak, &ChromatogramPeak::newCommand, this, &ChromatogramPlot::newCommand, Qt::QueuedConnection);
        peak->setLabel(entityPtr->getData(PeakTitle).toString());  // debug purpose
        mLayerables.add(uid, peak);
        if (lastPeak)
        {
          lastPeak->setRightPeak(peak);
          peak->setLeftPeak(lastPeak);
        }
        lastPeak = peak;
        connect(peak, &GPLayerable::destroyed, &mLayerables, [this, uid]()
        {
          mLayerables.remove(uid);
        });
      }
    }
    ////END PEAKS
    /// INTERVALS
    {
      QList<QUuid> uids = mModel->getUIdListOfEntities(GenesisMarkup::TypeInterval);
      std::sort(uids.begin(), uids.end(), [this](const QUuid& a, const QUuid& b) -> bool {
        auto aData = mModel->getEntity(a);
        auto bData = mModel->getEntity(b);
        auto aIter = aData->constFind(GenesisMarkup::IntervalLeft);
        auto bIter = bData->constFind(GenesisMarkup::IntervalLeft);
        if (aIter == aData->constEnd() || !aIter.value().isValid())
        {
          return false;  // a >= b
        }
        else
        {
          if (bIter == bData->constEnd() || !bIter.value().isValid())
            return true;  // a < b
          else
          {
            return aIter.value().value<TIntervalLeft>() < bIter.value().value<TIntervalLeft>();
          }
        }
      });

      ChromatogramInterval* lastInterval = nullptr;
      for (int i = 0; i < uids.size(); i++)  // uids - is peaks uids sorted by left border
      {
        auto& uid = uids[i];
        auto entityPtr = mModel->getEntity(uid);
        if (!entityPtr)
          continue;

        auto interval = new ChromatogramInterval(this, entityPtr->getData(GenesisMarkup::IntervalLeft).value<TIntervalLeft>(),
                                                 entityPtr->getData(GenesisMarkup::IntervalRight).value<TIntervalRight>(), uid);

        IntervalTypes type = GenesisMarkup::ITCustomParameters;
        if(entityPtr->hasDataAndItsValid(GenesisMarkup::IntervalType))
          type = (IntervalTypes)entityPtr->getData(GenesisMarkup::IntervalType).toInt();
        else
          type = IntervalTypes::ITNone;

        interval->setIntervalType(type);

        connect(interval, &ChromatogramInterval::newCommand, this, &ChromatogramPlot::newCommand);

        ///@todo colors
        auto color = entityPtr->getData(PeakColor).value<QColor>();
        //        interval->setBrush(color);
        //        color = color.darker(120);
        //        interval->setPen(QPen(color));
        //        interval->setSelectedBrush(color);
        //        color = color.darker(120);
        //        interval->setSelectedPen(QPen(color));
        mLayerables.add(uid, interval);
        if (lastInterval)
        {
          lastInterval->setRightInterval(interval);
          interval->setLeftInterval(lastInterval);
        }
        lastInterval = interval;
      }
    }
    /// END INTERVALS
  }

  auto boundings = boundingRange();
  qDebug() << boundings;
  mXDataRange = boundings.first;
  mYDataRange = boundings.second;
  replotOnVisibleViewPort(rpQueuedReplot);
}

void ChromatogramPlot::clearLayerables()
{
  clearCurves();
  clearGraphs();
  clearPlottables();
  clearItems();
  mLayerables.clearAndDelete();
}


void ChromatogramPlot::setupUiSettings()
{
  //  Plot->setPlottingHint(GP::phFastPolylines, true);
  //  Plot->setPlottingHint(GP::phFastPolylines, false);
  setOpenGl(true, 8);
  setAntialiasedElements(GP::aeAll);
  GraphicsPlot::setNoAntialiasingOnDrag(true);
}

void ChromatogramPlot::ensureAllCurvesVisible()
{
  auto boundings = boundingRange();
  mXDataRange = boundings.first;
  mYDataRange = boundings.second;
  xAxis->setRange(mXDataRange);
  yAxis->setRange(mYDataRange);
}

void ChromatogramPlot::connectModelSignals(GenesisMarkup::ChromatogrammModelPtr model)
{
  if (!model)
    return;

  connect(mModel.get(), &GenesisMarkup::ChromatogramDataModel::modelAboutToReset, this, &ChromatogramPlot::onModelAboutToReset);
  connect(mModel.get(), &GenesisMarkup::ChromatogramDataModel::modelReset, this, &ChromatogramPlot::onModelReset);
  connect(mModel.get(), &GenesisMarkup::ChromatogramDataModel::entityChanged, this, &ChromatogramPlot::onEntityChanged);
  connect(mModel.get(), &GenesisMarkup::ChromatogramDataModel::entityResetted, this, &ChromatogramPlot::onEntityResetted);
  connect(mModel.get(), &GenesisMarkup::ChromatogramDataModel::entityAdded, this, &ChromatogramPlot::onEntityAdded);
  connect(mModel.get(), &GenesisMarkup::ChromatogramDataModel::entityAboutToRemove, this, &ChromatogramPlot::onEntityAboutToRemove);
  if (auto model = mModel->markupModel())
  {
    connect(model, &MarkupDataModel::dataChanged, this, &ChromatogramPlot::onMarkupDataChanged);
  }
}

void ChromatogramPlot::customUpdate(RefreshPriority rp)
{
  //  qDebug() << this << " customUpdate IN";
  if (mReplotting) // incase signals loop back to replot slot
    return;
  mReplotting = true;
  mReplotQueued = false;
  emit beforeReplot();

  updateLayout();
  // draw all layered objects (grid, axes, plottables, items, legend,...) into their buffers:
  setupPaintBuffers();
  for (const auto& layer : mLayers)
    layer->drawToPaintBuffer();
  for (int i = 0; i < mPaintBuffers.size(); ++i)
    mPaintBuffers.at(i)->setInvalidated(false);

  if ((rp == rpRefreshHint && mPlottingHints.testFlag(GP::phImmediateRefresh)) || rp==rpImmediateRefresh)
    repaint();
  else
    update();

  emit afterReplot();
  mReplotting = false;

  //  qDebug() << this << " customUpdate OUT";
}

const GPRange& ChromatogramPlot::yDataRange() const
{
  return mYDataRange;
}

void ChromatogramPlot::replotOnVisibleViewPort(RefreshPriority rp)
{
  if (visibleRegion().isEmpty()){
    return;
  }
  if (!mReplotQueued){
    mReplotQueued = true;
    QTimer::singleShot(0, this, SLOT(customUpdate()));
  }
}

const GPRange& ChromatogramPlot::xDataRange() const
{
  return mXDataRange;
}

Qt::CursorShape ChromatogramPlot::GetCursorAt(const QPointF& pos)
{
  QMouseEvent ev(QEvent::MouseMove, pos, Qt::NoButton, qApp->mouseButtons(), qApp->queryKeyboardModifiers());
  QList<GPLayerable*> candidates = layerableListAt(pos, false);
  auto newCursor = Qt::ArrowCursor;
  unsigned int part;
  for (const auto& candidate : candidates)
  {
    newCursor = candidate->HitTest(&ev, &part);
    if (newCursor != Qt::ArrowCursor)
      break;
  }
  return newCursor;
}

GenesisMarkup::ChromatogrammModelPtr ChromatogramPlot::model() const
{
  return mModel;
}

bool ChromatogramPlot::setBLSelectionMode(bool enabled)
{
  if(baseLineInteractionsMode == enabled)
    return false;
  bool blactionsAllowed = false;
  if(isMaster())
  {
    if(mStepInteractions.masterInteractions.testFlag(StepMasterInteractions::SIMAddRemoveBaseLinePoint))
      blactionsAllowed = true;
  }
  else
  {
    if(mStepInteractions.slaveInteractions.testFlag(StepSlaveInteractions::SISAddRemoveBaseLinePoint))
      blactionsAllowed = true;
  }
  if(!blactionsAllowed)
    return false;

  baseLineInteractionsMode = enabled;
  auto setBLSelection = [this](GP::SelectionType selectionType)
  {
    QUuid firstBaseLineUid;
    auto uids = mModel->getUIdListOfEntities(GenesisMarkup::TypeBaseline);
    if(!uids.isEmpty())
    {
      auto uidIter = uids.begin();
      while(!mModel->getEntity(*uidIter) && uidIter != uids.end())
        uidIter++;
      if(uidIter != uids.end())
      {
        firstBaseLineUid = *uidIter;
      }
    }
    if(!firstBaseLineUid.isNull())
    {
      auto blItem = (ChromatogramBaseline*)mLayerables.get(firstBaseLineUid);
      if(blItem)
      {
        blItem->setSelectable(selectionType);
        blItem->setSelection(GPDataSelection());
      }
    }
  };

  if(!enabled)
  {
    setSelectionRectMode(GP::srmNone);

    if(blactionsAllowed)
      setBLSelection(GP::stSingleData);
  }
  else
  {
    if(blactionsAllowed)
      setBLSelection(GP::stMultipleDataRanges);
  }
  return true;
}

const GenesisMarkup::StepInfo& ChromatogramPlot::stepInteractions() const
{
  return mStepInteractions;
}

bool ChromatogramPlot::isMaster() const
{
  if (!mModel)
    return false;
  return mModel->isMaster();
}

void ChromatogramPlot::mouseMoveEvent(QMouseEvent* event)
{
  // set new cursor
  if (event->buttons().testFlag(Qt::NoButton))
  {
    // make hit tests only when have any selected peaks
    // because we have not any cursor shape logic without selected peaks yet
    // TODO: change it if cursor shape logic will change
    // SelectorItem alvays selected
    if (mLayerables.hasSelected())
      setCursor(GetCursorAt(event->pos()));
  }
  else
  {
    //FIXME: is this code still does anything?
    if (event->buttons().testFlag(Qt::LeftButton) && DraggingObject)
    {
      DraggingObject->DragMove(event);
      event->accept();
      replotOnVisibleViewPort(rpQueuedReplot);
      return;
    }
    if(!mTemporaryPeakUid.isNull())
    {
      if(auto peak = qobject_cast<ChromatogramPeak*>(mLayerables.get(mTemporaryPeakUid)))
        peak->setRightBorder(xAxis->pixelToCoord(event->pos().x()));
    }
  }
  return GraphicsPlot::mouseMoveEvent(event);
}

void ChromatogramPlot::contextMenuEvent(QContextMenuEvent* event)
{
  if(contextMenuLocked)
    return;
  // Global actions
  if (contextMenu)
    delete contextMenu;

  contextMenu = new QMenu(this);
  std::map<QAction*, std::function<void(void)>> actions;

  FillContextMenu(event, *contextMenu, actions);

  if (!actions.empty())
  {
    connect(contextMenu, &QMenu::triggered, [actions](QAction* action) {
      auto a = actions.find(action);
      if (a != actions.end())
      {
        a->second();
      }
    });

    // Show
    contextMenu->popup(event->globalPos());
    event->accept();

    // Done

    return;
  }
  GraphicsPlot::contextMenuEvent(event);
}

void ChromatogramPlot::FillContextMenu(QContextMenuEvent* event, QMenu& menu, std::map<QAction*, std::function<void()>>& actions)
{
  auto getMinMax = [this](QUuid left_peak_id, QUuid right_peak_id) {
    QPair<int, int> minmax = { -1, -1 };
    int left_marker = 0;
    int right_marker = 0;
    if (mLayerables.contains(left_peak_id))
    {
      auto left_peak = static_cast<ChromatogramPeak*>(mLayerables.get(left_peak_id));
      auto left_peak_entity = mModel->getEntity(left_peak_id);
      if (left_peak && left_peak_entity)
      {
        switch ((GenesisMarkup::PeakTypes)left_peak_entity->value(PeakType).toInt())
        {
        case PTFake:
        case PTNone:
        case PTPeak: {
          auto prev_peak = left_peak->getLeftPeak();
          while (prev_peak)
          {
            auto prev_peak_entity = mModel->getEntity(prev_peak->uid());
            if (prev_peak_entity)
            {
              switch ((GenesisMarkup::PeakTypes)prev_peak_entity->getData(PeakType).toInt())
              {
              case PTNone:
              case PTPeak:
              case PTFake:
                break;
              case PTMarker: {
                left_marker = prev_peak_entity->getData(PeakId).toInt();
                prev_peak = nullptr;
              }
                break;
              case PTInterMarker: {
                minmax.first = prev_peak_entity->getData(PeakId).toInt();
                prev_peak = nullptr;
              }
                break;
              }
            }
            if(prev_peak)
            {
              prev_peak = prev_peak->getLeftPeak();
            }
          }
        }
          break;
        case PTMarker:
          left_marker = left_peak_entity->getData(PeakId).toInt();
          break;
        case PTInterMarker:
          minmax.first = left_peak_entity->getData(PeakId).toInt();
          break;
        }
      }
    }

    auto list = mModel->markupModel()->getValuablePeaksSortedIdList();
    if (list.isEmpty())
    {
      mModel->markupModel()->updateValuablePeaksSortedIdList();
      list = mModel->markupModel()->getValuablePeaksSortedIdList();
    }

    if (left_marker < 0)
    {
      if (!list.isEmpty())
      {
        bool range = false;
        foreach (int id, list)
        {
          if (range)
          {
            if (id < 0)
            {
              break;
            }
            else
            {
              if (minmax.first < 0)
              {
                minmax.first = id - 1;
                minmax.second = id + 1;
              }
              else
              {
                minmax.second = id + 1;
              }
            }
          }
          if (id == left_marker)
          {
            range = true;
          }
        }
      }
      else
      {
        minmax.first = -1;
        minmax.second = -1;
        return minmax;
      }
    }
    if (minmax.first <= 0)
    {
      return minmax;
    }

    if (mLayerables.contains(right_peak_id))
    {
      auto right_peak = static_cast<ChromatogramPeak*>(mLayerables.get(right_peak_id));
      auto right_peak_entity = mModel->getEntity(right_peak_id);
      if (right_peak && right_peak_entity)
      {
        switch ((GenesisMarkup::PeakTypes)right_peak_entity->value(PeakType).toInt())
        {
        case PTFake:
        case PTNone:
        case PTPeak: {
          auto next_peak = right_peak->getRightPeak();
          while (next_peak)
          {
            auto next_peak_entity = mModel->getEntity(next_peak->uid());
            if (next_peak_entity)
            {
              switch ((GenesisMarkup::PeakTypes)next_peak_entity->getData(PeakType).toInt())
              {
              case PTNone:
              case PTPeak:
              case PTFake:
                break;
              case PTMarker: {
                minmax.second = next_peak_entity->getData(PeakId).toInt();
                next_peak = nullptr;
              }
                break;
              case PTInterMarker: {
                minmax.second = next_peak_entity->getData(PeakId).toInt();
                next_peak = nullptr;
              }
                break;
              }
            }
            if(next_peak)
            {
              next_peak = next_peak->getRightPeak();
            }
          }
        }
          break;
        case PTMarker:
          minmax.second = right_peak_entity->getData(PeakId).toInt();
          break;
        case PTInterMarker:
          minmax.second = right_peak_entity->getData(PeakId).toInt();
          break;
        }
      }
    }
    if(minmax.second < 0)
    {
      if (!list.isEmpty())
      {
        for (int i = 0; i < list.size(); i++)
        {
          int id = list[i];
          if (id == minmax.second)
          {
            if(list.at(i))
              minmax.second = list[i-1] + 1; // +1 to get last id in range
            break;
          }
        }
      }
      else
      {
        minmax.first = -1;
        minmax.second = -1;
        return minmax;
      }
    }
    return minmax;
  };

  bool master = isMaster();
  bool isBiomarkers = Core::GenesisContextRoot::Get()->GetContextModules()->GetData(Names::ModulesContextTags::kModule).toInt() == ModulesContextTags::MBiomarkers;
  mMode = (GenesisMarkup::ChromatogrammModes)mModel->getChromatogramValue(ChromatogrammMode).toInt();
  if(mMode == GenesisMarkup::CMMarkup)
  {
    QList<QUuid> uids = mModel->getUIdListOfEntities(GenesisMarkup::TypePeak);
    std::sort(uids.begin(), uids.end(), [this](const QUuid& a, const QUuid& b) -> bool {
      auto aData = mModel->getEntity(a);
      auto bData = mModel->getEntity(b);
      auto aIter = aData->constFind(GenesisMarkup::PeakStart);
      auto bIter = bData->constFind(GenesisMarkup::PeakStart);
      if (aIter == aData->constEnd() || !aIter.value().isValid())
      {
        return false;  // a >= b
      }
      else
      {
        if (bIter == bData->constEnd() || !bIter.value().isValid())
          return true;  // a < b
        else
        {
          return aIter.value().toPointF().x() < bIter.value().toPointF().x();
        }
      }
    });

    AbstractEntityDataModel::ConstDataPtr entity = nullptr;
    QUuid left_peak_id;
    QUuid right_peak_id;
    QUuid entity_id;
    PeakTypes peak_type = GenesisMarkup::PTNone;
    double x_key = xAxis->pixelToCoord(event->pos().x());
    double x_left = x_key - 3;
    double x_right = x_key + 3;
    bool find_left = false;
    bool find_right = false;

    for (int i = 0; i < uids.size(); i++)
    {
      auto& uid = uids[i];
      auto entityPtr = mModel->getEntity(uid);
      if (!entityPtr)
        continue;
      else
      {
        auto x_start = entityPtr->value(GenesisMarkup::PeakStart).toPointF().x();
        auto x_end = entityPtr->value(GenesisMarkup::PeakEnd).toPointF().x();

        if (!entity && x_start <= x_key && x_key <= x_end)
        {
          entity = entityPtr;
          entity_id = uids[i];
          peak_type = (GenesisMarkup::PeakTypes)entityPtr->getData(PeakType).toInt();
        }
        else
        {
          if (x_end <= x_key)
          {
            if(x_left < x_end)
            {
              x_left = x_end;
            }
            left_peak_id = uids[i];
          }

          if (x_start >= x_key)
          {
            if(x_right > x_start)
            {
              x_right = x_start;
            }
            right_peak_id = uids[i];
            break;
          }
        }
      }
    }

    double peakHeight;
    double x_peak_maximum = mModel->getEntities(TypeCurve).first()->getData(CurveData).value<TCurveData>()->maxValueXCoord(x_left, x_right, &peakHeight);

    // MARKUP MODE
    bool peakInteractionsAllowed = true;
    for(auto& l : layerableListAt(event->pos(), false))
    {
      if(auto intervalItem = qobject_cast<ChromatogramInterval*>(l))
      {
        if(intervalItem->intervalType() == GenesisMarkup::ITMarkupDeprecation)
        {
          peakInteractionsAllowed = false;
          break;
        }
      }
    }
    {//clicked pick highlighting
      bool isPeakAlredySelected = false;
      auto peak = qobject_cast<ChromatogramPeak*>(mLayerables.get(entity_id));
      if(peak)
      {
        isPeakAlredySelected = peak->getIsSelected();
        if(!isPeakAlredySelected)
        {
          peak->setIsSelected(true);
          replot(GraphicsPlot::rpQueuedReplot);
        }
      }
      connect(&menu, &QMenu::aboutToHide, this,
              [isPeakAlredySelected, peak, this]()
              {
                if(peak && !isPeakAlredySelected)
                {
                  peak->setIsSelected(false);
                  replot(GraphicsPlot::rpQueuedReplot);
                }
              });
    }
    if (master)
      /// MASTER PLOT MENU
    {
      if (peakInteractionsAllowed && mStepInteractions.masterInteractions.testFlag(SIMAddRemovePeak))
      {
        if (!entity)
        {
          DataModel data;
          data[PeakType] = PTPeak;
          data[PeakStart] = QPointF(x_left, 0);
          data[PeakEnd] = QPointF(x_right, 0);
          data[PeakRetentionTime] = x_peak_maximum;
          data[PeakHeight] = peakHeight;
          QAction* imageActions = menu.addAction(tr("AddPeak"));
          actions[imageActions] = [this, data]() {
            auto cmd = new AddEntityCommand(mModel, data, TypePeak);
            emit newCommand(cmd);
          };
        }
        else
        {
          if (peak_type == PTPeak)
          {
            QAction* action = menu.addAction(tr("RemovePeak"));
            actions[action] = [this, entity_id]() {
              auto cmd = new RemoveEntityCommand(mModel, entity_id);
              emit newCommand(cmd);
            };
          }
        }
      }


      if (peakInteractionsAllowed && mStepInteractions.masterInteractions.testFlag(SIMAddRemoveMarker))
      {
        if (!entity)
        {
          DataModel data;
          data[PeakType] = PTMarker;
          data[PeakStart] = QPointF(x_left, 0);
          data[PeakEnd] = QPointF(x_right, 0);
          data[PeakRetentionTime] = x_peak_maximum;
          data[PeakHeight] = peakHeight;
          auto actionText = isBiomarkers ? tr("AddMarkedPeak") : tr("AddMarker");
          QAction* imageActions = menu.addAction(actionText);
          actions[imageActions] = [this, data]() {
            auto cmd = new AddEntityCommand(mModel, data, TypePeak);
            emit newCommand(cmd);
          };
        }
        else
        {
          switch (PeakTypes(peak_type))
          {
          case PTNone:
          case PTFake:
          case PTInterMarker:
            break;
          case PTPeak:
          {
            if (mStepInteractions.masterInteractions.testFlag(GenesisMarkup::SIMSetUnsetMarker))
            {
              auto actionText = isBiomarkers ? tr("SetPeak") : tr("SetMarker");
              QAction* setMarkerActions = menu.addAction(actionText);
              actions[setMarkerActions] = [this, entity_id]()
              {
                auto cmd = new ChangeEntityDataCommand(mModel, entity_id, PeakType, PTMarker);

                emit newCommand(cmd);
              };
            }
            break;
          }
          case PTMarker:
          {
            if (mStepInteractions.masterInteractions.testFlag(GenesisMarkup::SIMSetUnsetMarker))
            {
              auto actionText = isBiomarkers ? tr("UnSetPeak") : tr("UnSetMarker");
              QAction* unSetMarkerActions = menu.addAction(actionText);
              actions[unSetMarkerActions] = [this, entity_id]()
              {
                auto cmd = new ChangeEntityDataCommand(mModel, entity_id, PeakType, PTPeak);
                emit newCommand(cmd);
              };
            }
            QAction* removeActions = menu.addAction(tr("RemoveMarker"));
            actions[removeActions] = [this, entity_id]()
            {
              auto cmd = new RemoveEntityCommand(mModel, entity_id);
              emit newCommand(cmd);
            };
            break;
          }
          }
        }
      }


      if (peakInteractionsAllowed && mStepInteractions.masterInteractions.testFlag(SIMSetUnsetInterMarker))
      {
        menu.addSeparator();
        if (!entity)
        {
          DataModel data;
          data[PeakType] = PTInterMarker;
          data[PeakStart] = QPointF(x_left, 0);
          data[PeakEnd] = QPointF(x_right, 0);
          data[PeakRetentionTime] = x_peak_maximum;
          data[PeakHeight] = peakHeight;
          QAction* imageActions = menu.addAction(tr("AddInterMarker"));
          actions[imageActions] = [this, data]() {
            auto cmd = new AddEntityCommand(mModel, data, TypePeak);
            emit newCommand(cmd);
          };
        }
        else
        {
          switch (PeakTypes(peak_type))
          {
          case PTNone:
          case PTFake:
          case PTMarker:
            break;
          case PTInterMarker:
          {
            if (mStepInteractions.masterInteractions.testFlag(GenesisMarkup::SIMSetUnsetInterMarker))
            {
              QAction* setInterMarkerActions = menu.addAction(tr("UnSetInterMarker"));
              actions[setInterMarkerActions] = [this, entity_id]()
              {
                auto cmd = new ChangeEntityDataCommand(mModel, entity_id, PeakType, PTPeak);
                emit newCommand(cmd);
              };
            }

            QAction* removeActions = menu.addAction(tr("RemoveInterMarker"));
            actions[removeActions] = [this, entity_id]()
            {
              auto cmd = new RemoveEntityCommand(mModel, entity_id);
              emit newCommand(cmd);
            };
            break;
          }
          case PTPeak:

            if (mStepInteractions.masterInteractions.testFlag(GenesisMarkup::SIMSetUnsetInterMarker))
            {
              QAction* unSetMarkerActions = menu.addAction(tr("SetInterMarker"));
              actions[unSetMarkerActions] = [this, entity_id]()
              {
                auto cmd = new ChangeEntityDataCommand(mModel, entity_id, PeakType, PTInterMarker);
                emit newCommand(cmd);
              };
            }
            break;
          }
        }
      }

      if (mStepInteractions.masterInteractions.testFlag(SIMAddRemoveInterval))
      {

        QAction* imageActions = menu.addAction(tr("Set Intervals"));
        actions[imageActions] = [this]() {
          auto cmd = new SetChromatogramDataCommand(mModel, ChromatogrammMode, CMIntervals);
          emit newCommand(cmd);
        };
      }

      if (mStepInteractions.masterInteractions.testFlag(SIMAddRemoveBaseLinePoint))
      {
        QList<QVariant> selectionDetails;
        QPoint pixelPos = event->pos();
        auto layerables = layerableListAt(pixelPos, false, &selectionDetails);
        for(auto& l : layerables)
        {
          if(auto baseLine = qobject_cast<ChromatogramBaseline*>(l))
          {
            auto closestPair = baseLine->closestPoint(pixelPos);
            if(closestPair.first < (double)mSelectionTolerance * 0.99)
            {
              menu.addSeparator();
              QAction* imageActions = menu.addAction(tr("RemoveBaseLinePoint"));
              actions[imageActions] = [ baseLine, closestPair]()
              {
                baseLine->removePointFromBaseLine(closestPair.second->key);
              };
              break;
            }
            else
            {
              menu.addSeparator();
              QAction* imageActions = menu.addAction(tr("AddBaseLinePoint"));
              actions[imageActions] = [pixelPos, baseLine]()
              {
                auto pos = baseLine->pixelsToCoords(pixelPos);
                baseLine->addPointToBaseLine(pos.x(), pos.y());
              };
            }
          }
        }
      }
    }
    /// END MASTER PLOT MENU
    else
      /// SLAVE PLOT MENU
    {
      if (peakInteractionsAllowed && mStepInteractions.slaveInteractions.testFlag(SISAddRemovePeak))
      {
        if (!entity)
        {
          DataModel data;
          data[PeakType] = PTPeak;
          data[PeakStart] = QPointF(x_left, 0);
          data[PeakEnd] = QPointF(x_right, 0);
          data[PeakRetentionTime] = x_peak_maximum;
          data[PeakHeight] = peakHeight;
          QAction* imageActions = menu.addAction(tr("AddPeak"));
          actions[imageActions] = [this, data]()
          {
            auto cmd = new AddEntityCommand(mModel, data, TypePeak);
            emit newCommand(cmd);
          };
        }
        else
        {
          if (peak_type == PTPeak)
          {
            QAction* action = menu.addAction(tr("RemovePeak"));
            actions[action] = [this, entity_id]()
            {
              auto cmd = new RemoveEntityCommand(mModel, entity_id);
              emit newCommand(cmd);
            };
          }
        }
      }

      if (mStepInteractions.slaveInteractions.testFlag(SISAddRemoveInterval))
      {

        QAction* imageActions = menu.addAction(tr("Set Intervals"));
        actions[imageActions] = [this]() {
          auto cmd = new SetChromatogramDataCommand(mModel, ChromatogrammMode, CMIntervals);
          emit newCommand(cmd);
        };
      }

      if (peakInteractionsAllowed && mStepInteractions.slaveInteractions.testFlag(SISAddRemoveMarker))
      {
        menu.addSeparator();
        if (!entity)
        {
          DataModel data;
          data[PeakType] = PTMarker;
          data[PeakStart] = QPointF(x_left, 0);
          data[PeakEnd] = QPointF(x_right, 0);
          data[PeakRetentionTime] = x_peak_maximum;
          data[PeakHeight] = peakHeight;
          auto actionText = isBiomarkers ? tr("AddMarkedPeak") : tr("AddMarker");
          QAction* imageActions = menu.addAction(actionText);
          actions[imageActions] = [this, data]()
          {
            auto cmd = new AddEntityCommand(mModel, data, TypePeak);
            emit newCommand(cmd);
          };
        }
        else
        {
          switch (PeakTypes(peak_type))
          {
          case PTNone:
          case PTFake:
          case PTInterMarker:
            break;
          case PTPeak:
            if (mStepInteractions.slaveInteractions.testFlag(GenesisMarkup::SISSetUnsetMarker))
            {
              auto actionText = isBiomarkers ? tr("SetPeak") : tr("SetMarker");
              QAction* setMarkerActions = menu.addAction(actionText);
              actions[setMarkerActions] = [this, entity_id]()
              {
                auto cmd = new ChangeEntityDataCommand(mModel, entity_id, PeakType, PTMarker);
                emit newCommand(cmd);
              };
            }
            break;

          case PTMarker:

            if (mStepInteractions.slaveInteractions.testFlag(GenesisMarkup::SISSetUnsetMarker))
            {
              QAction* unSetMarkerActions = menu.addAction(tr("UnSetMarker"));
              actions[unSetMarkerActions] = [this, entity_id]()
              {
                auto cmd = new ChangeEntityDataCommand(mModel, entity_id, PeakType, PTPeak);
                emit newCommand(cmd);
              };
            }

            QAction* removeActions = menu.addAction(tr("RemoveMarker"));
            actions[removeActions] = [this, entity_id]()
            {
              auto cmd = new RemoveEntityCommand(mModel, entity_id);
              emit newCommand(cmd);
            };
            break;
          }
        }
      }


      if (peakInteractionsAllowed && mStepInteractions.slaveInteractions.testFlag(GenesisMarkup::SISAddRemoveInterMarker))
      {
        menu.addSeparator();
        if (!entity)
        {
          DataModel data;
          data[PeakType] = PTInterMarker;
          data[PeakStart] = QPointF(x_left, 0);
          data[PeakEnd] = QPointF(x_right, 0);
          data[PeakRetentionTime] = x_key;
          data[PeakHeight] = peakHeight;
          QAction* addPeakAsInterMarker = nullptr;
          SpinBoxAction* sbaction = nullptr;
          addPeakAsInterMarker = sbaction = new SpinBoxAction(tr("Add inter marker"));
          auto availableRange = getMinMax(left_peak_id, right_peak_id);

          if (availableRange.first != -1 && availableRange.second != -1 && (availableRange.second - availableRange.first > 1))
          {
            menu.addAction(addPeakAsInterMarker);
            sbaction->SpinBox->setMinimum(availableRange.first + 1);
            sbaction->SpinBox->setMaximum(availableRange.second - 1);
            actions[addPeakAsInterMarker] = [this, x_left, x_right, x_peak_maximum, sbaction, peakHeight]() {
              DataModel data;
              data[PeakType] = PTInterMarker;
              data[PeakStart] = QPointF(x_left, 0);
              data[PeakEnd] = QPointF(x_right, 0);
              data[PeakRetentionTime] = x_peak_maximum;
              data[PeakHeight] = peakHeight;
              data[PeakId] = sbaction->SpinBox.get()->value();

              auto cmd = new AddEntityCommand(mModel, data, TypePeak);
              emit newCommand(cmd);
            };
          }
        }
        else
        {
          switch (PeakTypes(peak_type))
          {
          case PTNone:
          case PTFake:
          case PTMarker:
            break;
          case PTInterMarker:
            if (mStepInteractions.slaveInteractions.testFlag(GenesisMarkup::SISSetUnsetInterMarker))
            {
              QAction* unSetInterMarkerActions = menu.addAction(tr("UnsetInterMarker"));
              actions[unSetInterMarkerActions] = [this, entity_id]()
              {
                auto cmd = new ChangeEntityDataCommand(mModel, entity_id, PeakType, PTPeak);
                emit newCommand(cmd);
              };

              QAction* removeActions = menu.addAction(tr("RemoveInterMarker"));
              actions[removeActions] = [this, entity_id]() {
                auto cmd = new RemoveEntityCommand(mModel, entity_id);
                emit newCommand(cmd);
              };
            }
            break;

          case PTPeak:
            if (mStepInteractions.slaveInteractions.testFlag(GenesisMarkup::SISSetUnsetInterMarker))
            {
              QAction* setPeakAsInterMarker = nullptr;

              SpinBoxAction* sbaction = nullptr;
              setPeakAsInterMarker = sbaction = new SpinBoxAction(tr("Set as inter marker"));
              auto availableRange = getMinMax(left_peak_id, right_peak_id);

              if (availableRange.first != -1 && availableRange.second != -1 && (availableRange.second - availableRange.first > 1))
              {
                menu.addAction(setPeakAsInterMarker);
                sbaction->SpinBox->setMinimum(availableRange.first + 1);
                sbaction->SpinBox->setMaximum(availableRange.second - 1);
                actions[setPeakAsInterMarker] = [this, entity_id, sbaction]()
                {
                  DataModel data = *mModel->getEntity(entity_id);

                  data[PeakType] = PTInterMarker;
                  data[PeakId] = sbaction->SpinBox.get()->value();

                  auto cmd = new ResetEntityDataCommand(mModel, entity_id, data);
                  emit newCommand(cmd);
                };
              }
            }
            break;
          }
        }
      }


      if (mStepInteractions.slaveInteractions.testFlag(SISAddRemoveBaseLinePoint))
      {
        QList<QVariant> selectionDetails;
        QPoint pixelPos = event->pos();
        auto layerables = layerableListAt(pixelPos, false, &selectionDetails);
        for(auto& l : layerables)
        {
          if(auto baseLine = qobject_cast<ChromatogramBaseline*>(l))
          {
            auto closestPair = baseLine->closestPoint(pixelPos);
            if(closestPair.first < (double)mSelectionTolerance * 0.99)
            {
              menu.addSeparator();
              QAction* imageActions = menu.addAction(tr("RemoveBaseLinePoint"));
              actions[imageActions] = [ baseLine, closestPair]()
              {
                baseLine->removePointFromBaseLine(closestPair.second->key);
              };
              break;
            }
            else
            {
              menu.addSeparator();
              QAction* imageActions = menu.addAction(tr("AddBaseLinePoint"));
              actions[imageActions] = [pixelPos, baseLine]()
              {
                auto pos = baseLine->pixelsToCoords(pixelPos);
                baseLine->addPointToBaseLine(pos.x(), pos.y());
              };
            }
          }
        }

      }
    }  /// END SLAVE PLOT MENU
    if ((master && mStepInteractions.masterInteractions.testFlag(SIMExportAsImage)) ||
        (!master && mStepInteractions.slaveInteractions.testFlag(SISExportAsImage)))
    {
      menu.addSeparator();
      QAction* saveImageAction = menu.addAction(tr("Export Image"));
      actions[saveImageAction] = [this]() { saveImage(); };

      QAction* copyImageAction = menu.addAction(tr("Copy Image"));
      actions[copyImageAction] = [this]() { copyImage(); };

    }
  }
  else
    // INTERVALS MODE
  {

    QList<QUuid> intervals_uids = mModel->getUIdListOfEntities(GenesisMarkup::TypeInterval);
    std::sort(intervals_uids.begin(), intervals_uids.end(), [this](const QUuid& a, const QUuid& b) -> bool
    {
      auto aData = mModel->getEntity(a);
      auto bData = mModel->getEntity(b);
      auto aIter = aData->constFind(GenesisMarkup::IntervalLeft);
      auto bIter = bData->constFind(GenesisMarkup::IntervalLeft);
      if (aIter == aData->constEnd() || !aIter.value().isValid())
      {
        return false;  // a >= b
      }
      else
      {
        if (bIter == bData->constEnd() || !bIter.value().isValid())
          return true;  // a < b
        else
        {
          return aIter.value().value<TIntervalLeft>() < bIter.value().value<TIntervalLeft>();
        }
      }
    });

    AbstractEntityDataModel::ConstDataPtr interval_entity = nullptr;
    QUuid interval_entity_id;
    double x_key = xAxis->pixelToCoord(event->pos().x());
    double x_left = x_key - 50;
    double x_right = x_key + 50;
    GenesisMarkup::IntervalTypes interval_type = ITNone;

    for (int i = 0; i < intervals_uids.size(); i++)
    {
      auto& uid = intervals_uids[i];
      auto entityPtr = mModel->getEntity(uid);
      if (!entityPtr)
        continue;
      else
      {
        auto x_start = entityPtr->value(GenesisMarkup::IntervalLeft).value<TIntervalLeft>();
        auto x_end = entityPtr->value(GenesisMarkup::IntervalRight).value<TIntervalRight>();

        if (!interval_entity && x_start <= x_key && x_key <= x_end)
        {
          interval_entity = entityPtr;
          interval_entity_id = intervals_uids[i];
          interval_type = (GenesisMarkup::IntervalTypes)entityPtr->getData(IntervalType).toInt();
        }
        else
        {
          if (x_end <= x_key)
          {
            if(x_left < x_end)
            {
              x_left = x_end;
            }
          }

          if (x_start >= x_key)
          {
            if(x_right > x_start)
            {
              x_right = x_start;
            }
            break;
          }
        }
      }
    }

    if(!interval_entity)
    {
      {
        DataModel data;

        data[IntervalType] = ITCustomParameters;
        data[IntervalLeft] = x_left;
        data[IntervalRight] = x_right;
        data[MinimalPeakHeight] = Defaults::kChromatogrammSettingsMinimalPeakHeightDefault;
        data[Noisy] = Defaults::kChromatogrammSettingsNoisyDefault;
        data[ViewSmooth] = Defaults::kChromatogrammSettingsViewSmoothDefault;
        data[MedianFilterWindowSize] = Defaults::kChromatogrammSettingsMedianFilterWindowSizeDefault;
        data[SignalToMedianRatio] = Defaults::kChromatogrammSettingsSignalToMedianRatioDefault;
        data[DegreeOfApproximation] = Defaults::kChromatogrammSettingsDegreeOfApproximationDefault;
        data[BoundsByMedian] = Defaults::kChromatogrammSettingsBoundsByMedianDefault;
        data[Coelution] = Defaults::kChromatogrammSettingsCoelutionDefault;

        QAction* action = menu.addAction(tr("Add Parameters Interval"));
        actions[action] = [this, data]() {
          auto cmd = new AddEntityCommand(mModel, data, TypeInterval);
          emit newCommand(cmd);
        };
      }

      {
        DataModel data;

        data[IntervalType] = ITMarkupDeprecation;
        data[IntervalLeft] = x_left;
        data[IntervalRight] = x_right;

        QAction* action = menu.addAction(tr("Add Deprecation Interval"));
        actions[action] = [this, data]() {
          auto cmd = new AddEntityCommand(mModel, data, TypeInterval);
          emit newCommand(cmd);
        };
      }
    }
    else
    {
      QString text;
      switch (interval_type)
      {
      case ITNone:
        break;
      case ITCustomParameters:
        text = tr("Remove Parameters Interval");
        break;
      case ITMarkupDeprecation:
        text = tr("Remove Deprecation Interval");
        break;
      }
      if(!text.isEmpty())
      {
        QAction* removeActions = menu.addAction(text);
        actions[removeActions] = [this, interval_entity_id]()
        {
          auto cmd = new RemoveEntityCommand(mModel, interval_entity_id);
          emit newCommand(cmd);
        };
      }
    }

    ///@todo Move to buttons
    QAction* imageActions = menu.addAction(tr("Save Intervals"));
    actions[imageActions] = [this]() {
      auto dial = new ChromatogramWidgetSettingsDialogRefactor(mModel, mStepInteractions, 1, this);

      connect(dial, &WebDialog::Accepted, this, [this, dial]()
      {
        if(mModel->isMaster())
        {
          Q_ASSERT(mModel->markupModel()->hasInternalMaster());
          auto markupModel = mModel->markupModel();
          emit intervalsApplyingRequested(markupModel->getChromatogrammsIdList(), dial->getIntervalsSettings(), dial->getDeprecationIntervals(), dial->getSettings(), mStepInteractions.step);
        }
        else
        {
          emit intervalsApplyingRequested({mModel->id()}, dial->getIntervalsSettings(), dial->getDeprecationIntervals(), dial->getSettings(), mStepInteractions.step);
        }
      }, Qt::DirectConnection);
      dial->Open();
    };
  };

  // Cut tail
  //    if(master && mStepInteractions.masterInteractions.testFlag(SIMCutTail)
  //            || !master &&
  //            mStepInteractions.slaveInteractions.testFlag(SISCutTail))
  //    {
  //        menu.addSeparator();
  //        QAction* imageActions = menu.addAction(tr("CutTail"));
  //        actions[imageActions]= [this](){ qDebug()<<"CutTail"; };
  //    }

  //    if(master && mStepInteractions.masterInteractions.testFlag(SIMCutTail)
  //            || !master &&
  //            mStepInteractions.slaveInteractions.testFlag(SISCutTail))
  //    {
  //        menu.addSeparator();
  //        QAction* imageActions = menu.addAction(tr("CutTail"));
  //        actions[imageActions]= [this](){ qDebug()<<"CutTail"; };
  //    }
}

void ChromatogramPlot::mousePressEvent(QMouseEvent* event)
{
  if (event->button() == Qt::LeftButton)
  {
    if(event->modifiers().testFlag(Qt::AltModifier))
    {
      bool peakAddAllowed = (isMaster() && mStepInteractions.masterInteractions.testFlag(SIMAddRemovePeak)) ||
                            (!isMaster() && mStepInteractions.slaveInteractions.testFlag(SISAddRemovePeak));
      if(peakAddAllowed)
      {
        QList<QUuid> uids = mModel->getUIdListOfEntities(GenesisMarkup::TypePeak);
        std::sort(uids.begin(), uids.end(), [this](const QUuid& a, const QUuid& b) -> bool {
          auto aData = mModel->getEntity(a);
          auto bData = mModel->getEntity(b);
          auto aIter = aData->constFind(GenesisMarkup::PeakStart);
          auto bIter = bData->constFind(GenesisMarkup::PeakStart);
          if (aIter == aData->constEnd() || !aIter.value().isValid())
          {
            return false;  // a >= b
          }
          else
          {
            if (bIter == bData->constEnd() || !bIter.value().isValid())
              return true;  // a < b
            else
              return aIter.value().toPointF().x() < bIter.value().toPointF().x();
          }
        });

        AbstractEntityDataModel::ConstDataPtr entity = nullptr;
        QUuid left_peak_id;
        QUuid right_peak_id;
        QUuid entity_id;
        PeakTypes peak_type = GenesisMarkup::PTNone;
        double x_key = xAxis->pixelToCoord(event->pos().x());
        double x_left = x_key - 3;
        double x_right = x_key + 3;
        bool find_left = false;
        bool find_right = false;

        for (int i = 0; i < uids.size(); i++)
        {
          auto& uid = uids[i];
          auto entityPtr = mModel->getEntity(uid);
          if (!entityPtr)
            continue;
          auto x_start = entityPtr->value(GenesisMarkup::PeakStart).toPointF().x();
          auto x_end = entityPtr->value(GenesisMarkup::PeakEnd).toPointF().x();

          if (!entity && x_start <= x_key && x_key <= x_end)
          {
            entity = entityPtr;
            entity_id = uids[i];
            peak_type = (GenesisMarkup::PeakTypes)entityPtr->getData(PeakType).toInt();
          }
          else
          {
            if (x_end <= x_key)
            {
              if(x_left < x_end)
              {
                x_left = x_end;
              }
              left_peak_id = uids[i];
            }

            if (x_start >= x_key)
            {
              if(x_right > x_start)
              {
                x_right = x_start;
              }
              right_peak_id = uids[i];
              break;
            }
          }
        }

        if(!entity)
        {
          if(!mTemporaryPeakUid.isNull())
          {
            mLayerables.removeAndDelete(mTemporaryPeakUid);
            mTemporaryPeakUid = QUuid();
          }
          auto curveUuid = mModel->getUIdListOfEntities(GenesisMarkup::TypeCurve);
          if(curveUuid.empty() || curveUuid.first().isNull()){
            Q_ASSERT_X(false, "ChromatogramPlot::onModelReset()", "curveUuid.empty() || curveUuid.first().isNull()");
            return;
          }
          if(!mLayerables.contains(curveUuid.first()))
          {
            Q_ASSERT_X(false, "ChromatogramPlot::onModelReset()", "cant create peak, no curve found");
            return;
          }
          auto chromaCurve = static_cast<GPGraph*>(mLayerables.get(curveUuid.first()));

          QUuid firstBaseLineUid;
          auto uids = mModel->getUIdListOfEntities(GenesisMarkup::TypeBaseline);
          if(!uids.isEmpty())
          {
            auto uidIter = uids.begin();
            while(!mModel->getEntity(*uidIter) && uidIter != uids.end())
              uidIter++;
            if(uidIter != uids.end())
              firstBaseLineUid = *uidIter;
          }

          auto start = xAxis->pixelToCoord(event->pos().x());
          auto end = xAxis->pixelToCoord(event->pos().x() + 1);
          mTemporaryPeakUid = QUuid::createUuid();
          auto peak = new GenesisMarkup::ChromatogramPeak(this, *chromaCurve, start, end, mTemporaryPeakUid);

          if(!firstBaseLineUid.isNull())
            peak->setBaseLine((ChromatogramBaseline*)mLayerables.get(firstBaseLineUid));

          peak->setPeakType(GenesisMarkup::PTPeak);
          peak->setLeftPeak(qobject_cast<ChromatogramPeak*>(mLayerables.get(left_peak_id)));
          peak->setRightPeak(qobject_cast<ChromatogramPeak*>(mLayerables.get(right_peak_id)));

          mLayerables.add(mTemporaryPeakUid, peak);
          event->accept();
          return;
        }
      }
    }
    if(baseLineInteractionsMode)
    {
      if(event->modifiers().testFlag(Qt::ControlModifier))
        setSelectionRectMode(GP::srmNone);
      else
        setSelectionRectMode(GP::srmSelect);
    }
    // See where hit
    for (const auto& candidate : layerableListAt(event->pos(), false))
    {
      unsigned part;
      auto cursor = candidate->HitTest(event, &part);
      if (cursor != Qt::ArrowCursor)
      {
        if (auto di = dynamic_cast<GPEXDraggingInterface*>(candidate))
        {
          DraggingObject = di;
          di->DragStart(event, part);
          event->accept();
          return;
        }
      }
    }
  }
  else if (event->button() == Qt::RightButton)
  {
    //do nothing
  }
  GraphicsPlot::mousePressEvent(event);
}

void ChromatogramPlot::mouseReleaseEvent(QMouseEvent* event)
{
  if (DraggingObject)
  {//if we dragging something
    DraggingObject->DragAccept(event);
    DraggingObject = nullptr;
  }
  if(event->button() == Qt::LeftButton && !mTemporaryPeakUid.isNull())
  {//if new peak creating in progress
    qDebug() << "accept peak";
    auto peak = qobject_cast<ChromatogramPeak*>(mLayerables.get(mTemporaryPeakUid));
    if(peak)
    {
      auto left = peak->leftBorder();
      auto right = peak->rightBorder();
      auto extrem = peak->peakCenter().x();
      auto height = peak->peakCenter().y();
      auto type = peak->peakType();

      //удаляем временный пик
      mLayerables.removeAndDelete(mTemporaryPeakUid);
      mTemporaryPeakUid = QUuid();

      //создаем постоянный пик
      DataModel data;
      data[PeakType] = type;
      data[PeakStart] = QPointF(left, 0);
      data[PeakEnd] = QPointF(right, 0);
      data[PeakRetentionTime] = extrem;
      data[PeakHeight] = height;
      auto cmd = new AddEntityCommand(mModel, data, TypePeak);
      emit newCommand(cmd);
      event->accept();
      return;
    }
  }
  GraphicsPlot::mouseReleaseEvent(event);
}

void ChromatogramPlot::mouseDoubleClickEvent(QMouseEvent *event)
{

#ifdef Q_OS_WASM // <- looks like wasm cant receive right button clicks, provide menu by double click
  {
    // Global actions
    if (contextMenu)
    {
      delete contextMenu;
    }
    contextMenu = new QMenu(this);
    std::map<QAction*, std::function<void(void)>> actions;

    QContextMenuEvent cme(QContextMenuEvent::Mouse, event->pos(), event->globalPos());
    FillContextMenu(&cme, *contextMenu, actions);

    if (!actions.empty())
    {
      connect(contextMenu, &QMenu::triggered, [actions](QAction* action)
      {
        auto a = actions.find(action);
        if (a != actions.end())
        {
          a->second();
        }
      });

      // Show
      contextMenu->popup(event->globalPos());
      event->accept();

      // Done
      return;
    }
  }
#else
  if(baseLineInteractionsMode)
  {
    ChromatogramBaseline* baseLine = nullptr;
    QUuid firstBaseLineUid;
    auto uids = mModel->getUIdListOfEntities(GenesisMarkup::TypeBaseline);
    if(!uids.isEmpty())
    {
      auto uidIter = uids.begin();
      while(!mModel->getEntity(*uidIter) && uidIter != uids.end())
        uidIter++;
      if(uidIter != uids.end())
      {
        firstBaseLineUid = *uidIter;
      }
    }
    if(!firstBaseLineUid.isNull())
    {
      baseLine = (ChromatogramBaseline*)mLayerables.get(firstBaseLineUid);
    }
    if(baseLine)
    {
      auto pos = baseLine->pixelsToCoords(event->pos());
      baseLine->addPointToBaseLine(pos.x(), pos.y());
    }
  }
#endif // ~Q_OS_WASM
  return GraphicsPlot::mouseDoubleClickEvent(event);
}

void ChromatogramPlot::saveImage()
{
#ifdef Q_OS_WASM
  QBuffer buffer;
  savePng(&buffer);
  QFileDialog::saveFileContent(buffer.data(), "plot.png");
#else
  QString path = QFileDialog::getSaveFileName(this, tr("Set save path"), "", "png (*.png);;pdf (*.pdf)");
  if (!path.isEmpty())
  {
    path = QDir::toNativeSeparators(path);
    if(path.endsWith(".png", Qt::CaseInsensitive))
      savePng(path);
    else if(path.endsWith(".pdf", Qt::CaseInsensitive))
      savePdf(path);
    else
    {
      path += ".png";
      savePng(path);
    }
  }
#endif
}

void ChromatogramPlot::copyImage()
{
  QImage buffer = toPixmap(0, 0, 1.0).toImage();

  int resolution = 96;
  int dotsPerMeter = resolution/0.0254;
  buffer.setDotsPerMeterX(dotsPerMeter);
  buffer.setDotsPerMeterY(dotsPerMeter);
  QApplication::clipboard()->setImage(buffer, QClipboard::Clipboard);
}

void ChromatogramPlot::handleStepInteractions(const GenesisMarkup::StepInfo& newModelStepInteractions)
{
  mStepInteractions = newModelStepInteractions;
  handleStepInteractions();
}

void ChromatogramPlot::handleStepInteractions()
{
  bool enabledStyle = isMaster() ?
                        mStepInteractions.masterInteractions.testFlag(GenesisMarkup::SIMEnabledStyle) :
                        mStepInteractions.slaveInteractions.testFlag(GenesisMarkup::SISEnabledStyle);
  if(enabledStyle)
    setBackground(Qt::white);
  else
    setBackground(QColor(242, 244, 245));
}

bool LayerableContainer::add(const QUuid& uid, GPLayerable* ptr)
{
  if (mHash.contains(uid) || ptr == nullptr)
    return false;
  mHash[uid] = ptr;
  ptr->setProperty("uid", uid);
  connect(ptr, &QObject::destroyed, this, &LayerableContainer::onLayerableDeleted);
  return true;
}

bool LayerableContainer::hasSelected()
{
  if (mHash.empty())
    return false;
  for (auto& l : mHash)
  {
    auto prop = l->property("isSelected");
    if (prop.isValid() && prop.toBool())
      return true;
  }
  return false;
}

GPLayerable* LayerableContainer::get(const QUuid& uid) const
{
  if (mHash.contains(uid))
    return mHash[uid];
  else
    return nullptr;
}

void LayerableContainer::remove(const QUuid& uid)
{
  auto ptr = get(uid);
  if (!ptr)
    return;
  ptr->setProperty("uid", QVariant());
  disconnect(ptr, nullptr, this, nullptr);
  mHash.remove(uid);
}

void LayerableContainer::removeAndDelete(const QUuid& uid)
{
  auto ptr = get(uid);
  if (!ptr)
    return;
  delete ptr;
  mHash.remove(uid);
}

void LayerableContainer::clearAndDelete()
{
  QList<GPLayerable*> removeList{ mHash.begin(), mHash.end() };
  for (auto ptr : removeList)
    delete ptr;
}

void LayerableContainer::onLayerableDeleted()
{
  auto uidVariant = sender()->property("uid");
  if (!uidVariant.isValid() || uidVariant.isNull())
  {
    Q_ASSERT(false);  // Layerable miss it's UID but still send his "destroyed"
    // signal to container
    return;
  }
  auto key = sender()->property("uid").toUuid();
  if (mHash.contains(key))
    mHash.remove(key);
}

void ChromatogramPlot::paintEvent(QPaintEvent *event)
{
  GraphicsPlot::paintEvent(event);
}

void ChromatogramPlot::keyPressEvent(QKeyEvent *event)
{
  switch(event->key())
  {
  case Qt::Key_Delete:
  {
    if(!mLayerables.hasSelected())
      break;
    QList<QUuid> peaksToDelete;
    for(auto& item : mLayerables.getHash())
    {
      auto prop = item->property("isSelected");
      if (!prop.isValid() || !prop.toBool())
        continue;
      if(auto peak = qobject_cast<ChromatogramPeak*>(item))
      {
        auto peakEntity = mModel->getEntity(peak->uid());
        PeakTypes peakType = (PeakTypes)peakEntity->getData(ChromatogrammEntityDataRoles::PeakType).toInt();
        bool master = isMaster();
        if(master)
        {
          switch(peakType)
          {
          case PTPeak:
            if(mStepInteractions.masterInteractions.testFlag(SIMAddRemovePeak))
              peaksToDelete << peak->uid();
            break;
          case PTMarker:
            if(mStepInteractions.masterInteractions.testFlag(GenesisMarkup::SIMAddRemoveMarker))
              peaksToDelete << peak->uid();
            break;
          case PTInterMarker:
            if(mStepInteractions.masterInteractions.testFlag(GenesisMarkup::SIMAddRemoveInterMarker))
              peaksToDelete << peak->uid();
            break;
          default:
            break;
          }
        }
        else
        {
          switch(peakType)
          {
          case PTPeak:
            if(mStepInteractions.slaveInteractions.testFlag(SISAddRemovePeak))
              peaksToDelete << peak->uid();
            break;
          case PTMarker:
            if(mStepInteractions.slaveInteractions.testFlag(GenesisMarkup::SISAddRemoveMarker))
              peaksToDelete << peak->uid();
            break;
          case PTInterMarker:
            if(mStepInteractions.slaveInteractions.testFlag(GenesisMarkup::SISAddRemoveInterMarker))
              peaksToDelete << peak->uid();
            break;
          default:
            break;
          }
        }
      }
      else if(auto bl = qobject_cast<ChromatogramBaseline*>(item))
      {
        if(baseLineInteractionsMode)
          bl->removeSelectedPointsFromBaseLine();
      }
    }
    if(!peaksToDelete.isEmpty())
    {
      QUndoCommand* cmd = new RemoveEntityListCommand(mModel, peaksToDelete);
      emit newCommand(cmd);
    }
  }
    break;
  default:
    break;
  }
}
