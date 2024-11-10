#include "chromatogram_key_selector.h"
#include <ui/plots/chromatogram_plot.h>

ChromatogramKeySelector::ChromatogramKeySelector(ChromatogramPlot *plot, GenesisMarkup::CurveDataModelPtr curve, QUuid uid)
  :GPLayerable(plot),
  mCurve(curve)
{
  setProperty("isSelected", true);
}

ChromatogramKeySelector::~ChromatogramKeySelector()
{
}

void ChromatogramKeySelector::setXCoord(double x)
{
  mXCoord = x;
  mLayer->replot();
}

double ChromatogramKeySelector::closestCurvePoint(double xCoord)
{
  auto& keys = mCurve->keys();
  auto it = std::lower_bound(keys.begin(), keys.end(), xCoord);
  if(it == keys.begin())
    return *it;
  if(it == keys.end())
    return keys.last();
  auto prev = it - 1;
  if(fabs(*it - xCoord) > fabs(*prev - xCoord))
    return *prev;
  return *it;
}

void ChromatogramKeySelector::draw(GPPainter *painter)
{
  painter->save();
  auto yax = parentPlot()->yAxis;
  auto xax = parentPlot()->xAxis;
  auto ybot = yax->coordToPixel(yax->range().lower);
  auto ytop = yax->coordToPixel(yax->range().upper);
  auto x = xax->coordToPixel(mXCoord);
  painter->setPen(Qt::red);
  painter->drawLine({x, ytop}, {x, ybot});
  painter->restore();
}

void ChromatogramKeySelector::mousePressEvent(QMouseEvent *event, const QVariant &details)
{
  if(event->button() == Qt::LeftButton)
  {
    HitTest(event, &mDragPart);
    if(mDragPart != HitPartNone)
    {
      auto mappedX = parentPlot()->xAxis->pixelToCoord(event->pos().x());
      mXCoord = closestCurvePoint(mappedX);
      emit xCoordChangedByUser(mXCoord);
      event->accept();
      return;
    }
  }
  event->ignore();
}

void ChromatogramKeySelector::mouseMoveEvent(QMouseEvent *event, const QPointF &startPos)
{
  if(mDragPart == HitPartNone)
  {
    event->ignore();
    return;
  }
  event->accept();
  switch(mDragPart)
  {
  case HitPartCenter:
  {
    auto mappedX = parentPlot()->xAxis->pixelToCoord(event->pos().x());
    mXCoord = closestCurvePoint(mappedX);
    emit xCoordChangedByUser(mXCoord);
    mLayer->replot();
    break;
  }
  default://should never be here
    break;
  }
}

void ChromatogramKeySelector::mouseReleaseEvent(QMouseEvent *event, const QPointF &startPos)
{
  if(mDragPart != HitPartNone && event->modifiers().testFlag(Qt::ShiftModifier))
  {
    mDragPart = HitPartNone;
    event->accept();
    emit xCoordChangedByUser(mXCoord);
  }
  else
  {
    event->ignore();
    return;
  }
}

Qt::CursorShape ChromatogramKeySelector::HitTest(QMouseEvent *event, unsigned int *part)
{
  if (part)
    *part = HitPartNone;

  if (part && event->modifiers().testFlag(Qt::ShiftModifier))
  {
    auto pixelX = parentPlot()->xAxis->coordToPixel(mXCoord);
    if(fabs(pixelX - event->pos().x()) < 5)
      *part = HitPartCenter;
    else
      *part = HitPartPlot;
  }
  if(part)
  {
    if(*part == HitPartCenter)
      return Qt::SplitHCursor;
    else if(*part == HitPartPlot)
      return Qt::CrossCursor;
  }
  return Qt::ArrowCursor;
}


double ChromatogramKeySelector::selectTest(const QPointF &pos, bool onlySelectable, QVariant *details) const
{
  return 0;
  double mousePos = pos.x();
  if(fabs(parentPlot()->xAxis->coordToPixel(mXCoord) - mousePos) < 3)
    return 0;
  return -1.;
}
