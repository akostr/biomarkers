#include "graphics_item_interactive_pixmap.h"

GPItemInteractivePixmap::GPItemInteractivePixmap(GraphicsPlot *parentPlot)
  : GPItemPixmap(parentPlot)
  , CommonItemFuctional{ parentPlot }
  , topLeftEditable(createPosition(QLatin1String("topLeftEditable")))
  , bottomRightEditable(createPosition(QLatin1String("bottomRightEditable")))
  , center(createAnchor(QLatin1String("center"), aiCenter))
  , rotateAnchor(createAnchor(QLatin1String("rotate"), aiRotate))
  , topEditable(createAnchor(QLatin1String("topEditable"), aiTopEditable))
  , topRightEditable(createAnchor(QLatin1String("topRightEditable"), aiTopRightEditable))
  , rightEditable(createAnchor(QLatin1String("rightEditable"), aiRightEditable))
  , bottomEditable(createAnchor(QLatin1String("bottomEditable"), aiBottomEditable))
  , bottomLeftEditable(createAnchor(QLatin1String("bottomLeftEditable"), aiBottomLeftEditable))
  , leftEditable(createAnchor(QLatin1String("leftEditable"), aiLeftEditable))
  , m_opacity(1.0)
  , m_actionMode(MoveMode)
{
  setSelectable(true);
  const QPen OkPen = QPen(QColor(0x00,0x91,0xFF), 2);
  setSelectedPen(OkPen);
  DragModeMap = {{topLeft, DmResizeTopLeft},
      {topRight, DmResizeTopRight},
      {bottomLeft, DmResizeBottomLeft},
      {bottomRight, DmResizeBottomRight},
      {top, DmResizeTop},
      {bottom, DmResizeBottom},
      {left, DmResizeLeft},
      {right, DmResizeRight},
      {rotateAnchor, DmRotate},
      {topLeftEditable, DmResizeTopLeft},
      {topRightEditable, DmResizeTopRight},
      {bottomLeftEditable, DmResizeBottomLeft},
      {bottomRightEditable, DmResizeBottomRight},
      {topEditable, DmResizeTop},
      {bottomEditable, DmResizeBottom},
      {leftEditable, DmResizeLeft},
      {rightEditable, DmResizeRight},};
  //// Set valid or equally null input uom descriptor from axis before setCoords()
  if (parentPlot->xAxis)
  {
    topLeftEditable->mInputUomForKeys         = mParentPlot->xAxis->getUomDescriptor();
    bottomRightEditable->mInputUomForKeys     = mParentPlot->xAxis->getUomDescriptor();
  }
  if (parentPlot->yAxis)
  {
    topLeftEditable->mInputUomForValues       = mParentPlot->yAxis->getUomDescriptor();
    bottomRightEditable->mInputUomForValues   = mParentPlot->yAxis->getUomDescriptor();
  }
  topLeftEditable->setCoords(0, 1);
  bottomRightEditable->setCoords(1, 0);

  //// Restore input uom descriptor
  topLeftEditable->mInputUomForKeys = GPUomDescriptor();
  topLeftEditable->mInputUomForValues = GPUomDescriptor();
  bottomRightEditable->mInputUomForKeys = GPUomDescriptor();
  bottomRightEditable->mInputUomForValues = GPUomDescriptor();
  connect(this, &GPItemInteractivePixmap::DragMoved, this, &GPItemInteractivePixmap::move);
  connect(this, &GPItemInteractivePixmap::DragAccepted, this, &GPItemInteractivePixmap::move);
  connect(this, &GPItemInteractivePixmap::DragAccepted, this, &GPItemInteractivePixmap::updateTransform);
  connect (this, &GPItemInteractivePixmap::replot, parentPlot,
          [parentPlot]()
          {
            parentPlot->replot(GraphicsPlot::RefreshPriority::rpQueuedRefresh);
          });

  setInteractive(true);
  setScaled(true, Qt::IgnoreAspectRatio);
  setTransform(center->pixelPosition(), 0);
  setClipToAxisRect(true);

  connect(parentPlot->xAxis, qOverload<const GPRange &>(&GPAxis::rangeChanged), this, &GPItemInteractivePixmap::updateTransformAfterRangeChanged);
  connect(parentPlot->yAxis, qOverload<const GPRange &>(&GPAxis::rangeChanged), this, &GPItemInteractivePixmap::updateTransformAfterRangeChanged);
}

