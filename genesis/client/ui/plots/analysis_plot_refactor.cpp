#include "analysis_plot_refactor.h"
#include <ui/plots/gp_items/gpshape_with_label_item.h>
#include <ui/plots/gp_items/sample_info_plate.h>
#include <genesis_style/style.h>
#include <ui/dialogs/web_overlay.h>
// #include <ui/genesis_window.h>

using namespace AnalysisEntity;
const char* AnalysisPlotRefactor::kSampleId = "sample_id";
const char* AnalysisPlotRefactor::kEntityUid = "entity_id";
const char* AnalysisPlotRefactor::kOrderNum = "order_num";
const char* AnalysisPlotRefactor::kPeakId = "id";

AnalysisPlotRefactor::AnalysisPlotRefactor(QWidget* parent, const QSet<EntityType>& displayedTypes)
  : GraphicsPlot(parent),
  mModel(nullptr),
  mDisplayedItemsTypes(displayedTypes)
{
  setPlottingHint(GP::phImmediateRefresh, false);
  if (!mSelectionRect)
  {
    setSelectionRect(new GPSelectionRect(this));
  }
  auto rect = selectionRect();
  rect->setPen(QPen(QColor(0, 120, 210), 2));
  rect->setBrush(QBrush(QColor(0, 32, 51, 13)));
  setSelectionRectMode(GP::srmNone);
  setInteraction(GP::iRangeDrag, true);
  setInteraction(GP::iRangeZoom, true);
  setInteraction(GP::iSelectItems, true);
  setInteraction(GP::iMultiSelect, true);
  setMultiSelectModifier(Qt::ShiftModifier);
  addLayer("shapeLayer");
  addLayer("passportLayer");
}

void AnalysisPlotRefactor::setModel(QPointer<AnalysisEntityModel> model)
{
  if (model && model == mModel)
  {
    mShowNames = true;
    mShowPassports = true;
    onModelReset();
    return;
  }
  mShowNames = true;
  mShowPassports = true;
  if (mModel)
    disconnect(mModel, nullptr, this, nullptr);
  mModel = model;
  if (mModel)
    connectSignals(mModel);
  onModelReset();
}

AnalysisEntityModel* AnalysisPlotRefactor::getModel()
{
  return mModel;
}

void AnalysisPlotRefactor::resizeCached()
{
  resizeAxisToDefault();
}

void AnalysisPlotRefactor::resize()
{
  if (mUidsShapes.isEmpty())
    return;
  {
    auto firstPos = mUidsShapes.begin().value()->GetPos();
    mXRangeDefault = GPRange(firstPos.x(), firstPos.x());
    mYRangeDefault = GPRange(firstPos.y(), firstPos.y());
  }
  for (auto& shape : mUidsShapes)
  {
    auto pos = shape->GetPos();
    mXRangeDefault.expand(pos.x());
    mYRangeDefault.expand(pos.y());
  }

  double tenXPercents = mXRangeDefault.size() * 0.1;
  double tenYPercents = mYRangeDefault.size() * 0.1;
  //10% expanding in both directions
  mXRangeDefault.expand(mXRangeDefault.lower - tenXPercents);
  mXRangeDefault.expand(mXRangeDefault.upper + tenXPercents);
  mYRangeDefault.expand(mYRangeDefault.lower - tenYPercents);
  mYRangeDefault.expand(mYRangeDefault.upper + tenYPercents);
  resizeAxisToDefault();
}

void AnalysisPlotRefactor::setDefaultAxisRanges(const GPRange& xRange, const GPRange& yRange)
{
  mXRangeDefault = xRange;
  mYRangeDefault = yRange;
  resizeAxisToDefault();
}

void AnalysisPlotRefactor::resizeAxisToDefault()
{
  if (mModel->modelData(ModelRoleIsSpectral).toBool())
  {
    mYRangeDefault = GPRange(0, 0);
    for (int i = 0; i < mCurve->dataCount(); i++)
    {
      mYRangeDefault.expand(mCurve->dataValueRange(i));
    }
    double tenYPercents = mYRangeDefault.size() * 0.1;
    mYRangeDefault.expand(mYRangeDefault.lower - tenYPercents);
    mYRangeDefault.expand(mYRangeDefault.upper + tenYPercents);
    mXRangeDefault = GPRange(-1, mCurve->dataCount() + 1);
  }
  xAxis->setRange(mXRangeDefault);
  yAxis->setRange(mYRangeDefault);
  replot(GraphicsPlot::rpQueuedReplot);
}

void AnalysisPlotRefactor::setXAxisLabel(const QString& label)
{
  xAxis->setLabel(label);
}

void AnalysisPlotRefactor::setYAxisLabel(const QString& label)
{
  yAxis->setLabel(label);
}

void AnalysisPlotRefactor::setDisplayedItemTypes(const QSet<EntityType>& types)
{
  mDisplayedItemsTypes = types;
  onModelReset();
}

void AnalysisPlotRefactor::select(const QSet<TEntityUid>& uids)
{
  for (auto& uid : uids)
    if (auto shape = mUidsShapes.value(uid, nullptr))
      shape->setSelected(true);
  queuedReplot();
}

