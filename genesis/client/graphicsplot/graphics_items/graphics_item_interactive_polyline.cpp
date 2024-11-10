#include "graphics_item_interactive_polyline.h"

QPointF prevDir;
QPointF dir;

GraphicsItemInteractivePolyline::GraphicsItemInteractivePolyline(GraphicsPlot *parentPlot)
  : GPAbstractItem(parentPlot)
  , GPEXDraggingInterface()
  , center(createAnchor(QLatin1String("center"), aiCenter))
  , rotateAnchor(createAnchor(QLatin1String("rotate"), aiRotate))
  , topLeft(createAnchor(QLatin1String("topLeft"), aiTopLeft))
  , top(createAnchor(QLatin1String("top"), aiTop))
  , topRight(createAnchor(QLatin1String("topRight"), aiTopRight))
  , right(createAnchor(QLatin1String("right"), aiRight))
  , bottomRight(createAnchor(QLatin1String("bottomRight"), aiBottomRight))
  , bottom(createAnchor(QLatin1String("bottom"), aiBottom))
  , bottomLeft(createAnchor(QLatin1String("bottomLeft"), aiBottomLeft))
  , left(createAnchor(QLatin1String("left"), aiLeft))
  , mPoints(new QList<GPVector2D>())
  , mPen(Qt::black)
  , mBrush(Qt::white, Qt::NoBrush)
  , mLineEnding(GPLineEnding::EndingStyle::esNone)
{
  setSelectable(true);
  mDragModeMap = {{aiTopLeft, DmResizeTopLeft},
                 {aiTop, DmResizeTop},
                 {aiTopRight, DmResizeTopRight},
                 {aiRight, DmResizeRight},
                 {aiBottomRight, DmResizeBottomRight},
                 {aiBottom, DmResizeBottom},
                 {aiBottomLeft, DmResizeBottomLeft},
                 {aiLeft, DmResizeLeft},
                 {aiRotate, DmRotate}};

  connect (this, &GraphicsItemInteractivePolyline::replot, parentPlot,
          [parentPlot]()
          {
            parentPlot->replot(GraphicsPlot::RefreshPriority::rpQueuedRefresh);
          });
  setInteractive(true);
}

void GraphicsItemInteractivePolyline::FillContextMenu(QContextMenuEvent *event, QMenu &menu, std::map<QAction *, std::function<void ()> > &actions)
{
  connect(menu.addAction("Do nothing"), &QAction::triggered, this, [](){qDebug() << "do nothing";});
}

void GraphicsItemInteractivePolyline::setPoints(const QList<GPVector2D> &points)
{
  bool different = false;
  if(points.size() != mPoints->size())
  {
    different = true;
  }
  else
  {
    for(int i = 0; i < mPoints->size(); i++)
    {
      if(!qFuzzyCompare(mPoints->at(i).x(), points[i].x()))
      {
        different = false;
        break;
      }
      if(!qFuzzyCompare(mPoints->at(i).y(), points[i].y()))
      {
        different = false;
        break;
      }
    }
  }
  if(different)
  {
    *mPoints = points;
    rebuildSegments();
    emit pointsChanged();
  }
}

void GraphicsItemInteractivePolyline::insertPoint(int beforeInd, const GPVector2D &point)
{
  if(mPoints->isEmpty())
  {
    mPoints->append(point);
    emit pointsChanged();
    emit replot();
    return;
  }
  if(beforeInd >= mPoints->size())
  {
    auto startInd = mPoints->size() - 1;
    mPoints->append(point);
    PolySegment newSegment(mPoints, startInd, startInd + 1);
    mSegments.append(newSegment);
    emit pointsChanged();
    emit replot();
    return;
  }
  else if(beforeInd <= 0)
  {
    mPoints->prepend(point);
    PolySegment newSegment(mPoints, 0, 1);
    mSegments.prepend(newSegment);
    beforeInd = 1;
  }
  else
  {
    auto startInd = beforeInd - 1;
    auto endInd = beforeInd;
    mPoints->insert(beforeInd, point);
    PolySegment newSegment(mPoints, startInd, endInd);
    mSegments.insert(beforeInd - 1, newSegment);
  }

  for(int i = beforeInd; i < mSegments.size(); i++)
  {
    mSegments[i].startInd++;
    mSegments[i].endInd++;
  }
  //@TODO: recalculate segments directions as well
  emit pointsChanged();
  emit replot();
}