double GPItemInteractivePixmap::selectTest(const QPointF &pos, bool onlySelectable, QVariant *details) const
{
  if (onlySelectable && !mSelectable)
    return -1;

  QPointF realPos = m_invertedTransform.map(pos);

  double distance = rectDistance(getFinalRect(), realPos, true);

  if (selected())
  {
    QPointF delta = rotateAnchor->pixelPosition() - realPos;
    double rotatePointDistance = qSqrt(delta.x()*delta.x() + delta.y()*delta.y());
    distance = qMin(distance, rotatePointDistance);
  }
  return distance;
}

void GPItemInteractivePixmap::FillContextMenu(QContextMenuEvent* event, QMenu& menu, std::map<QAction*, std::function<void(void)>>& actions)
{
  if (m_actionMode == MoveMode)
  {
    QAction* setEditMode = menu.addAction(tr("Change active rect"));
    actions[setEditMode] = [this](){ this->setEditMode(); };
  }
  else
  {
    QAction* saveChanges = menu.addAction(tr("Save changes"));
    actions[saveChanges] = [this](){ this->saveEditModeChanges(); };
    QAction* rejectChanges = menu.addAction(tr("Reject changes"));
    actions[rejectChanges] = [this](){ this->rejectEditModeChanges(); };
  }
}

void GPItemInteractivePixmap::setEditMode()
{
  m_actionMode = EditMode;
  for (auto pos: mPositions)
  {
    m_savedPosition.append(pos->coords());
  }
  emit replot();
}

void GPItemInteractivePixmap::saveEditModeChanges()
{
  m_actionMode = MoveMode;
  m_savedPosition.clear();
  emit replot();
}

void GPItemInteractivePixmap::rejectEditModeChanges()
{
  m_actionMode = MoveMode;
  for (int i = 0; i < m_savedPosition.size(); i++)
  {
    mPositions[i]->setCoords(m_savedPosition[i]);
  }
  emit topLeftChanged(topLeft->coords());
  emit bottomRightChanged(bottomRight->coords());
  emit topLeftEditableChanged(topLeftEditable->coords());
  emit bottomRightEditableChanged(bottomRightEditable->coords());
  m_savedPosition.clear();
  emit replot();
}

void GPItemInteractivePixmap::movePos(QPointF delta)
{
  setTopLeftEditablePos(topLeftEditable->pixelPosition() + delta);
  setBottomRightEditablePos(bottomRightEditable->pixelPosition() + delta);
  setTopLeftPos(topLeft->pixelPosition() + delta);
  setBottomRightPos(bottomRight->pixelPosition() + delta);
}

void GPItemInteractivePixmap::resizePos(QPointF newTopLeft, QPointF newBottomRight)
{
  QPointF prevTopLeft = topLeft->pixelPosition();
  QPointF prevBottomRight = bottomRight->pixelPosition();
  QPointF prevTopLeftEditable = topLeftEditable->pixelPosition();
  QPointF prevBottomRightEditable = bottomRightEditable->pixelPosition();
  double x1 = prevTopLeftEditable.x() - prevTopLeft.x();
  double x2 = prevBottomRightEditable.x() - prevTopLeftEditable.x();
  double x3 = prevBottomRight.x() - prevBottomRightEditable.x();
  double y1 = prevTopLeftEditable.y() - prevTopLeft.y();
  double y2 = prevBottomRightEditable.y() - prevTopLeftEditable.y();
  double y3 = prevBottomRight.y() - prevBottomRightEditable.y();

  setTopLeftEditablePos(newTopLeft);
  QPointF newTopLeftEditable = topLeftEditable->pixelPosition();
  setBottomRightEditablePos(newBottomRight);
  QPointF newBottomRightEditable = bottomRightEditable->pixelPosition();
  double newX2 = newBottomRightEditable.x() - newTopLeftEditable.x();
  double newY2 = newBottomRightEditable.y() - newTopLeftEditable.y();

  double newX1 = newX2 * x1 / x2;
  double newY1 = newY2 * y1 / y2;
  double newX3 = newX2 * x3 / x2;
  double newY3 = newY2 * y3 / y2;

  double newTopLeftX = newTopLeftEditable.x() - newX1;
  double newTopLeftY = newTopLeftEditable.y() - newY1;
  double newBottomRightX = newBottomRightEditable.x() + newX3;
  double newBottomRightY = newBottomRightEditable.y() + newY3;

  setTopLeftPos(QPointF(newTopLeftX, newTopLeftY));
  setBottomRightPos(QPointF(newBottomRightX, newBottomRightY));
}