void AnalysisPlotRefactor::showNames(bool isShow)
{
  mShowNames = isShow;
  if (!mModel)
    return;
  for (const auto& [uid, shape]: mUidsShapes.asKeyValueRange())
  {
    if (auto ent = mModel->getEntity(uid))
      shape->setLabelHidden(!mShowNames && !ent->getData(RoleForceShowName).toBool());
  }
  queuedReplot();
}

void AnalysisPlotRefactor::showPassport(bool isShow)
{
  mShowPassports = isShow;
  for (const auto& [uid, shape] : mUidsShapes.asKeyValueRange())
  {
    if (auto plate = mShapesPlates.value(shape, nullptr))
    {
      if (auto ent = mModel->getEntity(uid))
        plate->showParameters(mShowPassports || ent->getData(RoleForceShowPassport).toBool());
    }
  }
  queuedReplot();
}

bool AnalysisPlotRefactor::namesHidden()
{
  return !mShowNames;
}

bool AnalysisPlotRefactor::passportsHidden()
{
  return !mShowPassports;
}

void AnalysisPlotRefactor::setRenderType(AnalysisEntity::RenderType type)
{
  mRenderType = type;
}

void AnalysisPlotRefactor::queuedReplot()
{
  if (!isVisible())
    mDirty = true;
  else
    replot(rpQueuedReplot);
}

void AnalysisPlotRefactor::handleHighlighting(QPointF plotPos)
{
  if (!mModel)
    return;
  if (mSelectionRect->isActive() && !mHasHighlighting)
  {
    return;
  }
  auto excludedSamples = mModel->getEntitiesUidsSet([](AbstractEntityDataModel::ConstDataPtr data)->bool
    {
      return data->getData(RoleExcluded).toBool();
    });
  QList<GPShapeWithLabelItem*> shapesUnderCursor;
  for(const auto& [shape, info] : mShapesPlates.asKeyValueRange())
  {
    auto shapeBoundingRect = shape->boundingRect();
    //add margins by 10 pix to each side
    shapeBoundingRect.translate(-10, -10);
    shapeBoundingRect.setWidth(shapeBoundingRect.height() + 10);
    shapeBoundingRect.setHeight(shapeBoundingRect.width());

    if (shapeBoundingRect.contains(plotPos))
    {
      shapesUnderCursor << shape;
    }
    auto uid = shape->property(kEntityUid).value<TEntityUid>();
    if (!shape->isInactive() && excludedSamples.contains(uid))
    {
      shape->setInactive(true);
      info->SetIsInactive(true);
    }
    else if (shape->isInactive() && !excludedSamples.contains(uid))
    {
      shape->setInactive(false);
      info->SetIsInactive(false);
    }
  }

  mHasHighlighting = false;
  if (mSelectionRect->isActive())
    return;
  if (shapesUnderCursor.isEmpty())
    return;

  auto nearestShape = *std::min_element(shapesUnderCursor.begin(), shapesUnderCursor.end(),
    [](GPShapeWithLabelItem* lt, GPShapeWithLabelItem* rt)
    {
      auto ltShapeBoundingRect = lt->boundingRect();
      auto rtShapeBoundingRect = rt->boundingRect();
      return ltShapeBoundingRect.center().manhattanLength() < rtShapeBoundingRect.center().manhattanLength();
    });

  //mShapesPlates[nearestShape] can't be nullptr cause it happens only when items is empty,
  //but if that happens, we will return earlier.
  nearestShape->setInactive(false);
  mShapesPlates[nearestShape]->SetIsInactive(false);
  mShapesPlates[nearestShape]->moveToLayer(layer("passportLayer"), false);
  nearestShape->moveToLayer(layer("shapeLayer"), false);

  auto currentCombinedRect = mShapesPlates[nearestShape]->BoundingRect().united(nearestShape->boundingRect());
  for (auto iter = mShapesPlates.constKeyValueBegin(); iter != mShapesPlates.constKeyValueEnd(); iter++)
  {
    if (iter->first == nearestShape)
      continue;
    auto uid = iter->first->property(kEntityUid).value<TEntityUid>();
    if (iter->second->BoundingRect().intersects(currentCombinedRect)
      || iter->first->boundingRect().intersects(currentCombinedRect))
    {
      iter->second->SetIsInactive(true);
      iter->first->setInactive(true);
    }
  }
  mHasHighlighting = true;
}

