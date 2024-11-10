#include "chromatogram_interval.h"
#include <ui/plots/chromatogram_plot.h>
#include "logic/markup/commands.h"
using namespace GenesisMarkup;

const QString kIntervalsLayerName = "intervalsLayer";
const QString kSelectedIntervalsLayerName = "selectedIntervalsLayer";

ChromatogramInterval::ChromatogramInterval(ChromatogramPlot *plot, double left, double right, QUuid uid)
  : GPLayerable(plot)
  , mParentPlot(plot)
  , mLeftBorder(left)
  , mRightBorder(right)
  , leftInterval(nullptr)
  , rightInterval(nullptr)
  , mUid(uid)
{
   auto color = QColor(86, 185, 242, 1);
   mCustomPen = QPen(color);
   mCustomPen.setStyle(Qt::DashLine);

   color.setAlphaF(0.2);
   mCustomBrush = QBrush(color);

   color.setAlphaF(0.5);
   mSelectedCustomBrush = QBrush(color);

   color = QColor(0, 32, 51, 1);
   mLockedPen = QPen(color);
   mLockedPen.setStyle(Qt::DashLine);

   color.setAlphaF(0.2);
   mLockedBrush = QBrush(color);

   color.setAlphaF(0.5);
   mSelectedLockedBrush = QBrush(color);


  if(!plot->layer(kIntervalsLayerName))
  {
    plot->addLayer(kIntervalsLayerName);
    plot->layer(kIntervalsLayerName)->setMode(GPLayer::lmLogical);
  }
  if(!plot->layer(kSelectedIntervalsLayerName))
  {
    plot->addLayer(kSelectedIntervalsLayerName);
    plot->layer(kSelectedIntervalsLayerName)->setMode(GPLayer::lmBuffered);
  }
  setLayer(kIntervalsLayerName);
  updateGeometry();
  updateInteractivityFlags(plot);
}

void ChromatogramInterval::updateGeometry()
{
  mPolygon.clear();
}


void ChromatogramInterval::updateInteractivityFlags(ChromatogramPlot *plot)
{
  if(!plot || !plot->model())
    return;
  auto model = plot->model();
  auto entity = model->getEntity(mUid);
  IntervalTypes type = ITNone;
  {
    auto iter = entity->constFind(ChromatogrammEntityDataRoles::IntervalType);
    if(iter != entity->constEnd() && iter.value().isValid())
      type = (IntervalTypes)iter.value().toInt();
  }
  updateInteractivityFlags(plot, type);
}

void ChromatogramInterval::updateInteractivityFlags(ChromatogramPlot *plot, IntervalTypes newIntervalType)
{
  if(!plot)
    return;
  mBorderMoveAllowed = false;
  auto interactions = plot->stepInteractions();
  bool isMaster = plot->isMaster();
  if(isMaster)
  {
    mBorderMoveAllowed = interactions.masterInteractions.testFlag(SIMModifyMarkupIntervals);
  }
  else
  {
    mBorderMoveAllowed = interactions.slaveInteractions.testFlag(SISModifyMarkupIntervals);
  }
}

void ChromatogramInterval::draw(GPPainter *painter)
{
  painter->save();
  double xLeft = parentPlot()->xAxis->coordToPixel(mLeftBorder),
      xRight = parentPlot()->xAxis->coordToPixel(mRightBorder),
      yTop = parentPlot()->yAxis->range().upper;

  painter->setPen(Qt::NoPen);
  switch ((IntervalTypes)mIntervalType) {
    case ITNone:
      break;
    case ITCustomParameters:
      if(isSelected)
      {
        painter->setBrush(mSelectedCustomBrush);
      }
      else
      {
        painter->setBrush(mCustomBrush);
      }
      break;

    case ITMarkupDeprecation:
      if(isSelected)
      {
        painter->setBrush(mSelectedLockedBrush);
      }
      else
      {
        painter->setBrush(mLockedBrush);

      }
      break;
  }


  mPolygon.clear();
  mPolygon << QPointF(mLeftBorder, 0);
  mPolygon << QPointF(mRightBorder, 0);
  mPolygon << QPointF(mRightBorder, yTop);
  mPolygon << QPointF(mLeftBorder, yTop);

  QPolygonF p;
  for(auto& pt : mPolygon)
    p.append({parentPlot()->xAxis->coordToPixel(pt.x()), parentPlot()->yAxis->coordToPixel(pt.y())});

  painter->drawPolygon(p);

  if(isSelected)
  {
    yTop = parentPlot()->xAxis->coordToPixel(yTop);
    QLineF leftLine(xLeft, yTop, xLeft, 0);
    QLineF rightLine(xRight, yTop, xRight, 0);
    if((IntervalTypes)mIntervalType == ITCustomParameters)
    {
      painter->setPen(mCustomPen);
    }
    else
    {
      painter->setPen(mLockedPen);
    }
    painter->drawLine(leftLine);
    painter->drawLine(rightLine);

  }
//  painter->setPen(mLabelPen);
//  auto textSize = mLabel.size();
//  if(yCenter - textSize.height() / 2.0 >= yCenterOnBase || yCenterOnBase >= yCenter + textSize.height())
//  {
//    QLineF centerLine(xCenter, yCenter - textSize.height() / 2.0, xCenter, yCenterOnBase);
//    if(yCenter <= yCenterOnBase)
//      centerLine.setP1({xCenter, yCenter + textSize.height() / 2.0});
//    painter->drawLine(centerLine);
//  }
//  painter->drawStaticText(QPoint(xCenter, yCenter) -
//                          QPoint(textSize.width() / 2.0, textSize.height() / 2.0),
//                          mLabel);
  painter->restore();
}