void GPItemInteractivePixmap::resizeTopLeftEditablePos(QPointF pos)
{
  resizePos(pos, bottomRightEditable->pixelPosition());
}

void GPItemInteractivePixmap::resizeBottomRightEditablePos(QPointF pos)
{
  resizePos(topLeftEditable->pixelPosition(), pos);
}

void GPItemInteractivePixmap::setTopLeftPos(QPointF pos)
{
  setTopLeftCoordinate(pixelToCoord(pos));
}

void GPItemInteractivePixmap::setBottomRightPos(QPointF pos)
{
  setBottomRightCoordinate(pixelToCoord(pos));
}

void GPItemInteractivePixmap::setTopLeftCoordinate(QPointF newTopLeft)
{
  if (topLeft->coords() != newTopLeft)
  {
    topLeft->setCoords(newTopLeft);
    emit topLeftChanged(newTopLeft);
    emit replot();
  }
}

void GPItemInteractivePixmap::setBottomRightCoordinate(QPointF newBottomRight)
{
  if (bottomRight->coords() != newBottomRight)
  {
    bottomRight->setCoords(newBottomRight);
    emit bottomRightChanged(newBottomRight);
    emit replot();
  }
}

void GPItemInteractivePixmap::setTopLeftEditablePos(QPointF pos)
{
  setTopLeftEditableCoordinate(pixelToCoord(pos));
}

void GPItemInteractivePixmap::setBottomRightEditablePos(QPointF pos)
{
  setBottomRightEditableCoordinate(pixelToCoord(pos));
}

void GPItemInteractivePixmap::setTopLeftEditableCoordinate(QPointF newTopLeft)
{
  if (std::isnan(newTopLeft.x()) || std::isinf(newTopLeft.x()))
    return;
  if (std::isnan(newTopLeft.y()) || std::isinf(newTopLeft.y()))
    return;
  if (topLeftEditable->coords() != newTopLeft)
  {
    topLeftEditable->setCoords(newTopLeft);
    emit topLeftEditableChanged(newTopLeft);
    emit replot();
  }
}

void GPItemInteractivePixmap::setBottomRightEditableCoordinate(QPointF newBottomRight)
{
  if (std::isnan(newBottomRight.x()) || std::isinf(newBottomRight.x()))
    return;
  if (std::isnan(newBottomRight.y()) || std::isinf(newBottomRight.y()))
    return;
  if (bottomRightEditable->coords() != newBottomRight)
  {
    bottomRightEditable->setCoords(newBottomRight);
    emit bottomRightEditableChanged(newBottomRight);
    emit replot();
  }
}

void GPItemInteractivePixmap::setVisible(bool isVisible)
{
  if (mVisible != isVisible)
  {
    mVisible = isVisible;
    emit replot();
  }
}

