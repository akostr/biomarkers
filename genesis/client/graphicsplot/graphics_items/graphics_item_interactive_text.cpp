#include "graphics_item_interactive_text.h"
#include <QVector2D>

GPItemInteractiveText::GPItemInteractiveText(GraphicsPlot *parentPlot)
  : GPItemText{ parentPlot }
  , CommonItemFuctional{ parentPlot }
  , rotateAnchor(createAnchor(QLatin1String("rotate"), aiRotate))
{
  setSelectable(true);
  const QPen OkPen = QPen(QColor(0x00,0x91,0xFF), 2);
  setSelectedPen(OkPen);
  connect(this, &GPItemInteractiveText::DragMoved, this, &GPItemInteractiveText::move);
  connect(this, &GPItemInteractiveText::DragAccepted, this, &GPItemInteractiveText::move);
  connect (this, &GPItemInteractiveText::replot, parentPlot,
          [parentPlot]()
          {
            parentPlot->replot(GraphicsPlot::RefreshPriority::rpQueuedRefresh);
          });
  setInteractive(true);
  setTransform(position->pixelPosition(), 0);
  setClipToAxisRect(true);
}

double GPItemInteractiveText::selectTest(const QPointF &pos, bool onlySelectable, QVariant *details) const
{
  if (onlySelectable && !mSelectable)
    return -1;

  double distance = GPItemText::selectTest(pos, onlySelectable, details);

  if (selected())
  {
    QPointF delta = rotateAnchor->pixelPosition() - pos;
    double rotatePointDistance = qSqrt(delta.x()*delta.x() + delta.y()*delta.y());
    distance = qMin(distance, rotatePointDistance);
  }
}

void GPItemInteractiveText::movePosition(QPointF delta)
{
  QPointF pixelPos = position->pixelPosition();
  switch(mDraggingPart)
  {
  case DmMove:
  {
    setPosition(pixelPos + delta);
    break;
  }
  case DmRotate:
  {
    m_rotationCenter = pixelPos;
    const QPointF centerPos = m_rotationCenter;
    const QPointF pos = m_previousDragPos + delta;
    const QLineF line(centerPos, pos);
    const qreal angle = 180 - QLineF(centerPos, QPointF(centerPos.x(), centerPos.y() + 1)).angleTo(line);
    setTransform(m_rotationCenter, angle);
    break;
  }
  }
}

void GPItemInteractiveText::setPosition(QPointF newPos)
{
  setPositionAsCoordinate(pixelToCoord(newPos));
}

void GPItemInteractiveText::setPositionAsCoordinate(QPointF newPos)
{
  if (position->coords() != newPos)
  {
    position->setCoords(newPos);
    emit positionChanged(newPos);
    emit replot();
  }
}

void GPItemInteractiveText::setVisible(bool isVisible)
{
  if (mVisible != isVisible)
  {
    mVisible = isVisible;
    emit replot();
  }
}

void GPItemInteractiveText::move(QPointF deltaMoving)
{
  movePosition(deltaMoving);
}

void GPItemInteractiveText::draw(GPPainter *painter)
{
  if(selected())
  {
    painter->setPen(mainPen());
    painter->setBrush(mainBrush());
    QPointF topPoint = top->pixelPosition();
    QPointF roratePoint = rotateAnchor->pixelPosition();
    QPointF topLeftPoint = topLeft->pixelPosition();
    QPointF bottomRightPoint = bottomRight->pixelPosition();
    painter->drawLine(topPoint, roratePoint);
    painter->drawRect(QRectF(roratePoint.x() - GPItemConst::HalfDeltaPixel,
                             roratePoint.y() - GPItemConst::HalfDeltaPixel,
                             GPItemConst::DeltaPixel,
                             GPItemConst::DeltaPixel));
  }
  GPItemText::draw(painter);
}

Qt::CursorShape GPItemInteractiveText::HitTest(QMouseEvent* event, unsigned* part)
{
  if (!selected())
    return Qt::ArrowCursor;

  QPoint pos = m_invertedTransform.map(event->pos());

  QPointF pixcelPosition(position->pixelPosition());
  QTransform transform;
  transform.translate(pixcelPosition.x(), pixcelPosition.y());
  if (!qFuzzyIsNull(mRotation))
    transform.rotate(mRotation);
  QFontMetrics fontMetrics(mainFont());
  QRect textRect = fontMetrics.boundingRect(0, 0, 0, 0, Qt::TextDontClip|mTextAlignment, mText);
  QRectF textBoxRect = textRect.adjusted(-mPadding.left(), -mPadding.top(), mPadding.right(), mPadding.bottom());
  QPointF textPos = getTextDrawPoint(QPointF(0, 0), textBoxRect, mPositionAlignment); // 0, 0 because the transform does the translation
  textBoxRect.moveTopLeft(textPos.toPoint());
  QPolygonF rectPoly = transform.map(QPolygonF(textBoxRect));

  Qt::CursorShape cursor;
  int mode = DmNone;
  if (rectPoly.containsPoint(pos, Qt::OddEvenFill))
  {
    mode = DmMove;
    cursor = Qt::SizeAllCursor;
  }
  else
  {
    QPointF rotatePos = rotateAnchor->pixelPosition();
    QRect rect(rotatePos.x()- GPItemConst::HalfDeltaPixel, rotatePos.y() - GPItemConst::HalfDeltaPixel, GPItemConst::DeltaPixel, GPItemConst::DeltaPixel);
    if (rect.contains(pos)) {
      mode = DmRotate;
      cursor = Qt::SizeAllCursor;
    }
    else
    {
      mode = DmNone;
      cursor = Qt::ArrowCursor;
    }
  }
  if (part)
    *part = mode;

  return cursor;
}

QPointF GPItemInteractiveText::anchorPixelPosition(int anchorId) const
{
  switch(anchorId)
  {
  case aiRotate:
  {
    QPointF top = anchorPixelPosition(aiTop);
    QPointF bottom = anchorPixelPosition(aiBottom);
    QPointF vector = top - bottom;
    QPointF normalizedVector = vector / (sqrt(vector.x() * vector.x() + vector.y() * vector.y()));
    return top + normalizedVector * GPItemConst::RotateLineLength;
  }
  default:
    return GPItemText::anchorPixelPosition(anchorId);
  }
}

void GPItemInteractiveText::setTransform(QPointF center, double moveRotationAngle)
{
  setRotation(moveRotationAngle);
  emit rotateChanged(moveRotationAngle);
  emit replot();
}