void AnalysisPlotRefactor::disconnectSignals(AnalysisEntityModel* model)
{
  if (!model)
    return;
  disconnect(model, &AnalysisEntityModel::modelDataChanged, this, &AnalysisPlotRefactor::onModelDataChanged);
  disconnect(model, &ShapeColorGroupedEntityPlotDataModel::groupColorChanged, this, &AnalysisPlotRefactor::onGroupColorChanged);
  disconnect(model, &ShapeColorGroupedEntityPlotDataModel::groupShapeChanged, this, &AnalysisPlotRefactor::onGroupShapeChanged);
  disconnect(model, &AbstractEntityDataModel::entityChanged, this, &AnalysisPlotRefactor::onEntityChanged);
  disconnect(model, &AbstractEntityDataModel::entityResetted, this, &AnalysisPlotRefactor::onEntityResetted);
  disconnect(model, &AbstractEntityDataModel::entityAdded, this, &AnalysisPlotRefactor::onEntityAdded);
  disconnect(model, &AbstractEntityDataModel::entityAboutToRemove, this, &AnalysisPlotRefactor::onEntityAboutToRemove);
  disconnect(model, &AbstractEntityDataModel::modelAboutToReset, this, &AnalysisPlotRefactor::onModelAboutToReset);
  disconnect(model, &AbstractEntityDataModel::modelReset, this, &AnalysisPlotRefactor::onModelReset);
}

void AnalysisPlotRefactor::connectSignals(AnalysisEntityModel* model)
{
  if (!model)
    return;
  connect(model, &AnalysisEntityModel::modelDataChanged, this, &AnalysisPlotRefactor::onModelDataChanged);
  connect(model, &ShapeColorGroupedEntityPlotDataModel::groupColorChanged, this, &AnalysisPlotRefactor::onGroupColorChanged);
  connect(model, &ShapeColorGroupedEntityPlotDataModel::groupShapeChanged, this, &AnalysisPlotRefactor::onGroupShapeChanged);
  connect(model, &AbstractEntityDataModel::entityChanged, this, &AnalysisPlotRefactor::onEntityChanged);
  connect(model, &AbstractEntityDataModel::entityResetted, this, &AnalysisPlotRefactor::onEntityResetted);
  connect(model, &AbstractEntityDataModel::entityAdded, this, &AnalysisPlotRefactor::onEntityAdded);
  connect(model, &AbstractEntityDataModel::entityAboutToRemove, this, &AnalysisPlotRefactor::onEntityAboutToRemove);
  connect(model, &AbstractEntityDataModel::modelAboutToReset, this, &AnalysisPlotRefactor::onModelAboutToReset);
  connect(model, &AbstractEntityDataModel::modelReset, this, &AnalysisPlotRefactor::onModelReset);
}

void AnalysisPlotRefactor::onModelDataChanged(const QHash<int, QVariant>& roleDataMap)
{
  if (mIsInResetState)
    return;
  bool componentsAlredyUpdated = false;
  for (auto iter = roleDataMap.constKeyValueBegin(); iter != roleDataMap.constKeyValueEnd(); iter++)
  {
    switch ((AnalysisModelDataRoles)iter->first)
    {
    case ModelRoleSpectralRenderType:
    {
      if (mCurve)
      {
        const auto peaksList = mModel->getEntities(TypePeak);
        const auto uid = peaksList.first()->getUid();
        const auto shapeDesc = mModel->getShapeDescriptor(uid);
        const auto color = mModel->getActualColor(uid);
        mCurve->setPen(color);
        mCurve->setScatterStyle(
          GPScatterStyle((iter->second == AnalysisEntity::Dot)
            ? GPScatterStyle::ssDisc
            : GPScatterStyle::ssNone,
            color, shapeDesc.pixelSize));
        mCurve->setLineStyle((iter->second == AnalysisEntity::Dot)
          ? GPCurve::LineStyle::lsNone
          : GPCurve::LineStyle::lsLine);
      }
      break;
    }
    case ModelRoleIsSpectral:
    {
      if (!mDisplayedItemsTypes.contains(TypePeak))
        break;
      auto peaksList = mModel->getEntities(TypePeak);
      for (auto& peak : peaksList)
      {
        auto uid = peak->getUid();
        if (mUidsShapes.contains(peak->getUid()))
        {
          auto shape = mUidsShapes[uid];
          if (iter->second.toBool())
          {
            shape->setVisible(false);
            if (!mCurve)
            {
              mCurve = addCurve();
            }
            updateCurve();
            rescaleAxes(true);
          }
          else
          {
            shape->setVisible(true);
            removeCurve(mCurve);
            mCurve = nullptr;
            updateShapes(TypePeak);
            resize();
          }
        }
      }
      queuedReplot();
      break;
    }
    case ModelRoleCurrentSpectralComponentNum:
    {
      updateCurve();
      break;
    }
    case ModelRoleCurrentXComponentNum:
    case ModelRoleCurrentYComponentNum:
    {
      if (componentsAlredyUpdated)
        break;
      updateShapes();
      componentsAlredyUpdated = true;
      rescaleAxes(true);
      break;
    }
    case ModelRoleComponentsExplVarianceMap:
    case ModelRoleLast:
    default:
      break;
    }
  }
  queuedReplot();
}

void AnalysisPlotRefactor::onGroupColorChanged(QSet<TEntityUid> affectedEntities)
{
  for (const auto& [uid, shape] : mUidsShapes.asKeyValueRange())
  {
    if (!affectedEntities.contains(uid))
      continue;
    shape->setColor(mModel->getActualColor(uid));
    queuedReplot();
  }
}