void GraphicsItemInteractivePolyline::prependPoint(const GPVector2D &point)
{
  insertPoint(0, point);
}

void GraphicsItemInteractivePolyline::appendPoint(const GPVector2D &point)
{
  insertPoint(mPoints->size(), point);
}

void GraphicsItemInteractivePolyline::rebuildSegments()
{
  mSegments.clear();
  for(auto it = mPoints->begin(); it != mPoints->end(); it++)
  {
    if(std::next(it) == mPoints->end())
      break;
    int ind = std::distance(mPoints->begin(), it);
    mSegments.emplaceBack(mPoints, ind, ind + 1);
  }
  calcPath();
  emit replot();
}

bool GraphicsItemInteractivePolyline::isClosed() const
{
  if(mSegments.isEmpty())
    return false;
  return mSegments.first().startInd == mSegments.last().endInd;
}

void GraphicsItemInteractivePolyline::closeLine(bool closed)
{
  if(mSegments.isEmpty())
    return;
  if(!isClosed() && closed == true)
  {
    PolySegment lastSegment(mPoints, mSegments.last().endInd, mSegments.first().startInd);
    mSegments.append(lastSegment);
    mBrush.setStyle(Qt::SolidPattern);
  }
  else if(isClosed() && closed == false)
  {
    mSegments.removeLast();
    mBrush.setStyle(Qt::NoBrush);
  }
  else
  {
    return;
  }
  emit replot();
  emit lineClosedStateChanged();
}

QSharedPointer<QList<GPVector2D> > GraphicsItemInteractivePolyline::points()
{
  return mPoints;
}

QRectF GraphicsItemInteractivePolyline::boundingRect() const
{
  if(mPoints->isEmpty())
    return QRectF();
  auto& firstPt = mPoints->first();
  GPRange xRange(firstPt.x(), firstPt.x());
  GPRange yRange(firstPt.y(), firstPt.y());
  for(auto& pt : *mPoints)
  {
    xRange.expand(pt.x());
    yRange.expand(pt.y());
  }
  return QRectF(xRange.lower, yRange.upper, xRange.size(), -yRange.size());
}

void GraphicsItemInteractivePolyline::movePos(QPointF pixelDelta)
{
  for(auto& pt : *mPoints)
    pt = pixelToCoord(coordToPixel(pt) + pixelDelta);
  emit replot();
}

void GraphicsItemInteractivePolyline::scale(QPointF coordOriginPt, double scaleX, double scaleY)
{
  QTransform worldToLocal;
  QTransform scaling;
  QTransform localToWorld;
  worldToLocal.translate(-coordOriginPt.x(), -coordOriginPt.y());
  scaling.scale(scaleX, scaleY);
  localToWorld.translate(coordOriginPt.x(), coordOriginPt.y());
  for(auto& pt : *mPoints)
  {
    QPointF p = pt.toPointF();
    p = p * worldToLocal * scaling * localToWorld;
    pt = {p};
  }
}

void GraphicsItemInteractivePolyline::scaleByDelta(GPItemAnchor *originAnchor, GPItemAnchor *draggingAnchor, QPointF pixelDelta, bool scaleX, bool scaleY)
{
  auto dragStartPos = draggingAnchor->pixelPosition();
  auto originPos = originAnchor->pixelPosition();
  auto dragEndPos = dragStartPos + pixelDelta;
  auto originScale = dragStartPos - originPos;
  auto deltaScale = dragEndPos - originPos;
  double xScale = scaleX ? deltaScale.x() / originScale.x() : 1.0;
  double yScale = scaleY ? deltaScale.y() / originScale.y() : 1.0;
  scale(pixelToCoord(originAnchor->pixelPosition()), xScale, yScale);
}