void GPItemInteractivePixmap::updateTransform()
{
  QPointF centerPoint = center->pixelPosition();
  QPointF editTopLeft = topLeftEditable->pixelPosition();
  QPointF editBottomRight = bottomRightEditable->pixelPosition();
  QPointF transformEditTopLeft = m_rotateTransform.map(editTopLeft);
  QPointF transformEditBottomRight = m_rotateTransform.map(editBottomRight);
  QPointF midPoint = (transformEditTopLeft + transformEditBottomRight) / 2;

  QPointF delta = midPoint - centerPoint;
  QPointF realTopLeft = editTopLeft + delta;
  QPointF realBottomRight = editBottomRight + delta;
  QRect realRect;
  realRect.setTopLeft(realTopLeft.toPoint());
  realRect.setBottomRight(realBottomRight.toPoint());
  QPointF realCenter = realRect.center();

  resizePos(realTopLeft, realBottomRight);
  setTransform(realCenter, m_rotate);
}

void GPItemInteractivePixmap::updateTransformAfterRangeChanged()
{
  QPointF centerPoint = center->pixelPosition();
  setTransform(centerPoint, m_rotate);
}

void GPItemInteractivePixmap::DragStart(QMouseEvent*event, unsigned part)
{
  CommonItemFuctional::DragStart(event, part);
}

void GPItemInteractivePixmap::move(QPointF deltaMoving)
{
  const QPointF newPos = m_previousDragPos + deltaMoving;
  QPointF newInvertedPosition = m_invertedTransform.map(newPos);
  QPointF previousInvertedPosition = m_invertedTransform.map(m_previousDragPos);
  QPointF deltaInverted = newInvertedPosition - previousInvertedPosition;

  if (m_actionMode == EditMode)
  {
    editPosition(deltaInverted);
  }
  else
  {
    movePosition(deltaInverted);
  }
}

void GPItemInteractivePixmap::movePosition(QPointF deltaMoving)
{
  QPointF topLeftPos = topLeftEditable->pixelPosition();
  QPointF bottomRightPos = bottomRightEditable->pixelPosition();
  switch(mDraggingPart)
  {
  case DmMove:
  {
    movePos(deltaMoving);
    break;
  }
  case DmResizeTopLeft:
  {
    resizeTopLeftEditablePos(topLeftPos + deltaMoving);
    break;
  }
  case DmResizeBottomRight:
  {
    resizeBottomRightEditablePos(bottomRightPos + deltaMoving);
    break;
  }
  case DmResizeTopRight:
  {
    resizePos(topLeftEditable->pixelPosition() +  QPointF(0, deltaMoving.y()), bottomRightEditable->pixelPosition() + QPointF(deltaMoving.x(), 0));
    break;
  }
  case DmResizeBottomLeft:
  {
    resizePos(topLeftEditable->pixelPosition() +  QPointF(deltaMoving.x(), 0), bottomRightEditable->pixelPosition() + QPointF(0, deltaMoving.y()));
    break;
  }
  case DmResizeTop:
  {
    resizeTopLeftEditablePos(topLeftPos + QPointF(0, deltaMoving.y()));
    break;
  }
  case DmResizeBottom:
  {
    resizeBottomRightEditablePos(bottomRightPos + QPointF(0, deltaMoving.y()));
    break;
  }
  case DmResizeLeft:
  {
    resizeTopLeftEditablePos(topLeftPos + QPointF(deltaMoving.x(), 0));
    break;
  }
  case DmResizeRight:
  {
    resizeBottomRightEditablePos(bottomRightPos + QPointF(deltaMoving.x(), 0));
    break;
  }
  case DmRotate:
  {
    const QPointF centerPos = m_rotationCenter;
    const QPointF pos = m_previousDragPos;
    const QLineF line(centerPos, pos);
    const qreal angle = 180 - QLineF(centerPos, QPointF(centerPos.x(), centerPos.y() + 1)).angleTo(line);
    setTransform(m_rotationCenter, angle);
    break;
  }
  case DmNone:
  default:
    break;
  }
}

