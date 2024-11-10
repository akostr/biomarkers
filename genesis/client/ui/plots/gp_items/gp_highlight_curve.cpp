#include "gp_highlight_curve.h"

GPHighlightCurve::GPHighlightCurve(GPAxis* keyAxis, GPAxis* valueAxis, bool registerInPLot)
  : GPCurve(keyAxis, valueAxis, registerInPLot)
{
  auto pen = mSelectionDecorator->pen();
  pen.setWidth(SelectionWidth);
  pen.setColor(SelectionColor);
  mSelectionDecorator->setPen(pen);
  GPScatterStyle scatterStyle;
  scatterStyle.setPen(pen);
  mSelectionDecorator->setScatterStyle(scatterStyle);
}

void GPHighlightCurve::ExcludeSelectedRange()
{
  QList<GPDataRange> selectedSegments, unselectedSegments;
  getDataSegments(selectedSegments, unselectedSegments);
  for (const auto& range : selectedSegments)
  {
    const auto it = std::find_if(ExcludedRanges.begin(), ExcludedRanges.end(),
      [&](const GPDataRange& excluded)
      {
        return excluded.intersects(range);
      });

    if (it != ExcludedRanges.end())
    {
      const auto expandedRange = it->expanded(range);
      it->setBegin(expandedRange.begin());
      it->setEnd(expandedRange.end());
    }
    else
    {
      ExcludedRanges.append(range);
    }
  }
}

void GPHighlightCurve::CancelExcludeSelectedRange()
{
  QList<GPDataRange> selectedSegments, unselectedSegments;
  getDataSegments(selectedSegments, unselectedSegments);
  for (const auto& range : selectedSegments)
    CancelExcludeSelectedRange(range);
}

QList<GPDataRange> GPHighlightCurve::GetExcludedRanges() const
{
  return ExcludedRanges;
}

QList<GPDataRange> GPHighlightCurve::GetSelectedSegments() const
{
  QList<GPDataRange> selectedSegments, unselectedSegments;
  getDataSegments(selectedSegments, unselectedSegments);
  return selectedSegments;
}

QList<GPDataRange> GPHighlightCurve::GetUnSelectedSegments() const
{
  QList<GPDataRange> selectedSegments, unselectedSegments;
  getDataSegments(selectedSegments, unselectedSegments);
  return unselectedSegments;
}

void GPHighlightCurve::AddRangeForExclude(const GPDataRange& range)
{
  const auto it = std::find_if(ExcludedRanges.begin(), ExcludedRanges.end(),
    [&](const GPDataRange& excluded)
    {
      return excluded.intersects(range);
    });

  if (it == ExcludedRanges.end())
    ExcludedRanges.append(range);
  else
  {
    auto expanded = it->expanded(range);
    ExcludedRanges.erase(it);
    ExcludedRanges.append(expanded);
  }
}

void GPHighlightCurve::CancelExcludeSelectedRange(const GPDataRange& range)
{
  const auto it = std::find_if(ExcludedRanges.begin(), ExcludedRanges.end(),
    [&](const GPDataRange& excluded)
    {
      return excluded.intersects(range);
    });
  // intersected intervals not found
  if (it == ExcludedRanges.end())
    return;
  // in case intervals is equal we delete interval
  if (*it == range)
  {
    ExcludedRanges.erase(it);
  }
  // in case range is subinterval it
  else if (it->contains(range))
  {
    auto leftRange = *it;
    leftRange.setEnd(range.begin());

    ExcludedRanges.erase(it); // should remove before add

    auto rightRange = *it;
    rightRange.setBegin(range.end());

    if (!leftRange.isEmpty())
      ExcludedRanges.append(leftRange);
    if (!rightRange.isEmpty())
      ExcludedRanges.append(rightRange);
  }
  // need found intersect part or
  // range is overlap it
  else if (it->intersects(range))
  {
    // Если range поверх it
    if (range.contains(*it))
    {
      ExcludedRanges.erase(it);
    }
    // found corner for cut
    else
    {
      if (range.begin() < it->begin()
        && it->begin() < range.end())
      {
        it->setBegin(range.end());
      }
      else if (range.begin() < it->end()
        && it->end() < range.end())
      {
        it->setEnd(range.begin());
      }
    }
  }
}

void GPHighlightCurve::ClearExcluded()
{
  ExcludedRanges.clear();
}

void GPHighlightCurve::SetExcludedColor(const QColor& color)
{
  ExcludedColor = color;
}

void GPHighlightCurve::SetSelectionColor(const QColor& color)
{
  SelectionColor = color;
}

void GPHighlightCurve::SetSelectionWidth(int width)
{
  SelectionWidth = width;
}

bool GPHighlightCurve::IsSelectedAreExcluded() const
{
  QList<GPDataRange> selectedSegments, unselectedSegments;
  getDataSegments(selectedSegments, unselectedSegments);
  for (const auto& range : selectedSegments)
  {
    const auto it = std::find_if(ExcludedRanges.begin(), ExcludedRanges.end(),
      [&](const GPDataRange& excluded)
      {
        return excluded.intersects(range);
      });
    if (it != ExcludedRanges.end())
    {
      return true;
    }
  }
  return false;
}