void GraphicsItemInteractivePolyline::rotateInCoord(QPointF coordOriginPt, double deg)
{
  QTransform worldToLocal;
  QTransform rotating;
  QTransform localToWorld;
  worldToLocal.translate(-coordOriginPt.x(), -coordOriginPt.y());
  rotating.rotate(deg);
  localToWorld.translate(coordOriginPt.x(), coordOriginPt.y());
  for(auto& pt : *mPoints)
  {
    QPointF p = pt.toPointF();
    p = p * worldToLocal * rotating * localToWorld;
    pt = {p};
  }
}

void GraphicsItemInteractivePolyline::rotateInPixel(QPointF pixelOriginPt, double deg)
{
  QTransform worldToLocal;
  worldToLocal.translate(-pixelOriginPt.x(), -pixelOriginPt.y());
  QTransform rotating;
  rotating.rotate(deg);
  QTransform localToWorld;
  localToWorld.translate(pixelOriginPt.x(), pixelOriginPt.y());
  QTransform transform = worldToLocal * rotating * localToWorld;

  for(auto& pt : *mPoints)
  {
    QPointF p = coordToPixel(pt.toPointF());
    p = p * transform;
    pt = pixelToCoord(p);
  }
}

void GraphicsItemInteractivePolyline::setVisible(bool isVisible)
{
  if (mVisible != isVisible)
  {
    mVisible = isVisible;
    emit replot();
  }
}

GPLineEnding GraphicsItemInteractivePolyline::lineEnding() const
{
  return mLineEnding;
}

void GraphicsItemInteractivePolyline::setLineEnding(const GPLineEnding &newLineEnding)
{
  if(mLineEnding.style() == newLineEnding.style())
    return;
  mLineEnding = newLineEnding;
  emit replot();
  emit lineEndingChanged();
}

void GraphicsItemInteractivePolyline::resetLineEnding()
{
  setLineEnding({}); // TODO: Adapt to use your actual default value
}

QBrush GraphicsItemInteractivePolyline::brush() const
{
  return mBrush;
}

void GraphicsItemInteractivePolyline::setBrush(const QBrush &newBrush)
{
  if (mBrush == newBrush)
    return;
  mBrush = newBrush;
  emit replot();
  emit brushChanged();
}

void GraphicsItemInteractivePolyline::resetBrush()
{
  setBrush({}); // TODO: Adapt to use your actual default value
}

QPen GraphicsItemInteractivePolyline::pen() const
{
  return mPen;
}

void GraphicsItemInteractivePolyline::setPen(const QPen &newPen)
{
  if (mPen == newPen)
    return;
  mPen = newPen;
  emit replot();
  emit penChanged();
}

void GraphicsItemInteractivePolyline::resetPen()
{
  setPen({}); // TODO: Adapt to use your actual default value
}

void GraphicsItemInteractivePolyline::DragStart(QMouseEvent *event, unsigned int part)
{
  m_draggingSavedInd = -1;
  mDragStartPosPixel = event->position();
  mDragPrevPosPixel = mDragStartPosPixel;
  mDraggingPart = part;
  if (part == DmRotate)
  {
    mDragRotateCenterCoord = boundingRect().center();
  }
  else if(part == DmPoint || part == DmLine)
  {
    m_draggingSavedInd = m_draggingInd;
  }
}

void GraphicsItemInteractivePolyline::DragMove(QMouseEvent *event)
{
  mDragCurrentPosPixel = event->position();
  mDragDeltaPixel = mDragCurrentPosPixel - mDragPrevPosPixel;
  move(mDragDeltaPixel);
  mDragPrevPosPixel = mDragCurrentPosPixel;
}

void GraphicsItemInteractivePolyline::DragAccept(QMouseEvent *event)
{
  if(mDraggingPart != DmNone)
    emit pointsChanged();
}

void GraphicsItemInteractivePolyline::move(QPointF pixelDelta)
{
  // if (m_actionMode == EditMode)
  //   /*editPosition(deltaMoving)*/;
  // else
  //   movePosition(pixelDelta);
  movePosition(pixelDelta);
}