void AnalysisPlotRefactor::onGroupShapeChanged(QSet<TEntityUid> affectedEntities)
{
  for (const auto& [uid, shape] : mUidsShapes.asKeyValueRange())
  {
    if (!affectedEntities.contains(uid))
      continue;
    auto shapeDescriptor = mModel->getShapeDescriptor(uid);
    shape->SetShape(shapeDescriptor.path);
    shape->SetSize(QSizeF(shapeDescriptor.pixelSize, shapeDescriptor.pixelSize));
    queuedReplot();
  }
}

void AnalysisPlotRefactor::onEntityChanged(EntityType type, TEntityUid eId, DataRoleType role, const QVariant& value)
{
  if (mIsInResetState)
    return;
  switch (type)
  {
  case TypeSample:
  {
    if (!mDisplayedItemsTypes.isEmpty() && !mDisplayedItemsTypes.contains(TypeSample))
      break;
    auto shape = mUidsShapes.value(eId, nullptr);
    if (!shape)
      break;

    if (role == RolePassport)
    {
      auto plate = mShapesPlates.value(shape, nullptr);
      if (!plate)
        break;
      auto passport = value.value<TPassport>();
      plate->SetParameter(Field, passport.value(PassportTags::field).toString());
      plate->SetParameter(Well, passport.value(PassportTags::well).toString());
      plate->SetParameter(Layer, passport.value(PassportTags::layer).toString());
      plate->SetParameter(Date, passport.value(PassportTags::date).toString());
    }
    else if (role == RoleTitle)
    {
      shape->setLabelText(value.toString());
    }
    else if (role == RoleExcluded)
    {
      bool isExcluded = value.toBool();
      shape->setInactive(isExcluded);
      auto plate = mShapesPlates.value(shape, nullptr);
      if (!plate)
        break;
      plate->SetIsInactive(isExcluded);
    }
    else if (role == RoleIntId)
    {
      shape->setProperty(kSampleId, value.toInt());
    }
    else if (role == RoleComponentsData)
    {
      shape->SetPos(mModel->currentEntityCoord(eId));
    }
    else if (role == RoleColor)
    {
      shape->setColor(mModel->getActualColor(eId));
    }
    else if (role == RoleShape)
    {
      shape->SetShape(mModel->getActualShape(eId));
    }
    else if (role == RoleShapePixelSize)
    {
      auto size = mModel->getActualShapeSize(eId);
      shape->SetSize(QSizeF(size, size));
    }
    else if (role == RoleForceShowName)
    {
      shape->setLabelHidden(!mShowNames && !value.toBool());
    }
    else if (role == RoleForceShowPassport)
    {
      auto plate = mShapesPlates.value(shape, nullptr);
      if (!plate)
        break;
      plate->showParameters(mShowPassports || value.toBool());
    }
    break;
  }
  case TypePeak:
  {
    if (!mDisplayedItemsTypes.isEmpty() && !mDisplayedItemsTypes.contains(TypePeak))
      break;
    if (!mModel->modelData(ModelRoleIsSpectral).toBool())
    {
      auto shape = mUidsShapes.value(eId, nullptr);
      if (!shape)
        break;

      if (role == RolePeakData)
      {
        auto peakData = value.value<TPeakData>();
        shape->setProperty(kOrderNum, peakData.orderNum);
      }
      else if (role == RoleTitle)
      {
        shape->setLabelText(value.toString());
      }
      else if (role == RoleExcluded)
      {
        bool isExcluded = value.toBool();
        shape->setInactive(isExcluded);
      }
      else if (role == RoleIntId)
      {
        shape->setProperty(kSampleId, value.toInt());
      }
      else if (role == RoleComponentsData)
      {
        shape->SetPos(mModel->currentEntityCoord(eId));
        if (mModel->modelData(ModelRoleIsSpectral).toBool())
          updateCurve();
      }
      else if (role == RoleColor)
      {
        shape->setColor(mModel->getActualColor(eId));
      }
      else if (role == RoleShape)
      {
        shape->SetShape(mModel->getActualShape(eId));
      }
      else if (role == RoleShapePixelSize)
      {
        auto size = mModel->getActualShapeSize(eId);
        shape->SetSize(QSizeF(size, size));
      }
      else if (role == RoleForceShowName)
      {
        shape->setLabelHidden(!mShowNames && !value.toBool());
      }
      break;
    }
  }
  default:
    break;
  }
  queuedReplot();
}