Qt::CursorShape ChromatogramInterval::HitTest(QMouseEvent *event, unsigned *part)
{
  if (part)
    *part = HitPartNone;
  if (!isSelected || !mBorderMoveAllowed)
    return Qt::ArrowCursor;
  double mousePos = event->pos().x();

  double leftPos = parentPlot()->xAxis->coordToPixel(mLeftBorder);
  if (fabs(leftPos - mousePos) < 3)
  {
    if (part)
      *part = HitPartStart;
    return Qt::SplitHCursor;
  }

  double rightPos = parentPlot()->xAxis->coordToPixel(mRightBorder);
  if (fabs(rightPos - mousePos) < 3)
  {
    if (part)
      *part = HitPartEnd;
    return Qt::SplitHCursor;
  }
  return Qt::ArrowCursor;
}

double ChromatogramInterval::selectTest(const QPointF &pos, bool onlySelectable, QVariant *details) const
{
  double mousePos = pos.x();
  GenesisMarkup::ChromatogrammModes mMode = (GenesisMarkup::ChromatogrammModes)mParentPlot->model()->getChromatogramValue(ChromatogrammMode).toInt();
  if(mMode != GenesisMarkup::CMIntervals && onlySelectable)
  {
    return -1.;
  }
/*------for proper HitTest working----------------------------------------*/
/**/  double leftPos = parentPlot()->xAxis->coordToPixel(mLeftBorder);   //|
/**/  if (fabs(leftPos - mousePos) < 3)                                  //|
/**/    return 0;                                                        //|
/**/                                                                     //|
/**/  double rightPos = parentPlot()->xAxis->coordToPixel(mRightBorder); //|
/**/  if (fabs(rightPos - mousePos) < 3)                                 //|
/**/    return 0;                                                        //|
/*-----for proper HitTest working-----------------------------------------*/

  QPointF mappedPos(parentPlot()->xAxis->pixelToCoord(pos.x()), parentPlot()->yAxis->pixelToCoord(pos.y()));

  if(mPolygon.containsPoint(mappedPos, Qt::OddEvenFill))
    return 0;
  else
    return -1.;
}

void ChromatogramInterval::selectEvent(QMouseEvent *event, bool additive, const QVariant &details, bool *selectionStateChanged)
{
  if(additive)
  {
    setIsSelected(!isSelected);
    if(isSelected)
      setLayer(kSelectedIntervalsLayerName);
    else
      setLayer(kIntervalsLayerName);

    if(selectionStateChanged)
      *selectionStateChanged = true;
  }
  else
  {
    if(!isSelected)
    {
      setIsSelected(true);
      setLayer(kSelectedIntervalsLayerName);
      if(selectionStateChanged)
        *selectionStateChanged = true;
    }
  }
}

void ChromatogramInterval::deselectEvent(bool *selectionStateChanged)
{
  if(isSelected)
  {
    setIsSelected(false);
    setLayer(kIntervalsLayerName);
    if(selectionStateChanged)
      *selectionStateChanged = true;
  }
}

double ChromatogramInterval::leftBorder() const
{
  return mLeftBorder;
}

double ChromatogramInterval::rightBorder() const
{
  return mRightBorder;
}

void ChromatogramInterval::setLeftBorder(double newLeftBorder)
{
  mLeftBorder = newLeftBorder;
  updateGeometry();
}

void ChromatogramInterval::setRightBorder(double newRightBorder)
{
  mRightBorder = newRightBorder;
  updateGeometry();
}

bool ChromatogramInterval::getIsSelected() const
{
  return isSelected;
}

void ChromatogramInterval::setIsSelected(bool newIsSelected)
{
  if (isSelected == newIsSelected)
    return;
  isSelected = newIsSelected;
  emit isSelectedChanged();
}