void GraphicsItemInteractivePolyline::movePosition(QPointF pixelDelta)
{
  switch(mDraggingPart)
  {
  case DmPoint:
  {
    auto& pt = (*mPoints)[m_draggingSavedInd];
    pt = pixelToCoord(coordToPixel(pt) + pixelDelta);
    emit replot();
    break;

  }
  case DmLine:
  {
    const auto& segment = mSegments.at(m_draggingSavedInd);
    auto& start = (*mPoints)[segment.startInd];
    auto& end = (*mPoints)[segment.endInd];
    start = pixelToCoord(coordToPixel(start) + pixelDelta);
    end = pixelToCoord(coordToPixel(end) + pixelDelta);
    emit replot();
    break;
  }
  case DmMove:
  {
    movePos(pixelDelta);
    break;
  }
  case DmResizeTopLeft:
  {
    scaleByDelta(bottomRight, topLeft, pixelDelta);
    emit replot();
    break;
  }
  case DmResizeBottomRight:
  {
    scaleByDelta(topLeft, bottomRight, pixelDelta);
    emit replot();
    break;
  }
  case DmResizeTopRight:
  {
    scaleByDelta(bottomLeft, topRight, pixelDelta);
    emit replot();
    break;
  }
  case DmResizeBottomLeft:
  {
    scaleByDelta(topRight, bottomLeft, pixelDelta);
    emit replot();
    break;
  }
  case DmResizeTop:
  {
    scaleByDelta(bottom, top, pixelDelta, false);
    emit replot();
    break;
  }
  case DmResizeBottom:
  {
    scaleByDelta(top, bottom, pixelDelta, false);
    emit replot();
    break;
  }
  case DmResizeLeft:
  {
    scaleByDelta(right, left, pixelDelta, true, false);
    emit replot();
    break;
  }
  case DmResizeRight:
  {
    scaleByDelta(left, right, pixelDelta, true, false);
    emit replot();
    break;
  }
  case DmRotate:
  {
    auto rotOrigin = coordToPixel(mDragRotateCenterCoord);
    GPVector2D prevDirection = mDragCurrentPosPixel - pixelDelta - rotOrigin;
    GPVector2D direction = mDragCurrentPosPixel - rotOrigin;
    prevDirection.normalize();
    direction.normalize();
    prevDir = prevDirection.toPointF();
    dir = direction.toPointF();
    QLineF prevDirLine(QPointF(), prevDirection.toPointF());
    QLineF directionLine(QPointF(), direction.toPointF());
    double angleDelta = prevDirLine.angle() - directionLine.angle();
    qDebug() << angleDelta;
    rotateInPixel(rotOrigin, angleDelta);
    break;
  }
  case DmNone:
  default:
    break;
  }
}

QPointF GraphicsItemInteractivePolyline::anchorPixelPosition(int anchorId) const
{
  QRectF rect = coordToPixel(boundingRect());
  auto center = rect.center();
  switch(anchorId)
  {
  case aiTopLeft:             return (rect.topLeft());
  case aiTop:                 return (QPointF{center.x(), rect.top()});
  case aiTopRight:            return (rect.topRight());
  case aiRight:               return (QPointF{rect.right(), center.y()});
  case aiBottomRight:         return (rect.bottomRight());
  case aiBottom:              return (QPointF{center.x(), rect.bottom()});
  case aiBottomLeft:          return (rect.bottomLeft());
  case aiLeft:                return (QPointF{rect.left(), center.y()});
  case aiCenter:              return (rect.center());
  case aiRotate:              return (QPointF{center.x(), rect.top()} + (QPointF(0, (rect.top() - rect.bottom()) / abs(rect.height())) * GPItemConst::RotateLineLength));
  }
  qDebug() << Q_FUNC_INFO << "invalid anchorId" << anchorId;
  return QPointF();
}

