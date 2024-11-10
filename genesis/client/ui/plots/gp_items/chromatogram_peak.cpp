#include "chromatogram_peak.h"
//#include "../../peak_interface.h"
#include "chromatogram_baseline.h"
#include <ui/plots/chromatogram_plot.h>
#include <logic/markup/commands.h>
using namespace GenesisMarkup;

const QString kPeaksLayerName = "peaksLayer";
const QString kSelectedPeaksLayerName = "selectedPeaksLayer";

ChromatogramPeak::ChromatogramPeak(ChromatogramPlot *plot, const GPGraph &graph, double left, double right, QUuid uid)
  : GPLayerable(plot)
  , mCurve(&graph)
  , mBaseLine(nullptr)
  , mParentPlot(plot)
  , mLeftBorder(left)
  , mRightBorder(right)
  , mPen(Qt::gray)
  , mSelectedPen(Qt::darkGray)
  , mBrush(Qt::lightGray)
  , mSelectedBrush(Qt::gray)
  , mBorderLinesPen(Qt::darkGray)
  , mLabelPen(QColor(0, 32, 51, 200))
  , mLabel("X")
  , leftPeak(nullptr)
  , rightPeak(nullptr)
  , mUid(uid)

{
  if(!plot->layer(kPeaksLayerName))
  {
    plot->addLayer(kPeaksLayerName);
    plot->layer(kPeaksLayerName)->setMode(GPLayer::lmLogical);
  }
  if(!plot->layer(kSelectedPeaksLayerName))
  {
    plot->addLayer(kSelectedPeaksLayerName);
    plot->layer(kSelectedPeaksLayerName)->setMode(GPLayer::lmBuffered);
  }
  setLayer(kPeaksLayerName);
  createCurveCut(mCurve);
  createBaseLineCut(mBaseLine);
  updateGeometry();
  if(plot->model()->getEntity(uid))
    updateInteractivityFlags(plot);
}

void ChromatogramPeak::updateGeometry()
{
  mPolygon.clear();
  mPolygon << mCurveCut;
  //base line data inverted here cause we have clock-wise path with start in first curve point
  //and end in firs base line point
  if(mBaseLineCut.isEmpty())
  {
    mPolygon << QPointF(mRightBorder, 0);
    mPolygon << QPointF(mLeftBorder, 0);
  }
  else
  {
    for(auto iter = mBaseLineCut.rbegin(); iter != mBaseLineCut.rend(); iter++)
      mPolygon << *iter;
  }
}

void ChromatogramPeak::setBaseLine(const ChromatogramBaseline *baseLine)
{
  if(mBaseLine)
    disconnect(mBaseLine, nullptr, this, nullptr);

  mBaseLine = baseLine;
  if(mBaseLine)
  {
    connect(baseLine, &ChromatogramBaseline::moved, this, [this]()
    {
      createBaseLineCut(mBaseLine);
      updateGeometry();
    });
    connect(baseLine, &QObject::destroyed, this, [this]()
    {
      mBaseLine = nullptr;
    });
  }
  createBaseLineCut(mBaseLine);
  updateGeometry();
}

double ChromatogramPeak::lerp(const double &x1, const double &y1, const double &x2, const double &y2, const double &x)
{
  return  y2 + ((y1 - y2)/(x1 - x2))*(x - x2);
}

double ChromatogramPeak::lerp(const QPointF &p1, const QPointF &p2, const double &x)
{
  return lerp(p1.x(), p1.y(), p2.x(), p2.y(), x);
}

double ChromatogramPeak::lerp(double val, const QList<GPGraphData>::const_iterator &iter, QSharedPointer<GPDataContainer<GPGraphData> > data, double eps)
{
  //will move input val to border of data range if it needed
  double lerpd;
  // will see
//  Q_ASSERT(iter != data->constBegin());//really bad error
  if(iter == data->constBegin())
  {
    lerpd = iter->value;
    val = iter->key;
  }
  else
  {
    double y1, y2, x1, x2;
    y1 = (iter - 1)->value;
    y2 = iter->value;
    x1 = (iter - 1)->key;
    x2 = iter->key;
    if((fabs(x2 - x1) + fabs(y2 - x1)) < eps)
      lerpd = y2;
    else
      lerpd = lerp(x1,y1,x2,y2,val);
  }
  return lerpd;
}