void AnalysisPlotRefactor::onEntityResetted(EntityType type, TEntityUid eId, AbstractEntityDataModel::ConstDataPtr data)
{
  if (mIsInResetState)
    return;
  switch (type)
  {
  case TypeSample:
  {
    if (!mDisplayedItemsTypes.isEmpty() && !mDisplayedItemsTypes.contains(TypeSample))
      break;
    auto& uid = eId;
    auto shapeDescriptor = mModel->getShapeDescriptor(uid);
    auto passport = data->getData(RolePassport).value<TPassport>();
    auto intId = data->getData(RoleIntId).toInt();
    auto isExcluded = data->getData(RoleExcluded).toBool();
    QFont font(Style::GetSASSValue("fontFaceNormal"), -1, Style::GetSASSValue("fontWeight").toInt());
    font.setPixelSize(Style::GetSASSValue("fontSizeSecondaryTextScalableFont").toInt());
    auto title = data->getData(RoleTitle).toString();
    auto shape = mUidsShapes.value(eId, nullptr);
    if (!shape)
      return onEntityAdded(type, eId, data);
    shape->SetPos(mModel->currentEntityCoord(uid));
    shape->SetShape(shapeDescriptor.path);
    shape->SetSize(QSizeF(shapeDescriptor.pixelSize, shapeDescriptor.pixelSize));
    shape->setLabelText(title);
    shape->setLabelFont(font);
    shape->setProperty(kSampleId, intId);
    shape->setLabelHidden(!mShowNames && !data->getData(RoleForceShowName).toBool());
    {
      auto brush = QBrush(mModel->getActualColor(uid));
      shape->setColor(mModel->getActualColor(uid));
      shape->setShapeSelectedBrush(brush);
    }

    auto plate = mShapesPlates.value(shape, nullptr);
    if (!plate)
    {
      plate = new SampleInfoPlate(this);
      plate->setSelectable(false);
      plate->topLeft->setParentAnchor(shape->right);
      plate->showParameters(false);

      QPointF identation = QPointF(5, shape->boundingRect().height() / 2 + 2);
      plate->topLeft->setCoords(identation);//identation from point

      plate->showParameters(mShowPassports || data->getData(RoleForceShowPassport).toBool());
      mShapesPlates[shape] = plate;
    }
    plate->SetParameter(Field, passport.value(PassportTags::field).toString());
    plate->SetParameter(Well, passport.value(PassportTags::well).toString());
    plate->SetParameter(Layer, passport.value(PassportTags::layer).toString());
    plate->SetParameter(Date, passport.value(PassportTags::date).toString());

    shape->setInactive(isExcluded);
    plate->SetIsInactive(isExcluded);
    break;
  }
  case TypePeak:
  {
    if (!mDisplayedItemsTypes.isEmpty() && !mDisplayedItemsTypes.contains(TypePeak))
      break;
    auto& uid = eId;
    auto peakData = data->getData(RolePeakData).value<TPeakData>();
    auto intId = data->getData(RoleIntId).toInt();
    auto isExcluded = data->getData(RoleExcluded).toBool();
    auto shapeDescriptor = mModel->getShapeDescriptor(uid);
    auto title = data->getData(RoleTitle).toString();
    auto shape = mUidsShapes.value(uid, nullptr);
    if (!shape)
    {
      onEntityAdded(type, eId, data);
      return;
    }
    shape->setProperty(kOrderNum, peakData.orderNum);
    shape->setProperty(kPeakId, intId);

    auto brush = QBrush(mModel->getActualColor(uid));
    shape->setColor(mModel->getActualColor(uid));
    shape->setShapeSelectedBrush(brush);
    shape->setLabelText(title);

    shape->setPlateSelectedBrush(QColor(86, 185, 242, 51)); //color from figma
    shape->setInactive(isExcluded);
    shape->setLabelHidden(!mShowNames && !data->getData(RoleForceShowName).toBool());

    if (mModel->modelData(ModelRoleIsSpectral).toBool())
    {
      shape->setVisible(false);
      updateCurve();
    }
    break;
  }
  default:
    break;
  }
  queuedReplot();
}