void GPItemInteractivePixmap::editPosition(QPointF deltaMoving)
{
  QPointF topLeftPos = topLeft->pixelPosition();
  QPointF bottomRightPos = bottomRight->pixelPosition();
  double minX = std::min(topLeftPos.x(), bottomRightPos.x());
  double maxX = std::max(topLeftPos.x(), bottomRightPos.x());
  double minY = std::min(topLeftPos.y(), bottomRightPos.y());
  double maxY = std::max(topLeftPos.y(), bottomRightPos.y());
  auto limitPosition = [minX, maxX, minY, maxY] (QPointF newPos) -> QPointF
  {
    newPos.setX(std::clamp(newPos.x(), minX, maxX));
    newPos.setY(std::clamp(newPos.y(), minY, maxY));
    return newPos;
  };

  QPointF topLeftEditablePos = topLeftEditable->pixelPosition();
  QPointF bottomRightEditablePos = bottomRightEditable->pixelPosition();
  switch(mDraggingPart)
  {
  case DmMove:
  {
    QPointF newTopLeft = limitPosition(topLeftEditablePos + deltaMoving);
    QPointF newBottomRight = limitPosition(bottomRightEditablePos + deltaMoving);
    setTopLeftEditablePos(newTopLeft);
    setBottomRightEditablePos(newBottomRight);
    break;
  }
  case DmResizeTopLeft:
  {
    QPointF newTopLeft = limitPosition(topLeftEditablePos + deltaMoving);
    setTopLeftEditablePos(newTopLeft);
    break;
  }
  case DmResizeBottomRight:
  {
    QPointF newBottomRight = limitPosition(bottomRightEditablePos + deltaMoving);
    setBottomRightEditablePos(newBottomRight);
    break;
  }
  case DmResizeTopRight:
  {
    QPointF newTopLeft = limitPosition(topLeftEditablePos + QPointF(0, deltaMoving.y()));
    QPointF newBottomRight = limitPosition(bottomRightEditablePos + QPointF(deltaMoving.x(), 0));
    setTopLeftEditablePos(newTopLeft);
    setBottomRightEditablePos(newBottomRight);
    break;
  }
  case DmResizeBottomLeft:
  {
    QPointF newTopLeft = limitPosition(topLeftEditablePos + QPointF(deltaMoving.x(), 0));
    QPointF newBottomRight = limitPosition(bottomRightEditablePos + QPointF(0, deltaMoving.y()));
    setTopLeftEditablePos(newTopLeft);
    setBottomRightEditablePos(newBottomRight);
    break;
  }
  case DmResizeTop:
  {
    QPointF newTopLeft = limitPosition(topLeftEditablePos + QPointF(0, deltaMoving.y()));
    setTopLeftEditablePos(newTopLeft);
    break;
  }
  case DmResizeBottom:
  {
    QPointF newBottomRight = limitPosition(bottomRightEditablePos + QPointF(0, deltaMoving.y()));
    setBottomRightEditablePos(newBottomRight);
    break;
  }
  case DmResizeLeft:
  {
    QPointF newTopLeft = limitPosition(topLeftEditablePos + QPointF(deltaMoving.x(), 0));
    setTopLeftEditablePos(newTopLeft);
    break;
  }
  case DmResizeRight:
  {
    QPointF newBottomRight = limitPosition(bottomRightEditablePos + QPointF(deltaMoving.x(), 0));
    setBottomRightEditablePos(newBottomRight);
    break;
  }
  case DmRotate:
  {
    const QPointF centerPos = m_rotationCenter;
    const QPointF pos = m_previousDragPos;
    const QLineF line(centerPos, pos);
    const qreal angle = 180 - QLineF(centerPos, QPointF(centerPos.x(), centerPos.y() + 1)).angleTo(line);
    setTransform(m_rotationCenter, angle);
    break;
  }
  case DmNone:
  default:
    break;
  }
}