double ChromatogramPeak::lerp(double val, const QList<GPCurveData>::const_iterator &iter, QSharedPointer<GPDataContainer<GPCurveData> > data, double eps)
{
  //will move input val to border of data range if it needed
  double lerpd;
  if(iter == data->constBegin())
  {
    // Q_ASSERT(false);//really bad error
    lerpd = iter->value;
    val = iter->key;
  }
  else
  {
    double y1, y2, x1, x2;
    y1 = (iter - 1)->value;
    y2 = iter->value;
    x1 = (iter - 1)->key;
    x2 = iter->key;
    if((fabs(x2 - x1) + fabs(y2 - x1)) < eps)
      lerpd = y2;
    else
      lerpd = lerp(x1,y1,x2,y2,val);
  }
  return lerpd;
}



QPair<double, double> ChromatogramPeak::interpolateBaseLine(GPCurve* baselineCurve,
                                                            double leftPeakXPoint,
                                                            double rightPeakXPoint) const
{

  if (!baselineCurve){
    return {};
  }

  auto iPol = [this](GPCurve* curve, double x)-> double {

    if (!curve){
      return -1;
    }

    double x1 = 0, x2 = 0, y1 = 0, y2 = 0;
    bool foundR = false;
    auto yValTest = curve->getValueRange(foundR, GP::SignDomain::sdBoth, GPRange(x, x));
    if (foundR){
      return yValTest.lower;
    }
    auto xValTest = curve->getKeyRange(foundR);
    if (x > xValTest.upper || x < xValTest.lower){
      return -1;
    }

    auto leftXPointPos = curve->findBegin(x, true) -1;
    auto rightXPointPos = curve->findBegin(x, false) -1;

    if (!leftXPointPos || !leftXPointPos){
      return -1;
    }

    x1 = curve->data()->at(leftXPointPos)->key;
    y1 = curve->data()->at(leftXPointPos)->value;

    x2 = curve->data()->at(rightXPointPos)->key;
    y2 = curve->data()->at(rightXPointPos)->value;

    return lerp(x1, y1, x2, y2, x);
  };



  auto leftValueY = iPol(baselineCurve, leftPeakXPoint);
  auto RightValueY= iPol(baselineCurve, rightPeakXPoint);

  if (leftValueY == -1 || RightValueY == -1){
    return {-1, -1};
  }

  return {leftValueY, RightValueY};
}

void ChromatogramPeak::createBaseLineCut(const ChromatogramBaseline *baseLine)
{
  mBaseLineCut.clear();
  auto fillVoid = [this]()
  {
    mBaseLineCut << QPointF(mLeftBorder, 0);
    mBaseLineCut << QPointF(mRightBorder, 0);
  };
  if(!baseLine)
    return fillVoid();
  bool foundRange = false;
  auto range = baseLine->getKeyRange(foundRange);
  if(!foundRange || range.lower > mLeftBorder || range.upper < mRightBorder)
    return fillVoid();
  auto data = baseLine->data();
  double eps = 0.001;

  auto leftBound = std::lower_bound(data->constBegin(), data->constEnd(), mLeftBorder,
                                    [](const GPCurveData &elem, const double& value)->bool{return elem.key < value;});

  auto rightBound = std::lower_bound(data->constBegin(), data->constEnd(), mRightBorder,
                                     [](const GPCurveData &elem, const double& value)->bool{return elem.key < value;});

  if(leftBound == data->constEnd())
    return fillVoid();//no curve cut for this;

  if(rightBound == data->constEnd())
  {
    rightBound--;
    mRightBorder = rightBound->key;
  }

  if(leftBound == data->constBegin())
  {
    mLeftBorder = leftBound->key;
  }

  double lval, rval;

  lval = lerp(mLeftBorder, leftBound, data, eps);
  rval = lerp(mRightBorder, rightBound, data, eps);

  mBaseLineCut.append({mLeftBorder, lval});
  for(auto iter = leftBound; iter < rightBound; iter++)
    mBaseLineCut.append({iter->key, iter->value});
  mBaseLineCut.append({mRightBorder, rval});
}