void AnalysisPlotRefactor::onEntityAdded(EntityType type, TEntityUid eId, AbstractEntityDataModel::ConstDataPtr data)
{
  if (mIsInResetState)
    return;
  switch (type)
  {
  case TypeSample:
  {
    if (!mDisplayedItemsTypes.isEmpty() && !mDisplayedItemsTypes.contains(TypeSample))
      break;
    QFont font(Style::GetSASSValue("fontFaceNormal"), -1, Style::GetSASSValue("fontWeight").toInt());
    font.setPixelSize(Style::GetSASSValue("fontSizeSecondaryTextScalableFont").toInt());
    auto& uid = eId;
    auto shapeDescriptor = mModel->getShapeDescriptor(uid);
    auto passport = data->getData(RolePassport).value<TPassport>();
    auto intId = data->getData(RoleIntId).toInt();
    auto isExcluded = data->getData(RoleExcluded).toBool();
    auto title = data->getData(RoleTitle).toString();
    auto shape = new GPShapeWithLabelItem(this, mModel->currentEntityCoord(uid),
      shapeDescriptor.path,
      shapeDescriptor.pixelSize,
      title,
      font);
    shape->setProperty(kSampleId, intId);
    shape->setProperty(kEntityUid, uid);
    shape->setLayer("shapeLayer");
    shape->setSelectable(true);
    shape->setMargins(5, 5, 5, 5);
    shape->setLabelHidden(!mShowNames && !data->getData(RoleForceShowName).toBool());
    {
      shape->setShapePen(Qt::NoPen);
      shape->setShapeSelectedPen(Qt::NoPen);
      auto brush = QBrush(mModel->getActualColor(uid));
      shape->setColor(mModel->getActualColor(uid));
      shape->setShapeSelectedBrush(brush);
    }

    {
      shape->setPlatePen(Qt::NoPen);
      shape->setPlateSelectedPen(Qt::NoPen);
      shape->setPlateBrush(Qt::NoBrush);
      shape->setPlateSelectedBrush(QColor(86, 185, 242, 51)); //color from figma
    }

    auto plate = new SampleInfoPlate(this);
    plate->setLayer("passportLayer");
    plate->setSelectable(false);
    plate->topLeft->setParentAnchor(shape->right);
    plate->showParameters(false);

    QPointF identation = QPointF(5, shape->boundingRect().height() / 2 + 2);
    plate->topLeft->setCoords(identation);//identation from point

    plate->showParameters(mShowPassports || data->getData(RoleForceShowPassport).toBool());
    plate->SetParameter(Field, passport.value(PassportTags::field).toString());
    plate->SetParameter(Well, passport.value(PassportTags::well).toString());
    plate->SetParameter(Layer, passport.value(PassportTags::layer).toString());
    plate->SetParameter(Date, passport.value(PassportTags::date).toString());

    shape->setInactive(isExcluded);
    plate->SetIsInactive(isExcluded);

    mShapesPlates[shape] = plate;
    mUidsShapes[uid] = shape;
    break;
    queuedReplot();
  }
  case TypePeak:
  {
    if (!mDisplayedItemsTypes.isEmpty() && !mDisplayedItemsTypes.contains(TypePeak))
      break;
    auto font = QFont();
    font.setBold(true);
    auto& uid = eId;
    auto peakData = data->getData(RolePeakData).value<TPeakData>();
    auto intId = data->getData(RoleIntId).toInt();
    auto isExcluded = data->getData(RoleExcluded).toBool();
    auto shapeDescriptor = mModel->getShapeDescriptor(uid);
    auto title = data->getData(RoleTitle).toString();
    auto shape = new GPShapeWithLabelItem(this,
      mModel->currentEntityCoord(uid),
      shapeDescriptor.path,
      shapeDescriptor.pixelSize,
      title,
      font
    );
    shape->setProperty(kOrderNum, peakData.orderNum);
    shape->setProperty(kEntityUid, uid);
    shape->setProperty(kPeakId, intId);
    shape->setSelectable(true);
    shape->setMargins(5, 5, 5, 5);
    shape->setLabelHidden(!mShowNames && !data->getData(RoleForceShowName).toBool());
    {
      shape->setShapePen(Qt::NoPen);
      shape->setShapeSelectedPen(Qt::NoPen);
      auto brush = QBrush(mModel->getActualColor(uid));
      shape->setColor(mModel->getActualColor(uid));
      shape->setShapeSelectedBrush(brush);
    }
    {
      shape->setPlatePen(Qt::NoPen);
      shape->setPlateSelectedPen(Qt::NoPen);
      shape->setPlateBrush(Qt::NoBrush);
      shape->setPlateSelectedBrush(QColor(86, 185, 242, 51)); //color from figma
    }
    shape->setInactive(isExcluded);
    mUidsShapes[uid] = shape;
    if (mModel->modelData(ModelRoleIsSpectral).toBool())
    {
      shape->setVisible(false);
      updateCurve();
    }
    break;
  }
  default:
    break;
  }
  queuedReplot();
}

void AnalysisPlotRefactor::onEntityAboutToRemove(EntityType type, TEntityUid eId)
{
  if (mIsInResetState)
    return;
  switch (type)
  {
  case TypeSample:
  {
    if (!mDisplayedItemsTypes.isEmpty() && !mDisplayedItemsTypes.contains(TypeSample))
      break;
    auto shape = mUidsShapes.value(eId, nullptr);
    if (!shape)
      return;
    auto plate = mShapesPlates.value(shape, nullptr);//can be nullptr
    mUidsShapes.remove(eId);
    mShapesPlates.remove(shape);
    removeItem(shape);
    removeItem(plate);
    break;
  }
  case TypePeak:
  {
    if (!mDisplayedItemsTypes.isEmpty() && !mDisplayedItemsTypes.contains(TypePeak))
      break;
    auto shape = mUidsShapes.value(eId, nullptr);
    if (!shape)
      return;
    mUidsShapes.remove(eId);
    removeItem(shape);
    if (mModel->modelData(ModelRoleIsSpectral).toBool())
      updateCurve();
    break;
  }
  default:
    break;
  }
  queuedReplot();
}

void AnalysisPlotRefactor::onModelAboutToReset()
{
  mIsInResetState = true;
}