void GPItemInteractivePixmap::draw(GPPainter *painter)
{
  bool flipHorz = false;
  bool flipVert = false;
  QRect rect = getFinalRect(&flipHorz, &flipVert);
  double clipPad = mainPen().style() == Qt::NoPen ? 0 : mainPen().widthF();
  QRect boundingRect = rect.adjusted(-clipPad, -clipPad, clipPad, clipPad);
  QPolygon polygon = m_rotateTransform.mapToPolygon(boundingRect);
  auto realRect = clipRect();
  if (polygon.boundingRect().intersects(realRect))
  {
    painter->save();
    QPen pen;
    painter->setOpacity(m_opacity);
    painter->setTransform(m_rotateTransform);
    if (realRect.contains(polygon.boundingRect()))
    {
      updateScaledPixmap(rect, flipHorz, flipVert);
      painter->drawPixmap(rect.topLeft(), mScaled ? mScaledPixmap : mPixmap);
    }
    else
    {
      QSize size = mPixmap.size();
      QSize scaleSize = rect.size();
      double scaleX = (double)size.width() / scaleSize.width();
      double scaleY = (double)size.height() / scaleSize.height();
      QPolygon invertRect = m_invertedTransform.mapToPolygon(realRect);
      QPolygon visiblePart = invertRect.intersected(rect);
      QRect visibleRect = visiblePart.boundingRect();

      QPoint pos = rect.topLeft();
      QTransform translateTransform;
      translateTransform.translate(-pos.x(), -pos.y());
      QTransform scaledTransform;
      scaledTransform.scale(scaleX, scaleY);
      QRect translateRect = translateTransform.mapRect(visibleRect);
      QRect pixmapVisiblePart = scaledTransform.mapRect(translateRect);
      QRect pixmapVisibleflipRect = pixmapVisiblePart;
      if (flipHorz)
      {
        QTransform trans;
        trans.translate(size.width() / 2.0, size.height() / 2.0);
        trans.scale(-1,1);
        trans.translate(-size.width() / 2.0, -size.height() / 2.0);
        pixmapVisibleflipRect = trans.mapRect(pixmapVisibleflipRect);
      }
      if (flipVert)
      {
        QTransform trans;
        trans.translate(size.width() / 2.0, size.height() / 2.0);
        trans.scale(1,-1);
        trans.translate(-size.width() / 2.0, -size.height() / 2.0);
        pixmapVisibleflipRect = trans.mapRect(pixmapVisibleflipRect);
      }
      if (m_pixmapVisibleRect != pixmapVisibleflipRect)
      {
        m_pixmapVisibleRect = pixmapVisibleflipRect;
        m_visiblePart = mPixmap.copy(m_pixmapVisibleRect);
        m_visibleScalePart = m_visiblePart.scaled(visibleRect.size(), mAspectRatioMode, mTransformationMode);
        if (flipHorz || flipVert)
          m_visibleScalePart = QPixmap::fromImage(m_visibleScalePart.toImage().mirrored(flipHorz, flipVert));
      }
      else if (m_visibleScalePart.size() != visibleRect.size())
      {
        m_visibleScalePart = m_visiblePart.scaled(visibleRect.size(), mAspectRatioMode, mTransformationMode);
        if (flipHorz || flipVert)
          m_visibleScalePart = QPixmap::fromImage(m_visibleScalePart.toImage().mirrored(flipHorz, flipVert));
      }

      painter->drawPixmap(visibleRect.topLeft(), m_visibleScalePart);
    }

    // все еще осталась ошибка если повернуть изображение, а затем изменить диапозон осей графика, преобразование будет не корректным.
    // для логарифмических осей ошибка усугубляется, поскольку это уже будет не линейное преобразование,
    // и сохранить соответствие между пиксельным изображением и осями координат невозможно сохраняя прямоугольную форму


    /*painter->drawPixmap(rect.topLeft(), mScaled ? mScaledPixmap : mPixmap);
    pen.setColor(Qt::cyan);
    pen.setWidth(15);
    painter->setPen(pen);
    painter->drawPoint(m_rotateTransform.map(center->pixelPosition()));*/
    /*
    pen.setColor(Qt::red);
    pen.setWidth(4);
    painter->setPen(pen);
    painter->drawText(center->pixelPosition(), QString::number(layer()->index()) + (destination ? " DEST" :""));
    */

    if (selected())
    {
      /*
      painter->setPen(pen);
      painter->setBrush(Qt::NoBrush);
      painter->drawRect(rect);
      const QBrush resizeBrush = QBrush(Qt::white);
      painter->setBrush(resizeBrush);*/

      QPen editPen = QPen(Qt::red);
      editPen.setStyle(Qt::DashLine);
      pen = m_actionMode == EditMode ? editPen : mainPen();
      painter->setPen(pen);
      QRectF innerRect;
      innerRect.setTopLeft(topLeftEditable->pixelPosition());
      innerRect.setBottomRight(bottomRightEditable->pixelPosition());
      painter->drawRect(innerRect);
      const QBrush resizeBrush = QBrush(Qt::white);
      painter->setBrush(resizeBrush);

      QPointF topPoint = topEditable->pixelPosition();
      QPointF roratePoint = rotateAnchor->pixelPosition();

      painter->drawLine(topPoint, roratePoint);
      painter->drawRect(QRectF(roratePoint.x() - GPItemConst::HalfDeltaPixel, roratePoint.y() - GPItemConst::HalfDeltaPixel, GPItemConst::DeltaPixel, GPItemConst::DeltaPixel));

      QList <GPItemAnchor *> positions = {topLeftEditable, bottomRightEditable, topEditable, topRightEditable, rightEditable, bottomEditable, bottomLeftEditable, leftEditable};
      for (const auto position: positions)
      {
        QPointF pos = position->pixelPosition();
        painter->drawRect(QRectF(pos.x() - GPItemConst::HalfDeltaPixel, pos.y() - GPItemConst::HalfDeltaPixel, GPItemConst::DeltaPixel, GPItemConst::DeltaPixel));
      }
    }

      // pen = mainPen();
      // painter->setPen(pen);
      // painter->setBrush(Qt::NoBrush);
      // painter->drawRect(rect);
      // const QBrush resizeBrush = QBrush(Qt::white);
      // painter->setBrush(resizeBrush);
      // QList <GPItemAnchor *> positions = {topLeft, bottomRight, top, topRight, right, bottom, bottomLeft, left};
      // for (const auto position: positions)
      // {
      //   QPointF pos = position->pixelPosition();
      //   painter->drawRect(QRectF(pos.x() - GPItemConst::HalfDeltaPixel, pos.y() - GPItemConst::HalfDeltaPixel, GPItemConst::DeltaPixel, GPItemConst::DeltaPixel));
      // }

/*
      QPointF centerPos = center->pixelPosition();
      QRectF rect;
      rect.setTopLeft(topLeft->pixelPosition());
      rect.setBottomRight(bottomRight->pixelPosition());
      QPolygon polygon = m_rotateTransform.mapToPolygon(rect.toRect());
      QPointF center1;
      for (const QPointF point: polygon)
      {
        center1 += point;
      }
      center1 = center1 / polygon.size();
      if (centerPos != center1)
      {
        centerPos = center1;
      }
      pen.setColor(Qt::red);
      pen.setWidth(15);
      painter->setPen(pen);
      painter->drawPoint(centerPos);
      pen.setColor(Qt::blue);
      pen.setWidth(15);
      painter->setPen(pen);
      painter->drawPoint(center->pixelPosition());
      pen.setColor(Qt::green);
      pen.setWidth(15);
      painter->setPen(pen);
      painter->drawPoint(m_rotationCenter);
    */

    painter->restore();
  }
}