void ChromatogramPeak::createCurveCut(const GPGraph *Curve)
{
  mCurveCut.clear();
  if(!Curve)
    return;
  bool foundRange = false;
  auto range = Curve->getKeyRange(foundRange);
  if(!foundRange || range.lower > mLeftBorder || range.upper < mRightBorder)
    return;
  auto data = Curve->data();
  double eps = 0.001;

  auto leftBound = std::lower_bound(data->constBegin(), data->constEnd(), mLeftBorder,
                                    [](const GPGraphData &elem, const double& value)->bool{return elem.key < value;});

  auto rightBound = std::lower_bound(data->constBegin(), data->constEnd(), mRightBorder,
                                     [](const GPGraphData &elem, const double& value)->bool{return elem.key < value;});

  if(leftBound == data->constEnd())
    return;//no curve cut for this;
  if(rightBound == data->constEnd())
  {
    rightBound--;
    mRightBorder = rightBound->key;
  }
  if(leftBound == data->constBegin())
  {
    mLeftBorder = leftBound->key;
  }

  double lval, rval;

  lval = lerp(mLeftBorder, leftBound, data, eps);
  rval = lerp(mRightBorder, rightBound, data, eps);

  mCurveCut.append({mLeftBorder, lval});
  for(auto iter = leftBound; iter < rightBound; iter++)
    mCurveCut.append({iter->key, iter->value});
  mCurveCut.append({mRightBorder, rval});
  mPeakCenter = QPointF();
  for(auto& pt : mCurveCut)
  {
    if(pt.y() > mPeakCenter.y())
      mPeakCenter = pt;
  }
}

void ChromatogramPeak::draw(GPPainter *painter)
{
  painter->save();
  if(isSelected)
  {
    painter->setPen(mSelectedPen);
    painter->setBrush(mSelectedBrush);
  }
  else
  {
    painter->setPen(mPen);
    painter->setBrush(mBrush);
  }
  QPolygonF p;
  for(auto& pt : mPolygon)
    p.append({parentPlot()->xAxis->coordToPixel(pt.x()), parentPlot()->yAxis->coordToPixel(pt.y()) - 1});
  painter->drawPolygon(p);

  double xLeft = parentPlot()->xAxis->coordToPixel(mLeftBorder),
      xRight = parentPlot()->xAxis->coordToPixel(mRightBorder),
      yTop = parentPlot()->yAxis->coordToPixel(parentPlot()->yAxis->range().upper),
      yCenterOnBase,
      yCenter = (yTop + parentPlot()->yAxis->coordToPixel(parentPlot()->yAxis->range().lower)) / 2.0,
      xCenter = parentPlot()->xAxis->coordToPixel(mPeakCenter.x());
  {
    auto baseLineCenterInterpolated = std::lower_bound(mBaseLineCut.begin(), mBaseLineCut.end(),
                                                       mPeakCenter.x(),
                                                       [](const QPointF &elem, const double &value)->bool{return elem.x() < value;});
    if(baseLineCenterInterpolated == mBaseLineCut.begin())
    {
      auto secondPoint = baseLineCenterInterpolated + 1;
      yCenterOnBase = parentPlot()->yAxis->coordToPixel(lerp(*baseLineCenterInterpolated, *secondPoint, mPeakCenter.x()));
    }
    else /*if(baseLineCenterInterpolated == mBaseLineCut.end() - 1)*/
    {
      auto firstPoint = baseLineCenterInterpolated - 1;
      yCenterOnBase = parentPlot()->yAxis->coordToPixel(lerp(*firstPoint, *baseLineCenterInterpolated, mPeakCenter.x()));
    }
  }
  if(isSelected)
  {
    QLineF leftLine(xLeft, yTop, xLeft, parentPlot()->yAxis->coordToPixel(mBaseLineCut.first().y()));
    QLineF rightLine(xRight, yTop, xRight, parentPlot()->yAxis->coordToPixel(mBaseLineCut.last().y()));

    painter->setPen(mBorderLinesPen);
    painter->drawLine(leftLine);
    painter->drawLine(rightLine);
  }

  painter->setPen(mLabelPen);
  auto textSize = mLabel.size();
  if(yCenter - textSize.height() / 2.0 >= yCenterOnBase || yCenterOnBase >= yCenter + textSize.height())
  {
    QLineF centerLine(xCenter, yCenter - textSize.height() / 2.0, xCenter, yCenterOnBase);
    if(yCenter <= yCenterOnBase)
      centerLine.setP1({xCenter, yCenter + textSize.height() / 2.0});
    painter->drawLine(centerLine);
  }
  painter->drawStaticText(QPoint(xCenter, yCenter) -
                          QPoint(textSize.width() / 2.0, textSize.height() / 2.0),
                          mLabel);
  painter->restore();
}