void AnalysisPlotRefactor::onModelReset()
{
  if (isVisible())
  {
    auto overlay = new WebOverlay(tr("Loading"), this);
    // qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
    connect(this, &AnalysisPlotRefactor::afterReplot, this,
      [overlay]()
      {
        overlay->deleteLater();
      }, Qt::SingleShotConnection);
  }
  mIsInResetState = false;
  mShapesPlates.clear();
  mUidsShapes.clear();
  clearItems();
  removeCurve(mCurve);
  if (!mModel)
    return;
  QFont font(Style::GetSASSValue("fontFaceNormal"), -1, Style::GetSASSValue("fontWeight").toInt());
  font.setPixelSize(Style::GetSASSValue("fontSizeSecondaryTextScalableFont").toInt());
  if (mDisplayedItemsTypes.isEmpty() || mDisplayedItemsTypes.contains(TypeSample))
  {
    for (auto& ent : mModel->getEntities(TypeSample))
    {
      auto uid = ent->getUid();
      auto shapeDescriptor = mModel->getShapeDescriptor(uid);
      auto passport = ent->getData(RolePassport).value<TPassport>();
      auto intId = ent->getData(RoleIntId).toInt();
      auto isExcluded = ent->getData(RoleExcluded).toBool();
      auto color = mModel->getActualColor(uid);
      auto title = ent->getData(RoleTitle).toString();
      auto shape = new GPShapeWithLabelItem(this, mModel->currentEntityCoord(uid),
        shapeDescriptor.path,
        shapeDescriptor.pixelSize,
        title,
        font);
      shape->setProperty(kSampleId, intId);
      shape->setProperty(kEntityUid, uid);
      shape->setLayer("shapeLayer");
      shape->setSelectable(true);
      shape->setMargins(5, 5, 5, 5);
      shape->setLabelHidden(!mShowNames && !ent->getData(RoleForceShowName).toBool());
      {
        shape->setShapePen(Qt::NoPen);
        shape->setShapeSelectedPen(Qt::NoPen);
        shape->setColor(color);
        shape->setShapeSelectedBrush(color);
      }

      {
        shape->setPlatePen(Qt::NoPen);
        shape->setPlateSelectedPen(Qt::NoPen);
        shape->setPlateBrush(Qt::NoBrush);
        shape->setPlateSelectedBrush(QColor(86, 185, 242, 51)); //color from figma
      }

      auto plate = new SampleInfoPlate(this);
      plate->setLayer("passportLayer");
      plate->setSelectable(false);
      plate->topLeft->setParentAnchor(shape->right);
      plate->showParameters(false);

      QPointF identation = QPointF(5, shape->boundingRect().height() / 2 + 2);
      plate->topLeft->setCoords(identation);//identation from point

      plate->showParameters(mShowPassports || ent->getData(RoleForceShowPassport).toBool());
      plate->SetParameter(Field, passport.value(PassportTags::field).toString());
      plate->SetParameter(Well, passport.value(PassportTags::well).toString());
      plate->SetParameter(Layer, passport.value(PassportTags::layer).toString());
      plate->SetParameter(Date, passport.value(PassportTags::date).toString());
      shape->setInactive(isExcluded);
      plate->SetIsInactive(isExcluded);
      mShapesPlates[shape] = plate;
      mUidsShapes[uid] = shape;
    }
  }
  if (mDisplayedItemsTypes.isEmpty() || mDisplayedItemsTypes.contains(TypePeak))
  {
    font = QFont();
    font.setBold(true);
    for (auto& ent : mModel->getEntities(TypePeak))
    {
      auto uid = ent->getUid();
      auto peakData = ent->getData(RolePeakData).value<TPeakData>();
      auto intId = ent->getData(RoleIntId).toInt();
      auto isExcluded = ent->getData(RoleExcluded).toBool();
      auto shapeDescriptor = mModel->getShapeDescriptor(uid);
      auto title = ent->getData(RoleTitle).toString();
      auto shape = new GPShapeWithLabelItem(this,
        mModel->currentEntityCoord(uid),
        shapeDescriptor.path,
        shapeDescriptor.pixelSize,
        title,
        font
      );
      shape->setProperty(kOrderNum, peakData.orderNum);
      shape->setProperty(kEntityUid, uid);
      shape->setProperty(kPeakId, intId);
      shape->setSelectable(true);
      shape->setMargins(5, 5, 5, 5);
      shape->setLabelHidden(!mShowNames && !ent->getData(RoleForceShowName).toBool());
      {
        shape->setShapePen(Qt::NoPen);
        shape->setShapeSelectedPen(Qt::NoPen);
        shape->setColor(mModel->getActualColor(uid));
        shape->setShapeSelectedBrush(mModel->getActualColor(uid));
      }
      {
        shape->setPlatePen(Qt::NoPen);
        shape->setPlateSelectedPen(Qt::NoPen);
        shape->setPlateBrush(Qt::NoBrush);
        shape->setPlateSelectedBrush(QColor(86, 185, 242, 51)); //color from figma
      }
      shape->setInactive(isExcluded);
      mUidsShapes[uid] = shape;
      if (mModel->modelData(ModelRoleIsSpectral).toBool())
        shape->setVisible(false);
    }

    if (mModel->modelData(ModelRoleIsSpectral).toBool())
    {
      mCurve = addCurve();
      auto renderType = mModel->modelData(ModelRoleSpectralRenderType).value<TSpectralRenderType>();
      const auto peaksList = mModel->getEntities(TypePeak);
      const auto uid = peaksList.first()->getUid();
      const auto shapeDesc = mModel->getShapeDescriptor(uid);
      const auto color = mModel->getActualColor(uid);
      mCurve->setScatterStyle(
        GPScatterStyle((renderType == AnalysisEntity::Dot)
          ? GPScatterStyle::ssDisc
          : GPScatterStyle::ssNone,
          color, shapeDesc.pixelSize));
      mCurve->setPen(color);
      mCurve->setLineStyle((renderType == AnalysisEntity::Dot)
        ? GPCurve::LineStyle::lsNone
        : GPCurve::LineStyle::lsLine);
      updateCurve();
    }
  }
  resize();
  queuedReplot();
}