void ChromatogramInterval::resetIsSelected()
{
  setIsSelected(false);
}

void ChromatogramInterval::mousePressEvent(QMouseEvent *event, const QVariant &details)
{
//  if(event->modifiers().testFlag(Qt::ShiftModifier))
//  {
//    if(mSetUnsetMarkerAllowed)
//    {
//      auto cmd = new SetPeakTypeCommand(mParentPlot->model(), mUid, mPeakType, (mPeakType == PTMarker ? PTPeak : PTMarker));
//      emit newCommand(cmd);
//      event->accept();
//    }
//    else
//      if(mSetUnsetInterMarkerAllowed && mPeakType != PTMarker)
//    {
//      //TODO: should cause intermarkers renumeration and recoloring inside near markers pair
//      auto cmd = new SetPeakTypeCommand(mParentPlot->model(), mUid, mPeakType, (mPeakType == PTInterMarker ? PTPeak : PTInterMarker));
//      emit newCommand(cmd);
//      event->accept();
//    }
//    else
//      return GPLayerable::mousePressEvent(event, details);
//  }

  if(!isSelected || !mBorderMoveAllowed)
    return GPLayerable::mousePressEvent(event, details);

  HitTest(event, &mDragPart);
  if(mDragPart != HitPartNone)
  {
    mMovedBorderOriginalValue = mDragPart == HitPartStart ? mLeftBorder : mRightBorder;
    event->accept();
  }
  else
    return GPLayerable::mousePressEvent(event, details);
}

void ChromatogramInterval::mouseMoveEvent(QMouseEvent *event, const QPointF &startPos)
{
  if(mDragPart == HitPartNone)
    return GPLayerable::mouseMoveEvent(event, startPos);
  event->accept();
  switch(mDragPart)
  {
  case HitPartStart:
  {
    auto mappedX = parentPlot()->xAxis->pixelToCoord(event->pos().x());
    if(leftInterval && mappedX < leftInterval->mRightBorder)
      mappedX = leftInterval->mRightBorder;
    if(mappedX > mRightBorder)
      mappedX = mRightBorder;
    setLeftBorder(mappedX);
    mLayer->replot();
    break;
  }
  case HitPartEnd:
  {
    auto mappedX = parentPlot()->xAxis->pixelToCoord(event->pos().x());
    if(rightInterval && mappedX > rightInterval->mLeftBorder)
      mappedX = rightInterval->mLeftBorder;
    if(mappedX < mLeftBorder)
      mappedX = mLeftBorder;
    setRightBorder(mappedX);
    mLayer->replot();
    break;
  }
  default://should never be here
    break;
  }
}

void ChromatogramInterval::mouseReleaseEvent(QMouseEvent *event, const QPointF &startPos)
{
  if(mDragPart != HitPartNone)
  {
    switch(mDragPart)
    {
    case HitPartStart:
    {
        ///@todo написать комманду
      auto cmd = new MoveIntervalBorderCommand(mParentPlot->model(), mUid, mMovedBorderOriginalValue, mLeftBorder, true);
      emit newCommand(cmd);
      break;
    }
    case HitPartEnd:
    {
        ///@todo написать комманду
      auto cmd = new MoveIntervalBorderCommand(mParentPlot->model(), mUid, mMovedBorderOriginalValue, mRightBorder, false);
      emit newCommand(cmd);
      break;
    }
    }

    mDragPart = HitPartNone;
    event->accept();
  }
  else
  {
    GPLayerable::mouseReleaseEvent(event, startPos);
  }
}

IntervalTypes ChromatogramInterval::intervalType() const
{
  return mIntervalType;
}

void ChromatogramInterval::setIntervalType(IntervalTypes newIntervalType)
{
  mIntervalType = newIntervalType;
}

ChromatogramInterval *ChromatogramInterval::getLeftInterval() const
{
  return leftInterval;
}

void ChromatogramInterval::setLeftInterval(ChromatogramInterval *newLeftInterval)
{
  leftInterval = newLeftInterval;
}

ChromatogramInterval *ChromatogramInterval::getRightInterval() const
{
  return rightInterval;
}

void ChromatogramInterval::setRightInterval(ChromatogramInterval *newRightInterval)
{
  rightInterval = newRightInterval;
}


QRect GenesisMarkup::ChromatogramInterval::clipRect() const
{
  auto left = parentPlot()->xAxis->coordToPixel(parentPlot()->xAxis->range().lower);
  auto top = parentPlot()->yAxis->coordToPixel(parentPlot()->yAxis->range().upper);
  return QRect(left,
               top,
               parentPlot()->xAxis->coordToPixel(parentPlot()->xAxis->range().upper) - left,
               parentPlot()->yAxis->coordToPixel(parentPlot()->yAxis->range().lower) - top);
}