Qt::CursorShape ChromatogramPeak::HitTest(QMouseEvent *event, unsigned *part)
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

double ChromatogramPeak::selectTest(const QPointF &pos, bool onlySelectable, QVariant *details) const
{
  double mousePos = pos.x();

  GenesisMarkup::ChromatogrammModes mMode = (GenesisMarkup::ChromatogrammModes)mParentPlot->model()->getChromatogramValue(ChromatogrammMode).toInt();
  if(mMode != GenesisMarkup::CMMarkup)
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

void ChromatogramPeak::selectEvent(QMouseEvent *event, bool additive, const QVariant &details, bool *selectionStateChanged)
{

  GenesisMarkup::ChromatogrammModes mMode = (GenesisMarkup::ChromatogrammModes)mParentPlot->model()->getChromatogramValue(ChromatogrammMode).toInt();
  if(mMode == GenesisMarkup::CMMarkup)
  {
    if(additive)
    {
      setIsSelected(!isSelected);
      if(isSelected)
        setLayer(kSelectedPeaksLayerName);
      else
        setLayer(kPeaksLayerName);

      if(selectionStateChanged)
        *selectionStateChanged = true;
    }
    else
    {
      if(!isSelected)
      {
        setIsSelected(true);
        setLayer(kSelectedPeaksLayerName);
        if(selectionStateChanged)
          *selectionStateChanged = true;
      }
    }
  }
}

void ChromatogramPeak::deselectEvent(bool *selectionStateChanged)
{
  if(isSelected)
  {
    setIsSelected(false);
    setLayer(kPeaksLayerName);
    if(selectionStateChanged)
      *selectionStateChanged = true;
  }
}

double ChromatogramPeak::leftBorder() const
{
  return mLeftBorder;
}

void ChromatogramPeak::setLeftBorder(double newLeftBorder)
{
  if(leftPeak && newLeftBorder < leftPeak->mRightBorder)
    newLeftBorder = leftPeak->mRightBorder;
  if(newLeftBorder > mRightBorder)
    newLeftBorder = mRightBorder;
  mLeftBorder = newLeftBorder;
  createCurveCut(mCurve);
  createBaseLineCut(mBaseLine);
  updateGeometry();
}

double ChromatogramPeak::rightBorder() const
{
  return mRightBorder;
}

void ChromatogramPeak::setRightBorder(double newRightBorder)
{
  if(rightPeak && newRightBorder > rightPeak->mLeftBorder)
    newRightBorder = rightPeak->mLeftBorder;
  if(newRightBorder < mLeftBorder)
    newRightBorder = mLeftBorder;
  mRightBorder = newRightBorder;
  createCurveCut(mCurve);
  createBaseLineCut(mBaseLine);
  updateGeometry();
}

void ChromatogramPeak::setPen(const QPen &newPen)
{
  mPen = newPen;
}

void ChromatogramPeak::setSelectedPen(const QPen &newSelectedPen)
{
  mSelectedPen = newSelectedPen;
}

void ChromatogramPeak::setBrush(const QBrush &newBrush)
{
  mBrush = newBrush;
}

void ChromatogramPeak::setSelectedBrush(const QBrush &newSelectedBrush)
{
  mSelectedBrush = newSelectedBrush;
}

void ChromatogramPeak::setBorderLinesPen(const QPen &newBorderLinesPen)
{
  mBorderLinesPen = newBorderLinesPen;
}

void ChromatogramPeak::setLabelPen(const QPen &newLabelPen)
{
  mLabelPen = newLabelPen;
}

void ChromatogramPeak::setLabel(const QString &newLabel)
{
  mLabel.setText(newLabel);
  mLabel.prepare();
}

ChromatogramPeak *ChromatogramPeak::getRightPeak() const
{
  return rightPeak;
}

void ChromatogramPeak::setRightPeak(ChromatogramPeak *newRightPeak)
{
  rightPeak = newRightPeak;
}

int ChromatogramPeak::peakId() const
{
  return mPeakId;
}

void ChromatogramPeak::setPeakId(int newPeakId)
{
    mPeakId = newPeakId;
}

const QUuid &ChromatogramPeak::uid() const
{
    return mUid;
}

QPointF ChromatogramPeak::peakCenter() const
{
  return mPeakCenter;
}

GenesisMarkup::PeakTypes ChromatogramPeak::peakType() const
{
  return mPeakType;
}

void ChromatogramPeak::setPeakType(GenesisMarkup::PeakTypes newPeakType)
{
  mPeakType = newPeakType;
//  switch(mPeakType)
//  {
//  case PTPeak:
//    setBrush(Qt::lightGray);
//    setPen(QPen(Qt::gray));
//    setSelectedBrush(Qt::gray);
//    setSelectedPen(QPen(Qt::darkGray));
//BUG #ReplotBug Calls replot HUNDERDS TIMES FOR EVERY PLOT WHEN INIT WITH PEAKS (5667 calls)
//    layer()->replot();
//    break;
//  case PTMarker:
//    setBrush(QColor("blue"));
//    setPen(QColor("blue").darker(120));
//    setSelectedBrush(QColor("blue").darker(120));
//    setSelectedPen(QColor("blue").darker(140));
//    layer()->replot();
//    break;
//  case PTInterMarker:
//  case PTNone:
//  case PTFake:
//  default:
//    break;
//  }
}

void ChromatogramPeak::updateInteractivityFlags(ChromatogramPlot *plot)
{
  if(!plot || !plot->model())
    return;
  auto model = plot->model();
  auto entity = model->getEntity(mUid);
  PeakTypes type = PTPeak;
  {
    auto iter = entity->constFind(ChromatogrammEntityDataRoles::PeakType);
    if(iter != entity->constEnd() && iter.value().isValid())
      type = (PeakTypes)iter.value().toInt();
  }
  updateInteractivityFlags(plot, type);
}

void ChromatogramPeak::updateInteractivityFlags(ChromatogramPlot *plot, PeakTypes newPeakType)
{
  if(!plot)
    return;
  mSetUnsetMarkerAllowed = false;
  mBorderMoveAllowed = false;
  auto interactions = plot->stepInteractions();
  bool isMaster = plot->isMaster();
  if(isMaster)
  {
    mSetUnsetMarkerAllowed = interactions.masterInteractions.testFlag(SIMSetUnsetMarker);
    mSetUnsetInterMarkerAllowed = interactions.masterInteractions.testFlag(SIMSetUnsetInterMarker);
    switch(newPeakType)
    {
    case PTPeak:
      mBorderMoveAllowed = interactions.masterInteractions.testFlag(SIMModifyPeaks);
      break;
    case PTMarker:
      mBorderMoveAllowed = interactions.masterInteractions.testFlag(SIMModifyMarkers);
      break;
    case PTInterMarker:
      mBorderMoveAllowed = interactions.masterInteractions.testFlag(SIMModifyInterMarkers);
      break;
    case PTNone:
    case PTFake:
    default:
      break;
    }
  }
  else
  {
    mSetUnsetMarkerAllowed = interactions.slaveInteractions.testFlag(SISSetUnsetMarker);
    mSetUnsetInterMarkerAllowed = false;//interactions.slaveInteractions.testFlag(SISSetUnsetInterMarker);
    switch(newPeakType)
    {
    case PTPeak:
      mBorderMoveAllowed = interactions.slaveInteractions.testFlag(SISModifyPeaks);
      break;
    case PTMarker:
      mBorderMoveAllowed = interactions.slaveInteractions.testFlag(SISModifyMarkers);
      break;
    case PTInterMarker:
      mBorderMoveAllowed = interactions.slaveInteractions.testFlag(SISModifyInterMarkers);
      break;
    case PTNone:
    case PTFake:
    default:
      break;
    }
  }
}

bool ChromatogramPeak::getIsSelected() const
{
  return isSelected;
}

void ChromatogramPeak::setIsSelected(bool newIsSelected)
{
  if (isSelected == newIsSelected)
    return;
  isSelected = newIsSelected;
  emit isSelectedChanged();
}

void ChromatogramPeak::resetIsSelected()
{
  setIsSelected(false); // TODO: Adapt to use your actual default value
}

ChromatogramPeak *ChromatogramPeak::getLeftPeak() const
{
  return leftPeak;
}

void ChromatogramPeak::setLeftPeak(ChromatogramPeak *newLeftPeak)
{
  leftPeak = newLeftPeak;
}

//void GenesisMarkup::ChromatogramPeak::mousePressEvent(QMouseEvent *event, const QVariant &details)
//{
//  qDebug() << Q_FUNC_INFO;
//  event->accept();

//}


QRect GenesisMarkup::ChromatogramPeak::clipRect() const
{
  auto left = parentPlot()->xAxis->coordToPixel(parentPlot()->xAxis->range().lower);
  auto top = parentPlot()->yAxis->coordToPixel(parentPlot()->yAxis->range().upper);
  return QRect(left,
               top,
               parentPlot()->xAxis->coordToPixel(parentPlot()->xAxis->range().upper) - left,
               parentPlot()->yAxis->coordToPixel(parentPlot()->yAxis->range().lower) - top);

}


void GenesisMarkup::ChromatogramPeak::mousePressEvent(QMouseEvent *event, const QVariant &details)
{
  if(event->button() == Qt::LeftButton)
  {
    if(event->modifiers().testFlag(Qt::ShiftModifier))
    {
      if(mSetUnsetMarkerAllowed)
      {
        auto cmd = new SetPeakTypeCommand(mParentPlot->model(), mUid, mPeakType, (mPeakType == PTMarker ? PTPeak : PTMarker));
        emit newCommand(cmd);
        event->accept();
        return;
      }
      else if(mSetUnsetInterMarkerAllowed && mPeakType != PTMarker)
      {
        //TODO: should cause intermarkers renumeration and recoloring inside near markers pair
        auto cmd = new SetPeakTypeCommand(mParentPlot->model(), mUid, mPeakType, (mPeakType == PTInterMarker ? PTPeak : PTInterMarker));
        emit newCommand(cmd);
        event->accept();
        return;
      }
      else
        return GPLayerable::mousePressEvent(event, details);
    }

    if(!isSelected || !mBorderMoveAllowed)
      return GPLayerable::mousePressEvent(event, details);

    HitTest(event, &mDragPart);
    if(mDragPart != HitPartNone)
    {
      mMovedBorderOriginalValue = mDragPart == HitPartStart ? mLeftBorder : mRightBorder;
      event->accept();
      return;
    }
  }
  return GPLayerable::mousePressEvent(event, details);
}

void GenesisMarkup::ChromatogramPeak::mouseMoveEvent(QMouseEvent *event, const QPointF &startPos)
{
  if(mDragPart == HitPartNone)
    return GPLayerable::mouseMoveEvent(event, startPos);
  event->accept();
  switch(mDragPart)
  {
  case HitPartStart:
  {
    auto mappedX = parentPlot()->xAxis->pixelToCoord(event->pos().x());
    setLeftBorder(mappedX);
    mLayer->replot();
    break;
  }
  case HitPartEnd:
  {
    auto mappedX = parentPlot()->xAxis->pixelToCoord(event->pos().x());
    setRightBorder(mappedX);
    mLayer->replot();
    break;
  }
  default://should never be here
    break;
  }
}

void GenesisMarkup::ChromatogramPeak::mouseReleaseEvent(QMouseEvent *event, const QPointF &startPos)
{
  if(mDragPart != HitPartNone)
  {
    switch(mDragPart)
    {
    case HitPartStart:
    {
      auto cmd = new MovePeakBorderCommand(mParentPlot->model(), mUid, mMovedBorderOriginalValue, mLeftBorder, mPeakCenter.x(), true);
      emit newCommand(cmd);
      break;
    }
    case HitPartEnd:
    {
      auto cmd = new MovePeakBorderCommand(mParentPlot->model(), mUid, mMovedBorderOriginalValue, mRightBorder, mPeakCenter.x(), false);
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