void AnalysisPlotRefactor::updateCurve()
{
  if (!mCurve)
    return;
  auto entList = mModel->getEntities(TypePeak);
  QVector<double> keys;
  QVector<double> values;
  QVector<double> sortKeys;
  for (auto& ent : entList)
  {
    auto sortValue = ent->getData(RolePeakData).value<TPeakData>().orderNum;
    auto value = mModel->currentEntitySpectralValue(ent->getUid());
    sortKeys.append(sortValue);
    keys.append(sortValue);
    values.append(value);
  }
  mCurve->setData(sortKeys, keys, values);
  const auto color = mModel->getActualColor(entList.first()->getUid());
  mCurve->setPen(color);
}

void AnalysisPlotRefactor::updateShapes(EntityType type)
{
  auto entList = mModel->getEntities(type);
  for (auto& ent : entList)
  {
    auto uid = ent->getUid();
    if (mUidsShapes.contains(uid))
    {
      auto shape = mUidsShapes[uid];
      shape->SetPos(mModel->currentEntityCoord(uid));
    }
  }
}

void AnalysisPlotRefactor::updateShapes()
{
  for (const auto& [uid, shape] : mUidsShapes.asKeyValueRange())
    shape->SetPos(mModel->currentEntityCoord(uid));
}

void AnalysisPlotRefactor::mousePressEvent(QMouseEvent* event)
{
  if (event->modifiers().testFlag(Qt::ControlModifier))
  {
    setSelectionRectMode(GP::srmNone);
  }
  else
  {
    setSelectionRectMode(GP::srmSelect);
  }
  GraphicsPlot::mousePressEvent(event);
}

void AnalysisPlotRefactor::mouseReleaseEvent(QMouseEvent* event)
{
  if (selectionRectMode() != GP::srmSelect)
    setSelectionRectMode(GP::srmSelect);
  GraphicsPlot::mouseReleaseEvent(event);
  //@TODO: take code from CountsPlot::mouseReleaseEvent
}

void AnalysisPlotRefactor::mouseMoveEvent(QMouseEvent* event)
{
  handleHighlighting(event->pos());
  GraphicsPlot::mouseMoveEvent(event);
}

void AnalysisPlotRefactor::mouseDoubleClickEvent(QMouseEvent* event)
{
  resizeAxisToDefault();
  GraphicsPlot::mouseDoubleClickEvent(event);
}

void AnalysisPlotRefactor::contextMenuEvent(QContextMenuEvent* event)
{
  emit contextMenuRequested(event);
}

void AnalysisPlotRefactor::processRectSelection(const QRect& rect, QMouseEvent* event)
{
  bool selectionStateChanged = false;

  if (mInteractions.testFlag(GP::iSelectItems))
  {
    QList<GPAbstractItem*> selections;
    QRectF rectF(rect.normalized());
    if (GPAxisRect* affectedAxisRect = axisRectAt(rectF.topLeft()))
    {
      // determine plottables that were hit by the rect and thus are candidates for selection:
      for (const auto& item : mItems)
      {
        if (auto plate = dynamic_cast<GPShapeWithLabelItem*>(item))
        {
          if (rectF.intersects(plate->boundingRect()))
            selections.append(item);
        }
      }

      bool additive = event->modifiers().testFlag(mMultiSelectModifier);
      // deselect all other layerables if not additive selection:
      if (!additive)
      {
        // emit deselection except to those plottables who will be selected afterwards:
        for (const auto& item : selectedItems())
        {
          if (!selections.contains(item))
          {
            item->setSelected(false);
            selectionStateChanged = true;
          }
        }
      }

      // go through selections in reverse (largest selection first) and emit select events:
      for (auto& item : selections)
      {
        if (!item->selected())
        {
          item->setSelected(true);
          selectionStateChanged = true;
        }
      }
    }
  }

  if (selectionStateChanged)
  {
    emit selectionChangedByUser();
    queuedReplot();
  }
  else if (mSelectionRect)
  {
    mSelectionRect->layer()->replot();
  }
  else
  {
    queuedReplot();
  }
}

void AnalysisPlotRefactor::showEvent(QShowEvent* event)
{
  GraphicsPlot::showEvent(event);
  if (mDirty)
  {
    replot(rpQueuedReplot);
    mDirty = false;
  }
}