QPointF GPItemInteractivePixmap::anchorPixelPosition(int anchorId) const
{
  QRectF rect;
  rect.setTopLeft(topLeftEditable->pixelPosition());
  rect.setBottomRight(bottomRightEditable->pixelPosition());
  switch(anchorId)
  {
  case aiCenter:
    return (rect.topLeft() + rect.bottomRight()) * 0.5;
  case aiRotate:
    return (rect.topLeft() + rect.topRight()) * 0.5 + (QPointF(0, (rect.top() - rect.bottom()) / abs(rect.height())) * GPItemConst::RotateLineLength);
  case aiTopEditable:
    return (rect.topLeft()+rect.topRight())*0.5;
  case aiTopRightEditable:
    return rect.topRight();
  case aiRightEditable:
    return (rect.topRight()+rect.bottomRight())*0.5;
  case aiBottomEditable:
    return (rect.bottomLeft()+rect.bottomRight())*0.5;
  case aiBottomLeftEditable:
    return rect.bottomLeft();
  case aiLeftEditable:
    return (rect.topLeft()+rect.bottomLeft())*0.5;;
  default:
    return GPItemPixmap::anchorPixelPosition(anchorId);
  }
}

Qt::CursorShape GPItemInteractivePixmap::HitTest(QMouseEvent* event, unsigned* part)
{
  if (!selected())
    return Qt::ArrowCursor;

  int mode = DmNone;
  QPoint pos = m_invertedTransform.map(event->pos());
  QList <GPItemAnchor *> anchors = {topLeftEditable, bottomLeftEditable, topRightEditable, bottomRightEditable, topEditable, bottomEditable, leftEditable, rightEditable, rotateAnchor};
  for (const auto &anchor: anchors)
  {
    QPointF anchorPos = anchor->pixelPosition();
    QRect rect(anchorPos.x()- GPItemConst::HalfDeltaPixel, anchorPos.y() - GPItemConst::HalfDeltaPixel, GPItemConst::DeltaPixel, GPItemConst::DeltaPixel);
    if (rect.contains(pos)) {
      mode = DragModeMap[anchor];
      break;
    }
  }
  if (mode == DmNone)
  {
    QRect rect = getFinalRect();
    if (rect.contains(pos))
      mode = DmMove;
  }
  Qt::CursorShape cursor;
  int rotate = m_rotate;

  bool flipHorz = false;
  bool flipVert = false;
  QRect rect = getFinalRect(&flipHorz, &flipVert);
  auto rotateCursorOfAngle = [this, rotate, flipHorz, flipVert] (Qt::CursorShape cursor) -> Qt::CursorShape
  {
    const QMap <Qt::CursorShape, int> angleMap =
        {
          {Qt::SizeFDiagCursor, 135},
          {Qt::SizeVerCursor, 90},
          {Qt::SizeBDiagCursor, 45},
          {Qt::SizeHorCursor, 0},
        };
    const int halfStep = 45 / 2.;
    int angle = (int)(angleMap[cursor] + m_rotate);
    if (flipHorz)
    {
      angle = 180 - angle;
    }
    if (flipVert)
    {
      angle = - angle;
    }
    angle = (360 + angle + halfStep) % 180 - halfStep ; // make angle from -22.5 - 157.5°

    for(auto it = angleMap.begin(); it != angleMap.end(); it++)
    {
      if (it.value() + halfStep > angle && it.value() - halfStep <= angle)
        return it.key();
    }
    return Qt::SizeAllCursor;
  };

  switch(mode)
  {
  case DmMove:
    cursor = Qt::SizeAllCursor;
    break;
  case DmResizeTopLeft:
  case DmResizeBottomRight:
    cursor = rotateCursorOfAngle(Qt::SizeFDiagCursor);
    break;
  case DmResizeTopRight:
  case DmResizeBottomLeft:
    cursor = rotateCursorOfAngle(Qt::SizeBDiagCursor);
    break;
  case DmResizeTop:
  case DmResizeBottom:
    cursor = rotateCursorOfAngle(Qt::SizeVerCursor);
    break;
  case DmResizeLeft:
  case DmResizeRight:
    cursor = rotateCursorOfAngle(Qt::SizeHorCursor);
    break;
  case DmRotate:
    cursor = Qt::SizeAllCursor;
    break;
  case DmNone:
  default:
    cursor = Qt::ArrowCursor;
  }
  if (part)
    *part = mode;

  return cursor;
}

double GPItemInteractivePixmap::opacity() const
{
  return m_opacity;
}

void GPItemInteractivePixmap::setOpacity(double opacity)
{
  if (qFuzzyCompare(m_opacity, opacity))
    return;
  m_opacity = opacity;
  // emit opacityChanged(m_opacity);//TODO: выяснить, почему этого не было, раскомментить если надо
  emit replot();
}

void GPItemInteractivePixmap::resetOpacity()
{
  setOpacity(1.0);
}