QPainterPath GraphicsItemInteractivePolyline::calcPath() const
{
  QPainterPath path;
  for(const auto& segment : mSegments)
  {
    auto pixelStart = coordToPixel(mPoints->at(segment.startInd));
    auto pixelEnd = coordToPixel(mPoints->at(segment.endInd));
    auto pixelStartDir = pixelStart + segment.startDir;
    auto pixelEndDir = pixelEnd + segment.endDir;
    if(path.isEmpty())
      path.moveTo(pixelStart.toPointF());
    path.cubicTo(pixelStartDir.toPointF(), pixelEndDir.toPointF(), pixelEnd.toPointF());
  }
  return path;
}

double GraphicsItemInteractivePolyline::selectTest(const QPointF &pixelPos, bool onlySelectable, QVariant *details) const
{
  if(onlySelectable && !mSelectable)
    return -1;
  if(selected())
  {
    QVariant localDetails;
    double dist = anchorsSelectTest(pixelPos, &localDetails);
    if(dist >= 0 && dist < mParentPlot->selectionTolerance())
    {
      if(details)
        *details = localDetails;
      return dist;
    }
  }
  double minDist = 0;
  int ind = 0;
  QVariant closestDetails;
  if(mSegments.isEmpty())
  {
    if(!mPoints->isEmpty())
    {
      minDist = (coordToPixel(mPoints->first()) - pixelPos).toPointF().manhattanLength();
      ind = 0;
      for(auto it = mPoints->begin(); it != mPoints->end(); it++)
      {
        double dist = (coordToPixel(*it) - pixelPos).toPointF().manhattanLength();
        if(dist < minDist)
        {
          minDist = dist;
          ind = std::distance(mPoints->begin(), it);
        }
      }
      if(details)
        *details = QVariant::fromValue(SelectTestDetails{PLSPPoint, ind});
      return minDist;
    }
  }
  else
  {
    auto segmentIt = mSegments.begin();
    for(auto it = mSegments.begin(); it != mSegments.end(); it++)
    {
      QVariant localDetails;
      auto dist = it->selectTest(pixelPos, this, mParentPlot->selectionTolerance(), &localDetails);
      if(it == mSegments.begin())
      {
        minDist = dist;
        closestDetails = localDetails;
        ind = 0;
        segmentIt = it;
      }
      else if(dist > 0 && (minDist > dist || minDist < 0))
      {//if we have proper dist AND this dist lesser than minDist OR minDist is not valid
        minDist = dist;
        closestDetails = localDetails;
        ind = std::distance(mSegments.begin(), it);
        segmentIt = it;
      }
    }
    auto part = closestDetails.value<PolySegment::SelectPart>();
    switch(part)
    {
    case PolySegment::PSSPStart:
    {
      if(details)
        *details = QVariant::fromValue(SelectTestDetails{PLSPPoint, segmentIt->startInd});
      return minDist;
    }
    case PolySegment::PSSPLine:
    {
      if(details)
        *details = QVariant::fromValue(SelectTestDetails{PLSPLine, ind});
      return minDist;
    }
    case PolySegment::PSSPEnd:
    {
      if(details)
        *details = QVariant::fromValue(SelectTestDetails{PLSPPoint, segmentIt->endInd});
      return minDist;
    }
    case PolySegment::PSSPNone:
    {
      if(isClosed() && calcPath().contains(pixelPos))
      {
        if(details)
          *details = QVariant::fromValue(SelectTestDetails{PLSPPolygon, -1});
        return 0;
      }
    }
    default:
      return minDist;
    }
  }
  return minDist;
}

double GraphicsItemInteractivePolyline::anchorsSelectTest(const QPointF &pixelPos, QVariant *details) const
{
  QList <int> anchors = {aiTopLeft, aiTop, aiTopRight, aiRight, aiBottomRight, aiBottom, aiBottomLeft, aiLeft, aiRotate};
  QRect rect(0, 0, GPItemConst::DeltaPixel, GPItemConst::DeltaPixel);
  for (const auto &anchor: anchors)
  {
    auto center = anchorPixelPosition(anchor);
    rect.moveCenter(center.toPoint());
    if (rect.contains(pixelPos.toPoint()))
    {
      if(details)
        *details = QVariant::fromValue(SelectTestDetails{PLSPAnchor, anchor});
      return 0;
    }
  }
  return -1;
}