void GPHighlightCurve::draw(GPPainter* painter)
{
  if (!mDataContainer)
    return;

  if (mDataContainer->isEmpty())
    return;

  // allocate line vector:
  QVector<QPointF> lines, scatters;

  // loop over and draw segments of unselected/selected data:
  QList<GPDataRange> selectedSegments, unselectedSegments, allSegments;
  getDataSegments(selectedSegments, unselectedSegments);
  allSegments << unselectedSegments << selectedSegments << ExcludedRanges;
  const auto unselSize = unselectedSegments.size();
  const auto selUnselSize = unselSize + selectedSegments.size();
  for (int i = 0; i < allSegments.size(); ++i)
  {
    bool isSelectedSegment = i >= unselSize && i < selUnselSize;
    bool isExcludedSegment = i >= selUnselSize;
    // fill with curve data:
    QPen finalCurvePen = mPen; // determine the final pen already here, because the line optimization depends on its stroke width
    if (isSelectedSegment && mSelectionDecorator)
      finalCurvePen = mSelectionDecorator->pen();
    else if (isExcludedSegment)
      finalCurvePen.setColor(ExcludedColor);
    GPDataRange lineDataRange = isSelectedSegment ? allSegments.at(i) : allSegments.at(i).adjusted(-1, 1); // unselected segments extend lines to bordering selected data point (safe to exceed total data bounds in first/last segment, getCurveLines takes care)
    getCurveLines(&lines, lineDataRange, finalCurvePen.widthF());

    // check data validity if flag set:
#ifdef GRAPHICSPLOT_CHECK_DATA
    for (GPCurveDataContainer::const_iterator it = mDataContainer->constBegin(); it != mDataContainer->constEnd(); ++it)
    {
      if (GP::isInvalidData(it->t) ||
        GP::isInvalidData(it->key, it->value))
        qDebug() << Q_FUNC_INFO << "Data point at" << it->key << "invalid." << "Plottable name:" << name();
    }
#endif

    // draw curve fill:
    applyFillAntialiasingHint(painter);
    if (isSelectedSegment && mSelectionDecorator)
      mSelectionDecorator->applyBrush(painter);
    else
      painter->setBrush(mBrush);
    painter->setPen(Qt::NoPen);

    if (painter->brush().style() != Qt::NoBrush && painter->brush().color().alpha() != 0)
    {
      bool isHorizontal = (mKeyAxis->axisType() == GPAxis::atBottom || mKeyAxis->axisType() == GPAxis::atTop);

      double valueMin = mValueAxis->coordToPixel(mValueAxis->range().lower);
      double valueMax = mValueAxis->coordToPixel(mValueAxis->range().upper);
      if (valueMin > valueMax)
        qSwap(valueMin, valueMax);

      double axisValue = 0;
      if (mKeyAxis->axisType() == GPAxis::atBottom || mKeyAxis->axisType() == GPAxis::atRight)
        axisValue = valueMax;
      else
        axisValue = valueMin;

      int i = 0;
      while (i < mDataContainer->size())
      {
        QPolygonF polygon;
        for (; i < mDataContainer->size(); ++i)
        {
          auto it = mDataContainer->at(i);
          if (!qIsNaN(it->key) && !qIsNaN(it->value))
          {
            if (isHorizontal)
              polygon << QPointF(mKeyAxis->coordToPixel(it->key), mValueAxis->coordToPixel(it->value));
            else
              polygon << QPointF(mValueAxis->coordToPixel(it->value), mKeyAxis->coordToPixel(it->key));
          }
          else
          {
            ++i;
            break;
          }
        }

        if (polygon.size() > 1)
        {
          QPointF last = polygon.last();
          QPointF first = polygon.first();
          if (isHorizontal)
          {
            polygon.append(QPointF(last.x(), axisValue));
            polygon.append(QPointF(first.x(), axisValue));
          }
          else
          {
            polygon.append(QPointF(axisValue, last.y()));
            polygon.append(QPointF(axisValue, first.y()));
          }
          painter->drawPolygon(polygon);
        }
      }
    }

    // draw curve line:
    if (mLineStyle != lsNone)
    {
      painter->setPen(finalCurvePen);
      painter->setBrush(Qt::NoBrush);
      drawCurveLine(painter, lines);
    }

    // draw scatters:
    GPScatterStyle finalScatterStyle = mScatterStyle;
    if (isSelectedSegment && mSelectionDecorator)
      finalScatterStyle = mSelectionDecorator->getFinalScatterStyle(mScatterStyle);
    else if (isExcludedSegment)
    {
      auto pen = finalScatterStyle.pen();
      pen.setColor(ExcludedColor);
      finalScatterStyle.setPen(pen);
      finalScatterStyle.setSize(SelectionWidth);
    }
    if (!finalScatterStyle.isNone())
    {
      getScatters(&scatters, allSegments.at(i), finalScatterStyle.size());
      drawScatterPlot(painter, scatters, finalScatterStyle);
    }
  }

  // draw other selection decoration that isn't just line/scatter pens and brushes:
  if (mSelectionDecorator)
    mSelectionDecorator->drawDecoration(painter, selection());
}
