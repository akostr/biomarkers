#include "spectrum_plot.h"
#include <ui/plots/gp_items/gpshape_item.h>
#include <ui/plots/gp_items/bars_labels_item.h>

SpectrumPlot::SpectrumPlot(QWidget *parent)
  :GraphicsPlot(parent)
{
  for(int i = 0; i < GPShapeItem::LastColor; i++)
    kColorsDrum << GPShapeItem::ToColor((GPShapeItem::DefaultColor)i);

  mGroup = new GPBarsGroup(this);
  mGroup->setSpacing(0);
  mGroup->setSpacingType(GPBarsGroup::SpacingType::stPlotCoords);
  addLayer("referenceBarsLayer");
  addLayer("referenceCurveLayer");
  addLayer("otherBarsLayer");
  addLayer("otherCurveLayer");
  addLayer("otherLabelsLayer");
  addLayer("referenceLabelsLayer");
  layer("referenceCurveLayer")->setVisible(false);
  layer("otherCurveLayer")->setVisible(false);
  layer("referenceLabelsLayer")->setVisible(false);
  layer("otherLabelsLayer")->setVisible(false);
  addLayer("tipLayer");
  layer("tipLayer")->setMode(GPLayer::LayerMode::lmBuffered);
  mTip = new GPItemText(this);
  mTipLine = new GPItemLine(this);
  mTipLineEnd = new GPItemLine(this);
  mTip->setText("");
  mTip->setLayer("tipLayer");
  mTipLine->setLayer("tipLayer");
  mTipLineEnd->setLayer("tipLayer");

  mTip->position->setPixelPosition({0,0});
  mTipLine->start->setPixelPosition({0,0});
  mTipLine->end->setPixelPosition({0,0});
  mTipLineEnd->start->setPixelPosition({0,0});
  mTipLineEnd->end->setPixelPosition({0,0});

  mTip->setBrush(QColor(255,255,255,150));

  //// axes texts
  xAxis->setLabel("m/z");
  xAxis->setLabelColor(QColor(102, 121, 133));
  xAxis->setTickLabelColor(QColor(102, 121, 133));

  yAxis->setLabel(tr("Relative intensity, %"));
  yAxis->setLabelColor(QColor(102, 121, 133));
  yAxis->setTickLabelColor(QColor(102, 121, 133));

  //// axes lines
  xAxis->setBasePen(QColor(102, 121, 133));
  xAxis->setTickPen(QColor(102, 121, 133));
  xAxis->setSubTickPen(QColor(102, 121, 133));
  xAxis->setSelectedBasePen(QColor(102, 121, 133));
  xAxis->setSelectedLabelColor(QColor(102, 121, 133));
  xAxis->setSelectedTickLabelColor(QColor(102, 121, 133));
  xAxis->setSelectedTickPen(QColor(102, 121, 133));
  xAxis->setSelectedSubTickPen(QColor(102, 121, 133));

  yAxis->setBasePen(QColor(102, 121, 133));
  yAxis->setTickPen(QColor(102, 121, 133));
  yAxis->setSubTickPen(QColor(102, 121, 133));
  yAxis->setSelectedBasePen(QColor(102, 121, 133));
  yAxis->setSelectedLabelColor(QColor(102, 121, 133));
  yAxis->setSelectedTickLabelColor(QColor(102, 121, 133));
  yAxis->setSelectedTickPen(QColor(102, 121, 133));
  yAxis->setSelectedSubTickPen(QColor(102, 121, 133));
}

void SpectrumPlot::SwitchMode(bool isBarsMode)
{
  mIsBarsMode = isBarsMode;
  layer("referenceBarsLayer")->setVisible(isBarsMode);
  layer("referenceCurveLayer")->setVisible(!isBarsMode);
  layer("otherBarsLayer")->setVisible(isBarsMode);
  layer("otherCurveLayer")->setVisible(!isBarsMode);
  replot();
}

void SpectrumPlot::setChromaLabelsVisible(bool visible)
{
  layer("referenceLabelsLayer")->setVisible(visible);
  layer("referenceLabelsLayer")->replot();
}