void GraphicsItemInteractivePolyline::draw(GPPainter *painter)
{
  if(selected())
  {//draw points selection decorations
    painter->save();
    painter->setPen(Qt::blue);
    painter->setBrush(Qt::white);
    QRect ptRect(-5, -5, 10, 10);
    for(const auto& pt : *mPoints)
    {
      ptRect.moveCenter(coordToPixel(pt.toPointF()).toPoint());
      painter->drawRect(ptRect);
    }
    painter->restore();
  }

  painter->save();
  {//draw polyline

    QRect clip = clipRect();
    auto path = calcPath();
    QRect cubicRect = path.controlPointRect().toRect();
    if (cubicRect.isEmpty()) // may happen when start and end exactly on same x or y position
      cubicRect.adjust(0, 0, 1, 1);
    if (clip.intersects(cubicRect))
    {
      painter->setPen(mPen);
      painter->setBrush(mBrush);
      painter->drawPath(path);

      if(mLineEnding.style() != GPLineEnding::esNone)
      {
        auto& segment = mSegments.last();
        auto pixelStart = coordToPixel(mPoints->at(segment.startInd));
        auto pixelEnd = coordToPixel(mPoints->at(segment.endInd));
        auto pen = mPen;
        pen.setStyle(Qt::SolidLine);
        painter->save();
        painter->setPen(pen);
        mLineEnding.draw(painter, pixelEnd, pixelEnd - pixelStart);
        painter->restore();
        // mLineEnding.draw(painter, pixelEnd, -path.angleAtPercent(1)/180.0*M_PI);
      }
    }
  }
  painter->restore();

  if(selected())
  {//draw selection decorations
    //rect controls
    drawBoundingRect(painter);
    drawAnchors(painter);
  }
}

void GraphicsItemInteractivePolyline::drawAnchors(GPPainter *painter)
{
  painter->save();
  painter->setPen(Qt::black);
  painter->setBrush(Qt::blue);
  QList <int> anchors = {aiTopLeft, aiTop, aiTopRight, aiRight, aiBottomRight, aiBottom, aiBottomLeft, aiLeft, aiRotate};
  QRect rect(0, 0, GPItemConst::DeltaPixel, GPItemConst::DeltaPixel);
  rect.moveCenter({0,0});
  for (const auto &anchor: anchors)
  {
    painter->save();
    auto center = anchorPixelPosition(anchor);
    painter->translate(center);
    painter->drawRect(rect);
    painter->restore();
  }
  painter->restore();
}

void GraphicsItemInteractivePolyline::drawBoundingRect(GPPainter *painter)
{
  painter->save();
  auto br = coordToPixel(boundingRect());
  painter->setPen(Qt::black);
  painter->setBrush(Qt::NoBrush);
  painter->drawRect(br);
  painter->restore();
}

