#include "custom_grid.h"

#include "custom_axis.h"

void CustomGrid::drawGridLines(GPPainter* painter) const
{
  if (!mParentAxis)
    return;

  const auto tickVector = mParentAxis->tickVector();
  const auto axisRect = mParentAxis->axisRect();
  const auto axisRange = mParentAxis->range();
  const int tickCount = tickVector.size();
  double axisPixelCoord; // helper variable, result of coordinate-to-pixel transforms
  if (mParentAxis->orientation() == Qt::Horizontal)
  {
    // draw zeroline:
    int zeroLineIndex = -1;
    if (mZeroLinePen.style() != Qt::NoPen
      && axisRange.lower < 0
      && axisRange.upper > 0)
    {
      applyAntialiasingHint(painter, mAntialiasedZeroLine, GP::aeZeroLine);
      painter->setPen(mZeroLinePen);
      const double epsilon = axisRange.size() * 1E-6; // for comparing double to zero
      const auto it = std::find_if(tickVector.begin(), tickVector.end(), [&](double tickPos) { return qAbs(tickPos) < epsilon; });
      if (it != tickVector.end())
      {
        zeroLineIndex = std::distance(tickVector.begin(), it);
        axisPixelCoord = mParentAxis->coordToPixel(*it); // x
        painter->drawLine(QLineF(axisPixelCoord, axisRect->bottom(), axisPixelCoord, axisRect->top()));
      }
    }
    // draw grid lines:
    applyAntialiasingHint(painter, true, GP::aeGrid);
    painter->setPen(mPen);
    const auto middlePoint = (axisRect->right() - axisRect->left()) / 2.;
    const auto leftCoord = mParentAxis->coordToPixel(tickVector.front());
    const auto mid = mParentAxis->coordToPixel((tickVector.front() + tickVector.back()) / 2);
    for (int i = 1; i < tickCount - 1; ++i)
    {
      axisPixelCoord = mParentAxis->coordToPixel(tickVector.at(i));
      const auto offset = axisRect->height() * (axisPixelCoord - leftCoord) / (mid - leftCoord) / 2;
      painter->drawLine(QPointF(axisPixelCoord, axisRect->bottom()),
        QPointF((leftCoord + axisPixelCoord) / 2 + middlePoint,
          axisRect->top() + offset));

      // second line
      painter->drawLine(QPointF(axisPixelCoord, axisRect->bottom()),
        QPointF((leftCoord + axisPixelCoord) / 2,
          axisRect->bottom() - offset));
    }
  }
  else
  {
    // draw zeroline:
    int zeroLineIndex = -1;
    if (mZeroLinePen.style() != Qt::NoPen
      && axisRange.lower < 0
      && axisRange.upper > 0)
    {
      applyAntialiasingHint(painter, mAntialiasedZeroLine, GP::aeZeroLine);
      painter->setPen(mZeroLinePen);
      double epsilon = axisRange.size() * 1E-6; // for comparing double to zero
      const auto it = std::find_if(tickVector.begin(), tickVector.end(), [&](double tickPos) { return qAbs(tickPos) < epsilon; });
      if (it != tickVector.end())
      {
        zeroLineIndex = std::distance(tickVector.begin(), it);
        axisPixelCoord = mParentAxis->coordToPixel(*it); // y
        painter->drawLine(QLineF(axisRect->left(), axisPixelCoord, axisRect->right(), axisPixelCoord));
      }
    }
    // draw grid lines:
    applyDefaultAntialiasingHint(painter);
    painter->setPen(mPen);
    const auto middlePoint = (axisRect->right() - axisRect->left()) / 2.; // x
    for (int i = 1; i < tickCount - 1; ++i)
    {
      axisPixelCoord = mParentAxis->coordToPixel(tickVector.at(i)); // y
      const auto offset = axisRect->width() * (axisPixelCoord - axisRect->top()) / (axisRect->height()) / 2;
      painter->drawLine(QLineF(axisRect->left() + middlePoint - offset, axisPixelCoord,
        axisRect->left() + middlePoint + offset, axisPixelCoord));
    }
  }
}

void CustomGrid::drawSubGridLines(GPPainter* painter) const
{
  if (!mParentAxis)
    return;
  if (const auto customAxis = qobject_cast<CustomAxis*>(mParentAxis))
  {
    const auto subTickVector = customAxis->subTickVector();
    const auto axisRect = mParentAxis->axisRect();
      applyAntialiasingHint(painter, mAntialiasedSubGrid, GP::aeSubGrid);
    double axisPixelCoord; // helper variable, result of coordinate-to-pixel transforms
    painter->setPen(mSubGridPen);
    if (mParentAxis->orientation() == Qt::Horizontal)
    {
      for (const auto& tickPos : subTickVector)
      {
        axisPixelCoord = mParentAxis->coordToPixel(tickPos); // x
        painter->drawLine(QLineF(axisPixelCoord, axisRect->bottom(), axisPixelCoord, axisRect->top()));
      }
    }
    else
    {
      for (const auto& tickPos : subTickVector)
      {
        axisPixelCoord = mParentAxis->coordToPixel(tickPos); // x
        painter->drawLine(QLineF(axisRect->left(), axisPixelCoord, axisRect->right(), axisPixelCoord));
      }
    }
  }
}