void SpectrumPlot::setCompoundLabelsVisible(bool visible)
{
  layer("otherLabelsLayer")->setVisible(visible);
  layer("otherLabelsLayer")->replot();
}


void SpectrumPlot::mouseMoveEvent(QMouseEvent *event)
{
  GraphicsPlot::mouseMoveEvent(event);
  auto textPos = event->pos() + QPointF(0, -50);
  mTip->position->setPixelPosition(textPos);
  mTipLine->start->setPixelPosition(textPos);
  mTipLine->end->setPixelPosition(textPos);
  mTipLineEnd->start->setPixelPosition(textPos);
  mTipLineEnd->end->setPixelPosition(textPos);
  mTip->setText("");
  QList<QVariant> selectionDetails;
  auto getPoint = [](const QVariant& vds, GPBars* bars)->QVariant
  {
    if(!vds.isValid() || bars == nullptr)
      return QVariant();
    auto ds = vds.value<GPDataSelection>();
    if(ds.dataRangeCount() == 0)
      return QVariant();
    auto it = bars->data()->at(ds.dataRange(0).begin());
    return QPointF(it->key, it->value);
  };
  auto candidates = layerableListAt(event->pos(), false, &selectionDetails);
  if(!candidates.isEmpty())
  {
    QPointF closestPt;
    GPBars* bars = nullptr;
    for(int i = 0; i < candidates.size(); i++)
    {
      auto tmpbars = dynamic_cast<GPBars*>(candidates[i]);
      if(tmpbars)
      {
        closestPt = getPoint(selectionDetails[i], tmpbars).toPointF();
        bars = tmpbars;
        if(tmpbars != mReferenceBars)
          break;
      }
    }
    if(bars)
    {
      mTip->setText(QString(tr("m/z: %1;\nintensity: %2")).arg(closestPt.x()).arg(closestPt.y()));
      QPointF left = closestPt + QPointF(-0.5, 0);
      QPointF right = closestPt + QPointF(0.5, 0);
      QPointF bestPt = left;
      QPointF plotCursorPos = QPointF(xAxis->pixelToCoord(event->pos().x()), yAxis->pixelToCoord(event->pos().y()));
      if(fabs(left.x() - plotCursorPos.x()) > fabs(right.x() - plotCursorPos.x()))
        bestPt = right;

      mTipLine->end->setCoords(bestPt);
      mTipLineEnd->start->setCoords(left);
      mTipLineEnd->end->setCoords(right);
    }
  }
  layer("tipLayer")->replot();
}

void SpectrumPlot::UpdateBars()
{
  double barsCount = mBarsMap.size();
  double maxWidth = 0.8;
  double width = maxWidth / barsCount * 0.8;
  for(auto& bars : mBarsMap)
  {
    bars->setWidth(width);
  }
}

void SpectrumPlot::setReferenceSpec(const Spectrum &spec)
{
if(spec.first.empty() || spec.second.empty())
  {
    if(!mReferenceBars && !mReferenceCurve && !mReferenceLabels)
      return;
    if(mReferenceBars)
      removePlottable(mReferenceBars);
    if(mReferenceCurve)
      removePlottable(mReferenceCurve);
    if(mReferenceLabels)
      removePlottable(mReferenceLabels);
    mReferenceBars = nullptr;
    mReferenceCurve = nullptr;
    mReferenceLabels = nullptr;
    return;
  }
  bool rescale = false;
  if(!mReferenceBars)
  {
    mReferenceBars = new GPBars(xAxis, yAxis);
    mReferenceBars->setPen(Qt::NoPen);
    mReferenceBars->setBrush(QColor(204, 217, 224, 255));
    mReferenceBars->setLayer("referenceBarsLayer");
    mReferenceBars->setWidth(0.9);

    mReferenceCurve = new GPCurve(xAxis, yAxis);
    mReferenceCurve->setPen(QColor(204, 217, 224, 255));
    mReferenceCurve->setLayer("referenceCurveLayer");

    mReferenceLabels = new BarsLabelsItem(xAxis, yAxis);
    mReferenceLabels->setPen(QColor(102, 121, 133));
    mReferenceLabels->setLayer("referenceLabelsLayer");

    rescale = true;
  }

  mReferenceBars->setData(spec.first, spec.second, true);
  mReferenceCurve->setData(spec.first, spec.second);
  mReferenceLabels->setData(spec.first, spec.second, true);
  UpdateBars();
  if(rescale)
  {
    rescaleAxes();
  }
  replot();
}

