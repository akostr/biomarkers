#include "custom_axis_painter_private.h"

#include <QByteArray>
#include <QPoint>
#include <QLine>

CustomAxisPainterPrivate::CustomAxisPainterPrivate(GraphicsPlot* parent)
  : GPAxisPainterPrivate(parent)
{
}

void CustomAxisPainterPrivate::draw(GPPainter* painter)
{
  QByteArray newHash = generateTickLabelParameterHash();
  if (newHash != mTickLabelParameterHash)
  {
    mLabelCache.clear();
    mTickLabelParameterHash = std::move(newHash);
  }

  QPoint origin;
  switch (type)
  {
  case GPAxis::atLeft:   origin = axisRect.bottomLeft() ; break;
  case GPAxis::atRight:  origin = axisRect.bottomRight() ; break;
  case GPAxis::atTop:    origin = axisRect.topLeft(); break;
  case GPAxis::atBottom: origin = axisRect.bottomLeft() ; break;
  }

  double xCor = 0, yCor = 0; // paint system correction, for pixel exact matches (affects baselines and ticks of top/right axes)
  double xOffset = 0;
  const auto middlePoint = std::abs((static_cast<double>(axisRect.left()) - static_cast<double>(axisRect.right()))) / 2;
  switch (type)
  {
  case GPAxis::atTop: yCor = -1; break;
  case GPAxis::atRight: xCor = 1; xOffset = -middlePoint; break;
  case GPAxis::atLeft: xOffset = middlePoint; break;
  default: break;
  }
  int margin = 0;

  // draw baseline:
  QLineF baseLine;
  painter->setPen(basePen);
  if (GPAxis::orientation(type) == Qt::Horizontal)
    baseLine.setPoints(origin + QPointF(xCor, yCor), origin + QPointF(axisRect.width() + xCor, yCor));
  else
    baseLine.setPoints(origin + QPointF(xCor, yCor), origin + QPointF(xCor + xOffset, -axisRect.height() + yCor));
  painter->drawLine(baseLine);
  // draw ticks:
  if (!tickPositions.isEmpty())
  {
    const auto step = xOffset / (tickPositions.size() - 1);
    painter->setPen(tickPen);
    int tickDir = (type == GPAxis::atBottom || type == GPAxis::atRight) ? -1 : 1; // direction of ticks ("inward" is right for left axis and left for right axis)
    if (GPAxis::orientation(type) == Qt::Horizontal)
    {
      for (int i = 0; i < tickPositions.size(); ++i)
        painter->drawLine(QLineF(tickPositions.at(i) + xCor, origin.y() - tickLengthOut * tickDir + yCor, tickPositions.at(i) + xCor, origin.y() + tickLengthIn * tickDir + yCor));
    }
    else
    {
      for (int i = 0; i < tickPositions.size(); ++i)
        painter->drawLine(QLineF(origin.x() - tickLengthOut * tickDir + xCor + (reversedEndings? tickPositions.size() - i - 1 : i) * step, tickPositions.at(i) + yCor, origin.x() + tickLengthIn * tickDir + xCor + (reversedEndings ? tickPositions.size() - i - 1 : i) * step, tickPositions.at(i) + yCor));
    }
  }

  // draw subticks:
  if (!subTickPositions.isEmpty())
  {
    const auto subticksPerTicks = subTickPositions.size() / (tickPositions.size() - 1);
    painter->setPen(subTickPen);
    const auto step = xOffset / (subTickPositions.size()  + tickPositions.size() - 1);
    // direction of ticks ("inward" is right for left axis and left for right axis)
    int tickDir = (type == GPAxis::atBottom || type == GPAxis::atRight) ? -1 : 1;
    if (GPAxis::orientation(type) == Qt::Horizontal)
    {
      for (int i = 0; i < subTickPositions.size(); ++i)
        painter->drawLine(QLineF(subTickPositions.at(i) + xCor, origin.y() - subTickLengthOut * tickDir + yCor, subTickPositions.at(i) + xCor, origin.y() + subTickLengthIn * tickDir + yCor));
    }
    else
    {
      int indexOffset = 0;
      for (int i = 0; i < subTickPositions.size(); ++i)
      {
        if (i % subticksPerTicks == 0)
          indexOffset++;
        const auto correctPos = (reversedEndings ? subTickPositions.size() - i + (subticksPerTicks - indexOffset + 2) : i + indexOffset) * step;
        painter->drawLine(QLineF(origin.x() - subTickLengthOut * tickDir + xCor + correctPos, subTickPositions.at(i) + yCor, origin.x() + subTickLengthIn * tickDir + xCor + correctPos, subTickPositions.at(i) + yCor));
      }
    }
  }
  margin += qMax(0, qMax(tickLengthOut, subTickLengthOut));

  // draw axis base endings:
  bool antialiasingBackup = painter->antialiasing();
  painter->setAntialiasing(true); // always want endings to be antialiased, even if base and ticks themselves aren't
  painter->setBrush(QBrush(basePen.color()));
  GPVector2D baseLineVector(baseLine.dx(), baseLine.dy());
  if (lowerEnding.style() != GPLineEnding::esNone)
    lowerEnding.draw(painter, GPVector2D(baseLine.p1()) - baseLineVector.normalized() * lowerEnding.realLength() * (lowerEnding.inverted() ? -1 : 1), -baseLineVector);
  if (upperEnding.style() != GPLineEnding::esNone)
    upperEnding.draw(painter, GPVector2D(baseLine.p2()) + baseLineVector.normalized() * upperEnding.realLength() * (upperEnding.inverted() ? -1 : 1), baseLineVector);
  painter->setAntialiasing(antialiasingBackup);

  // tick labels:
  QRect oldClipRect;
  if (tickLabelSide == GPAxis::lsInside) // if using inside labels, clip them to the axis rect
  {
    oldClipRect = painter->clipRegion().boundingRect();
    painter->setClipRect(axisRect);
  }
  QSize tickLabelsSize(0, 0); // size of largest tick label, for offset calculation of axis label
  if (!tickLabels.isEmpty())
  {
    if (tickLabelSide == GPAxis::lsOutside)
      margin += tickLabelPadding;
    painter->setFont(tickLabelFont);
    painter->setPen(QPen(tickLabelColor));
    const int maxLabelIndex = qMin(tickPositions.size(), tickLabels.size());
    int distanceToAxis = margin;
    if (tickLabelSide == GPAxis::lsInside)
      distanceToAxis = -(qMax(tickLengthIn, subTickLengthIn) + tickLabelPadding);

    xOffset = ((type == GPAxis::atTop) || (type == GPAxis::atBottom)) ? 0 : -middlePoint;
    const auto step = xOffset / (maxLabelIndex - 1);
    offset = 0;
    for (int i = 0; i < maxLabelIndex; ++i)
      placeTickLabel(painter, tickPositions.at(i) - 5, distanceToAxis + (reversedEndings ? tickPositions.size() - i - 1 : i) * step, tickLabels.at(i), &tickLabelsSize);
    if (tickLabelSide == GPAxis::lsOutside)
      margin += (GPAxis::orientation(type) == Qt::Horizontal) ? tickLabelsSize.height() : tickLabelsSize.width();
  }
  if (tickLabelSide == GPAxis::lsInside)
    painter->setClipRect(oldClipRect);

  // axis label:
  QRect labelBounds;
  if (!label.isEmpty())
  {
    painter->setFont(labelFont);
    painter->setPen(QPen(labelColor));
    painter->setBackground(Qt::transparent);
    painter->setCompositionMode(QPainter::CompositionMode_SourceOver);
    labelBounds = painter->fontMetrics().boundingRect(0, 0, 0, 0, Qt::TextDontClip, label);
    if (type == GPAxis::atLeft || type == GPAxis::atRight)
    {
      QByteArray newHash = generateLabelParameterHash();
      if (mLabelParameterHash != newHash || mNameLabelCache.size() != labelBounds.size())
      {
        mLabelParameterHash = std::move(newHash);
        QImage buffer(labelBounds.width(), labelBounds.height(), QImage::Format_RGBA8888_Premultiplied);
        buffer.fill(Qt::transparent);
        {
          QPainter bufferPainter(&buffer);
          bufferPainter.setRenderHints(QPainter::TextAntialiasing | QPainter::Antialiasing);
          bufferPainter.setFont(labelFont);
          bufferPainter.setPen(QPen(labelColor));
          bufferPainter.drawText(0, 0, labelBounds.width(), labelBounds.height(), Qt::TextDontClip | Qt::AlignCenter, label);
        }
        mNameLabelCache = std::move(buffer);
      }

      QTransform oldTransform = painter->transform();
      if (type == GPAxis::atRight)
      {
        painter->translate(origin.x() + margin + labelBounds.height() + labelPadding, origin.y() - axisRect.height() / 2 - labelBounds.width() / 2);
        painter->rotate(90);
        painter->drawImage(0, 0, mNameLabelCache);
      }
      else
      {
        painter->translate(origin.x() - margin - labelBounds.height() - labelPadding, origin.y() - axisRect.height() / 2 + labelBounds.width() / 2);
        painter->rotate(-90);
        painter->drawImage(0, 0, mNameLabelCache);
      }
      painter->setTransform(oldTransform);
    }
    else if (type == GPAxis::atTop)
      painter->drawText(origin.x(), origin.y() - margin - labelBounds.height(), axisRect.width(), labelBounds.height(), Qt::TextDontClip | Qt::AlignCenter, label);
    else if (type == GPAxis::atBottom)
      painter->drawText(origin.x(), origin.y() + margin, axisRect.width(), labelBounds.height(), Qt::TextDontClip | Qt::AlignCenter, label);
  }

  // set selection boxes:
  int selectionTolerance = 0;
  if (mParentPlot)
    selectionTolerance = mParentPlot->selectionTolerance() * 0.99;
  else
    qDebug() << Q_FUNC_INFO << "mParentPlot is null";
  int selAxisOutSize = qMax(qMax(tickLengthOut, subTickLengthOut), selectionTolerance);
  int selAxisInSize = 0;// selectionTolerance;
  int selTickLabelSize;
  int selTickLabelOffset;
  if (tickLabelSide == GPAxis::lsOutside)
  {
    selTickLabelSize = (GPAxis::orientation(type) == Qt::Horizontal ? tickLabelsSize.height() : tickLabelsSize.width());
    selTickLabelOffset = qMax(tickLengthOut, subTickLengthOut) + tickLabelPadding;
  }
  else
  {
    selTickLabelSize = -(GPAxis::orientation(type) == Qt::Horizontal ? tickLabelsSize.height() : tickLabelsSize.width());
    selTickLabelOffset = -(qMax(tickLengthIn, subTickLengthIn) + tickLabelPadding);
  }
  int selLabelSize = labelBounds.height();
  int selLabelOffset = qMax(tickLengthOut, subTickLengthOut) + (!tickLabels.isEmpty() && tickLabelSide == GPAxis::lsOutside ? tickLabelPadding + selTickLabelSize : 0) + labelPadding;
  if (type == GPAxis::atLeft)
  {
    mAxisSelectionBox.setCoords(origin.x() - selAxisOutSize, axisRect.top(), origin.x() + selAxisInSize, axisRect.bottom());
    mTickLabelsSelectionBox.setCoords(origin.x() - selTickLabelOffset - selTickLabelSize, axisRect.top(), origin.x() - selTickLabelOffset, axisRect.bottom());
    mLabelSelectionBox.setCoords(origin.x() - selLabelOffset - selLabelSize, axisRect.top(), origin.x() - selLabelOffset, axisRect.bottom());
  }
  else if (type == GPAxis::atRight)
  {
    mAxisSelectionBox.setCoords(origin.x() - selAxisInSize, axisRect.top(), origin.x() + selAxisOutSize, axisRect.bottom());
    mTickLabelsSelectionBox.setCoords(origin.x() + selTickLabelOffset + selTickLabelSize, axisRect.top(), origin.x() + selTickLabelOffset, axisRect.bottom());
    mLabelSelectionBox.setCoords(origin.x() + selLabelOffset + selLabelSize, axisRect.top(), origin.x() + selLabelOffset, axisRect.bottom());
  }
  else if (type == GPAxis::atTop)
  {
    mAxisSelectionBox.setCoords(axisRect.left(), origin.y() - selAxisOutSize, axisRect.right(), origin.y() + selAxisInSize);
    mTickLabelsSelectionBox.setCoords(axisRect.left(), origin.y() - selTickLabelOffset - selTickLabelSize, axisRect.right(), origin.y() - selTickLabelOffset);
    mLabelSelectionBox.setCoords(axisRect.left(), origin.y() - selLabelOffset - selLabelSize, axisRect.right(), origin.y() - selLabelOffset);
  }
  else if (type == GPAxis::atBottom)
  {
    mAxisSelectionBox.setCoords(axisRect.left(), origin.y() - selAxisInSize, axisRect.right(), origin.y() + selAxisOutSize);
    mTickLabelsSelectionBox.setCoords(axisRect.left(), origin.y() + selTickLabelOffset + selTickLabelSize, axisRect.right(), origin.y() + selTickLabelOffset);
    mLabelSelectionBox.setCoords(axisRect.left(), origin.y() + selLabelOffset + selLabelSize, axisRect.right(), origin.y() + selLabelOffset);
  }
  mAxisSelectionBox = mAxisSelectionBox.normalized();
  mTickLabelsSelectionBox = mTickLabelsSelectionBox.normalized();
  mLabelSelectionBox = mLabelSelectionBox.normalized();
  mAxisFullBox = mLabelSelectionBox.united(mTickLabelsSelectionBox).united(mAxisSelectionBox).united(mAxisSelectionBox);

}
