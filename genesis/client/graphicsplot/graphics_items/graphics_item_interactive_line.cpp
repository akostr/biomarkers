#include "graphics_item_interactive_line.h"

GPItemInteractiveLine::GPItemInteractiveLine(GraphicsPlot *parentPlot)
  : GPItemLine{ parentPlot }
  , CommonItemFuctional{ parentPlot }
{
  setInteractive(true);
  setClipToAxisRect(true);

  connect(this, &GPItemInteractiveLine::DragMoved, this, &GPItemInteractiveLine::move);
  connect(this, &GPItemInteractiveLine::DragAccepted, this, &GPItemInteractiveLine::move);
}

void GPItemInteractiveLine::setStartCoord(QPointF coord)
{
  if (start->coords() != coord)
  {
    start->setCoords(coord);
    emit startCoordinateChanged(coord);
  }
}

void GPItemInteractiveLine::setStartPixel(QPointF pixel)
{
  setStartCoord(pixelToCoord(pixel));
}

void GPItemInteractiveLine::setEndCoord(QPointF coord)
{
  if (end->coords() != coord)
  {
    end->setCoords(coord);
    emit endCoordinateChanged(coord);
  }
}

void GPItemInteractiveLine::setEndPixel(QPointF pixel)
{
  setEndCoord(pixelToCoord(pixel));
}

void GPItemInteractiveLine::move(QPointF deltaMoving)
{
  QPointF oldStart = start->pixelPosition();
  QPointF oldEnd = end->pixelPosition();
  switch(mDraggingPart)
  {
  case DmMoveStart:
  {
    setStartPixel(oldStart + deltaMoving);
    break;
  }
  case DmMoveEnd:
  {
    setEndPixel(oldEnd + deltaMoving);
    break;
  }
  case DmMove:
  {
    setStartPixel(oldStart + deltaMoving);
    setEndPixel(oldEnd + deltaMoving);
    break;
  }
  case DmNone:
  default:
    break;
  }
}

void GPItemInteractiveLine::draw(GPPainter *painter)
{
  GPItemLine::draw(painter);
  if (mSelected)
  {
    painter->setBrush(Qt::NoBrush);
    painter->setPen(Qt::black);
    QPointF startPos = start->pixelPosition();
    QRect startRect;
    startRect.setTopLeft(QPoint(startPos.x() - GPItemConst::DeltaPixel,
                                startPos.y() + GPItemConst::DeltaPixel));
    startRect.setBottomRight(QPoint(startPos.x() + GPItemConst::DeltaPixel,
                                    startPos.y() - GPItemConst::DeltaPixel));
    painter->drawRect(startRect);

    QPointF endPos = end->pixelPosition();
    QRect endRect;
    endRect.setTopLeft(QPoint(endPos.x() - GPItemConst::DeltaPixel,
                              endPos.y() + GPItemConst::DeltaPixel));
    endRect.setBottomRight(QPoint(endPos.x() + GPItemConst::DeltaPixel,
                                  endPos.y() - GPItemConst::DeltaPixel));
    painter->drawRect(endRect);
  }
}

Qt::CursorShape GPItemInteractiveLine::HitTest(QMouseEvent* event, unsigned* part)
{
  if (!selected())
    return Qt::ArrowCursor;

  GPVector2D posVec(event->pos());
  QPointF startVec(start->pixelPosition());
  QPointF endVec(end->pixelPosition());
  Qt::CursorShape cursor;
  int mode = DmNone;
  QRect startRect;
  startRect.setTopLeft(QPoint(startVec.x() - GPItemConst::DeltaPixel,
                              startVec.y() + GPItemConst::DeltaPixel));
  startRect.setBottomRight(QPoint(startVec.x() + GPItemConst::DeltaPixel,
                                  startVec.y() - GPItemConst::DeltaPixel));
  QRect endRect;
  endRect.setTopLeft(QPoint(endVec.x() - GPItemConst::DeltaPixel,
                            endVec.y() + GPItemConst::DeltaPixel));
  endRect.setBottomRight(QPoint(endVec.x() + GPItemConst::DeltaPixel,
                                endVec.y() - GPItemConst::DeltaPixel));

  if (startRect.contains(posVec.toPoint()))
  {
    mode = DmMoveStart;
    cursor = Qt::SizeHorCursor;
  }
  else if (endRect.contains(posVec.toPoint()))
  {
    mode = DmMoveEnd;
    cursor = Qt::SizeVerCursor;
  }
  else if (posVec.distanceSquaredToLine(startVec, endVec) < GPItemConst::DeltaPixel)
  {
    mode = DmMove;
    cursor = Qt::SizeAllCursor;
  }
  else
  {
    mode = DmNone;
    cursor = Qt::ArrowCursor;
  }

  if (part)
    *part = mode;

  return cursor;
}