void SpectrumPlot::addSpec(int row, const Spectrum &spec)
{
  if(!mBarsMap.contains(row))
  {
    mBarsMap[row] = new GPBars(xAxis, yAxis);
    mBarsMap[row]->setPen(Qt::NoPen);
    mBarsMap[row]->setBrush(kColorsDrum[(mBarsMap.size() - 1) % kColorsDrum.size()]);
    mBarsMap[row]->setLayer("otherBarsLayer");
    mBarsMap[row]->setBarsGroup(mGroup);

    mCurvesMap[row] = new GPCurve(xAxis, yAxis);
    mCurvesMap[row]->setPen(kColorsDrum[(mBarsMap.size() - 1) % kColorsDrum.size()]);
    mCurvesMap[row]->setLayer("otherCurveLayer");

    mLabelsMap[row] = new BarsLabelsItem(xAxis, yAxis);
    mLabelsMap[row]->setLayer("otherLabelsLayer");
  }
  mBarsMap[row]->setData(spec.first, spec.second, true);
  mCurvesMap[row]->setData(spec.first, spec.second);
  mLabelsMap[row]->setData(spec.first, spec.second, true);
  assignColor(row);
  UpdateBars();
  replot();
}

void SpectrumPlot::removeSpec(int row)
{
  if(mBarsMap.contains(row))
  {
    removePlottable(mBarsMap[row]);
    removePlottable(mCurvesMap[row]);
    removePlottable(mLabelsMap[row]);
  }
  removeColor(row);

  mBarsMap.remove(row);
  mCurvesMap.remove(row);
  mLabelsMap.remove(row);
  UpdateBars();
  replot();
}

void SpectrumPlot::clearSpecs()
{
  clearPlottables();
  mBarsMap.clear();
  mCurvesMap.clear();
  mLabelsMap.clear();
  mReferenceBars = nullptr;
  mReferenceCurve = nullptr;
  mReferenceLabels = nullptr;
  replot();
}


void SpectrumPlot::mouseDoubleClickEvent(QMouseEvent *event)
{
  GraphicsPlot::mouseDoubleClick(event);
  rescaleAxes();
  replot();
}

#define EMPTY_ROW_VALUE -1
#define INVALID_INDEX -1

void SpectrumPlot::assignColor(int row)
{
  auto& bars = mBarsMap[row];
  auto& curve = mCurvesMap[row];
  auto& labels = mLabelsMap[row];
  int freeIndex = INVALID_INDEX;
  //looking for free space between
  for(int i = 0; i < mColorsToRowsMap.size(); i++)
  {
    if(mColorsToRowsMap[i] == EMPTY_ROW_VALUE)
    {
      freeIndex = i;
      break;
    }
  }
  QColor color;
  if(freeIndex != INVALID_INDEX)
  {//if there is free space, insert there
    mColorsToRowsMap[freeIndex] = row;
    color = kColorsDrum[freeIndex % kColorsDrum.size()];
  }
  else
  {//else append to the end
    color = kColorsDrum[mColorsToRowsMap.size() % kColorsDrum.size()];
    mColorsToRowsMap.append(row);
  }
  bars->setBrush(color);
  curve->setPen(color);
  labels->setPen(color);
  emit barColorChanged(row, color);
}

void SpectrumPlot::removeColor(int row)
{
  for(int i = 0; i < mColorsToRowsMap.size(); i++)
  {
    if(mColorsToRowsMap[i] == row)
    {
      mColorsToRowsMap[i] = EMPTY_ROW_VALUE;
      emit barColorChanged(row, QVariant());
      return;
    }
  }
}