Qt::CursorShape GraphicsItemInteractivePolyline::HitTest(QMouseEvent *event, unsigned int *part)
{
  if (!selected())
    return Qt::ArrowCursor;

  m_draggingInd = -1;
  int mode = GPItemConst::DmNone;
  QPoint pos = event->pos();
  if(selected())
  {
    QVariant details;
    auto dist = anchorsSelectTest(pos, &details);
    if(dist >= 0 && dist < mParentPlot->selectionTolerance())
    {
      mode = mDragModeMap[details.value<SelectTestDetails>().index];
    }
  }
  Qt::CursorShape cursor;
  if (mode == DmNone)
  {
    QRectF pixelBoundingRect;
    {
      auto coordsRect = boundingRect();
      auto topLeft = coordsRect.topLeft();
      auto bottomRight = coordsRect.bottomRight();
      auto pixelTL = coordToPixel(topLeft);
      auto pixelBR = coordToPixel(bottomRight);
      pixelBoundingRect = QRectF(pixelTL, pixelBR);
    }
    if (pixelBoundingRect.contains(pos))
    {
      QVariant details;
      auto dist = selectTest(pos, false, &details);
      SelectTestDetails selectionDetails = details.value<SelectTestDetails>();
      if(dist <= mParentPlot->selectionTolerance())
      {
        switch(selectionDetails.part)
        {
        case PLSPPoint:
          mode = DmPoint;
          cursor = Qt::SizeAllCursor;
          m_draggingInd = selectionDetails.index;
          break;
        case PLSPLine:
          mode = DmLine;
          cursor = Qt::CrossCursor;
          m_draggingInd = selectionDetails.index;
          break;
        case PLSPPolygon:
          mode = DmMove;
          cursor = Qt::SizeAllCursor;
          break;
        case PLSPNone:
          mode = DmMove;
          cursor = Qt::SizeAllCursor;
          break;
        case PLSPAnchor:
          break;
        }
        if(mode != DmNone)
        {
          if(part)
            *part = mode;
          return cursor;
        }
      }
      else
      {
        mode = DmMove;
      }
    }
  }

  auto rotateCursorOfAngle = [this] (Qt::CursorShape cursor) -> Qt::CursorShape
  {
    const QMap <Qt::CursorShape, int> angleMap =
        {
         {Qt::SizeFDiagCursor, 135},
         {Qt::SizeVerCursor, 90},
         {Qt::SizeBDiagCursor, 45},
         {Qt::SizeHorCursor, 0},
         };
    const int halfStep = 45 / 2.;
    int angle = (int)(angleMap[cursor]/* + m_rotate*/);
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

PolySegment::PolySegment(QSharedPointer<QList<GPVector2D> > points, int startIndex, int endIndex, std::optional<GPVector2D> startDirection, std::optional<GPVector2D> endDirection)
  : pointsArrayPtr(points),
  startInd(startIndex),
  endInd(endIndex),
  startDir(GPVector2D()),
  endDir(GPVector2D())
{}

double PolySegment::selectTest(const QPointF &pixelPos, const GraphicsItemInteractivePolyline* item, double selectionTolerance, QVariant *details) const
{
  if(isIndexesValid())
  {
    auto start = item->coordToPixel(pointsArrayPtr->at(startInd));
    auto end = item->coordToPixel(pointsArrayPtr->at(endInd));
    auto dist = (pixelPos - start.toPointF()).manhattanLength();
    if(dist <= selectionTolerance)
    {
      *details = PSSPStart;
      return dist;
    }
    dist = (pixelPos - end.toPointF()).manhattanLength();
    if(dist <= selectionTolerance)
    {
      *details = PSSPEnd;
      return dist;
    }
    //@TODO to proper calculation it is necessary to take into account bezier direcions.
    //for now we just detect point on straight line

    //if straight line:
    dist = GPVector2D(pixelPos).distanceToStraightLine(start, end - start);
    if(dist <= selectionTolerance)
    {
      *details = PSSPLine;
      return dist;
    }
    //
    //if curve line:
    // QPointF startVec(start->pixelPosition());
    // QPointF startDirVec(startDir->pixelPosition());
    // QPointF endDirVec(endDir->pixelPosition());
    // QPointF endVec(end->pixelPosition());

    // QPainterPath cubicPath(startVec);
    // cubicPath.cubicTo(startDirVec, endDirVec, endVec);

    // QPolygonF polygon = cubicPath.toSubpathPolygons().first();
    // GPVector2D p(pos);
    // double minDistSqr = std::numeric_limits<double>::max();
    // for (int i=1; i<polygon.size(); ++i)
    // {
    //   double distSqr = p.distanceSquaredToLine(polygon.at(i-1), polygon.at(i));
    //   if (distSqr < minDistSqr)
    //     minDistSqr = distSqr;
    // }
    // return qSqrt(minDistSqr);
    //
  }
  *details = PSSPNone;
  return -1;
}

bool PolySegment::isIndexesValid() const
{
  if(!pointsArrayPtr
      || startInd < 0
      || endInd < 0
      || startInd >= pointsArrayPtr->size()
      || endInd >= pointsArrayPtr->size())
  {
    return false;
  }
  return true;
}
