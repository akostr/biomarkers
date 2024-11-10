#include "gp_curve_tracer_item.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////// GPCurveTracerItem
////////////////////////////////////////////////////////////////////////////////////////////////////

/*! \class GPCurveTracerItem
  \brief Item that sticks to GPCurve data points

  \image html GPCurveTracerItem.png "Tracer example. Blue dotted circles are anchors, solid blue discs are positions."

  The tracer can be connected with a GPCurve via \ref setCurve. Then it will automatically adopt
  the coordinate axes of the curve and update its \a position to be on the curve's data. This means
  the key stays controllable via \ref setCurveKey, but the value will follow the curve data. If a
  GPCurve is connected, note that setting the coordinates of the tracer item directly via \a
  position will have no effect because they will be overriden in the next redraw (this is when the
  coordinate update happens).

  If the specified key in \ref setCurveKey is outside the key bounds of the curve, the tracer will
  stay at the corresponding end of the curve.

  With \ref setInterpolating you may specify whether the tracer may only stay exactly on data
  points or whether it interpolates data points linearly, if given a key that lies between two data
  points of the curve.

  The tracer has different visual styles, see \ref setStyle. It is also possible to make the tracer
  have no own visual appearance (set the style to \ref tsNone), and just connect other item
  positions to the tracer \a position (used as an anchor) via \ref
  GPItemPosition::setParentAnchor.

  \note The tracer position is only automatically updated upon redraws. So when the data of the
  curve changes and immediately afterwards (without a redraw) the position coordinates of the
  tracer are retrieved, they will not reflect the updated data of the curve. In this case \ref
  updatePosition must be called manually, prior to reading the tracer coordinates.
*/

/*!
  Creates a tracer item and sets default values.

  The created item is automatically registered with \a parentPlot. This GraphicsPlot instance takes
  ownership of the item, so do not delete it manually but use GraphicsPlot::removeItem() instead.
*/
GPCurveTracerItem::GPCurveTracerItem(GraphicsPlot *parentPlot) :
  GPAbstractItem(parentPlot),
  position(createPosition(QLatin1String("position"))),
  mSize(6),
  mStyle(tsCrosshair),
  mCurve(nullptr),
  mCurveKey(0),
  mInterpolating(false)
{
  //// Set valid or equally null input uom descriptor from axis before setCoords()
  if (parentPlot->xAxis)
  {
    position->mInputUomForKeys    = mParentPlot->xAxis->getUomDescriptor();
  }
  if (parentPlot->yAxis)
  {
    position->mInputUomForValues  = mParentPlot->yAxis->getUomDescriptor();
  }

  //// Safely setCoords() w/ input uom descriptor set
  position->setCoords(0, 0);

  //// Restore input uom descriptor
  position->mInputUomForKeys      = GPUomDescriptor();
  position->mInputUomForValues    = GPUomDescriptor();

  setBrush(Qt::NoBrush);
  setSelectedBrush(Qt::NoBrush);
  setPen(QPen(Qt::black));
  setSelectedPen(QPen(Qt::blue, 2));
}

GPCurveTracerItem::~GPCurveTracerItem()
{
}

/*!
  Sets the pen that will be used to draw the line of the tracer

  \see setSelectedPen, setBrush
*/
void GPCurveTracerItem::setPen(const QPen &pen)
{
  mPen = pen;
}

/*!
  Sets the pen that will be used to draw the line of the tracer when selected

  \see setPen, setSelected
*/
void GPCurveTracerItem::setSelectedPen(const QPen &pen)
{
  mSelectedPen = pen;
}

/*!
  Sets the brush that will be used to draw any fills of the tracer

  \see setSelectedBrush, setPen
*/
void GPCurveTracerItem::setBrush(const QBrush &brush)
{
  mBrush = brush;
}

/*!
  Sets the brush that will be used to draw any fills of the tracer, when selected.

  \see setBrush, setSelected
*/
void GPCurveTracerItem::setSelectedBrush(const QBrush &brush)
{
  mSelectedBrush = brush;
}

/*!
  Sets the size of the tracer in pixels, if the style supports setting a size (e.g. \ref tsSquare
  does, \ref tsCrosshair does not).
*/
void GPCurveTracerItem::setSize(double size)
{
  mSize = size;
}

/*!
  Sets the style/visual appearance of the tracer.

  If you only want to use the tracer \a position as an anchor for other items, set \a style to
  \ref tsNone.
*/
void GPCurveTracerItem::setStyle(GPCurveTracerItem::TracerStyle style)
{
  mStyle = style;
}

/*!
  Sets the GPCurve this tracer sticks to. The tracer \a position will be set to type
  GPItemPosition::ptPlotCoords and the axes will be set to the axes of \a curve.

  To free the tracer from any curve, set \a curve to 0. The tracer \a position can then be placed
  freely like any other item position. This is the state the tracer will assume when its curve gets
  deleted while still attached to it.

  \see setCurveKey
*/
void GPCurveTracerItem::setCurve(GPCurve *curve)
{
  if (curve)
  {
    if (curve->parentPlot() == mParentPlot)
    {
      position->setType(GPItemPosition::ptPlotCoords);
      position->setAxes(curve->keyAxis(), curve->valueAxis());
      mCurve = curve;
      updatePosition();
    } else
      qDebug() << Q_FUNC_INFO << "curve isn't in same GraphicsPlot instance as this item";
  } else
  {
    mCurve = nullptr;
  }
}

/*!
  Sets the key of the curve's data point the tracer will be positioned at. This is the only free
  coordinate of a tracer when attached to a curve.

  Depending on \ref setInterpolating, the tracer will be either positioned on the data point
  closest to \a key, or will stay exactly at \a key and interpolate the value linearly.

  \see setCurve, setInterpolating
*/
void GPCurveTracerItem::setCurveKey(double key)
{
  mCurveKey = key;
}

/*!
  Sets whether the value of the curve's data points shall be interpolated, when positioning the
  tracer.

  If \a enabled is set to false and a key is given with \ref setCurveKey, the tracer is placed on
  the data point of the curve which is closest to the key, but which is not necessarily exactly
  there. If \a enabled is true, the tracer will be positioned exactly at the specified key, and
  the appropriate value will be interpolated from the curve's data points linearly.

  \see setCurve, setCurveKey
*/
void GPCurveTracerItem::setInterpolating(bool enabled)
{
  mInterpolating = enabled;
}

/* inherits documentation from base class */
double GPCurveTracerItem::selectTest(const QPointF &pos, bool onlySelectable, QVariant *details) const
{
  Q_UNUSED(details)
  if (onlySelectable && !mSelectable)
    return -1;

  QPointF center(position->pixelPosition());
  double w = mSize/2.0;
  QRect clip = clipRect();
  switch (mStyle)
  {
    case tsNone: return -1;
    case tsPlus:
    {
      if (clipRect().intersects(QRectF(center-QPointF(w, w), center+QPointF(w, w)).toRect()))
        return qSqrt(qMin(GPVector2D(pos).distanceSquaredToLine(center+QPointF(-w, 0), center+QPointF(w, 0)),
                          GPVector2D(pos).distanceSquaredToLine(center+QPointF(0, -w), center+QPointF(0, w))));
      break;
    }
    case tsCrosshair:
    {
      return qSqrt(qMin(GPVector2D(pos).distanceSquaredToLine(GPVector2D(clip.left(), center.y()), GPVector2D(clip.right(), center.y())),
                        GPVector2D(pos).distanceSquaredToLine(GPVector2D(center.x(), clip.top()), GPVector2D(center.x(), clip.bottom()))));
    }
    case tsCircle:
    {
      if (clip.intersects(QRectF(center-QPointF(w, w), center+QPointF(w, w)).toRect()))
      {
        // distance to border:
        double centerDist = GPVector2D(center-pos).length();
        double circleLine = w;
        double result = qAbs(centerDist-circleLine);
        // filled ellipse, allow click inside to count as hit:
        if (result > mParentPlot->selectionTolerance()*0.99 && mBrush.style() != Qt::NoBrush && mBrush.color().alpha() != 0)
        {
          if (centerDist <= circleLine)
            result = mParentPlot->selectionTolerance()*0.99;
        }
        return result;
      }
      break;
    }
    case tsSquare:
    {
      if (clip.intersects(QRectF(center-QPointF(w, w), center+QPointF(w, w)).toRect()))
      {
        QRectF rect = QRectF(center-QPointF(w, w), center+QPointF(w, w));
        bool filledRect = mBrush.style() != Qt::NoBrush && mBrush.color().alpha() != 0;
        return rectDistance(rect, pos, filledRect);
      }
      break;
    }
  }
  return -1;
}

/* inherits documentation from base class */
void GPCurveTracerItem::draw(GPPainter *painter)
{
  updatePosition();
  if (mStyle == tsNone)
    return;

  painter->setPen(mainPen());
  painter->setBrush(mainBrush());
  QPointF center(position->pixelPosition());
  double w = mSize/2.0;
  QRect clip = clipRect();
  switch (mStyle)
  {
    case tsNone: return;
    case tsPlus:
    {
      if (clip.intersects(QRectF(center-QPointF(w, w), center+QPointF(w, w)).toRect()))
      {
        painter->drawLine(QLineF(center+QPointF(-w, 0), center+QPointF(w, 0)));
        painter->drawLine(QLineF(center+QPointF(0, -w), center+QPointF(0, w)));
      }
      break;
    }
    case tsCrosshair:
    {
      if (center.y() > clip.top() && center.y() < clip.bottom())
        painter->drawLine(QLineF(clip.left(), center.y(), clip.right(), center.y()));
      if (center.x() > clip.left() && center.x() < clip.right())
        painter->drawLine(QLineF(center.x(), clip.top(), center.x(), clip.bottom()));
      break;
    }
    case tsCircle:
    {
      if (clip.intersects(QRectF(center-QPointF(w, w), center+QPointF(w, w)).toRect()))
        painter->drawEllipse(center, w, w);
      break;
    }
    case tsSquare:
    {
      if (clip.intersects(QRectF(center-QPointF(w, w), center+QPointF(w, w)).toRect()))
        painter->drawRect(QRectF(center-QPointF(w, w), center+QPointF(w, w)));
      break;
    }
  }
}

/*!
  If the tracer is connected with a curve (\ref setCurve), this function updates the tracer's \a
  position to reside on the curve data, depending on the configured key (\ref setCurveKey).

  It is called automatically on every redraw and normally doesn't need to be called manually. One
  exception is when you want to read the tracer coordinates via \a position and are not sure that
  the curve's data (or the tracer key with \ref setCurveKey) hasn't changed since the last redraw.
  In that situation, call this function before accessing \a position, to make sure you don't get
  out-of-date coordinates.

  If there is no curve set on this tracer, this function does nothing.
*/
void GPCurveTracerItem::updatePosition()
{
  if (mCurve)
  {
    if (mParentPlot->hasPlottable(mCurve))
    {
      if (mCurve->data()->size() > 1)
      {
        GPCurveDataContainer::const_iterator first = mCurve->data()->constBegin();
        GPCurveDataContainer::const_iterator last = mCurve->data()->constEnd()-1;
        if (mCurveKey <= first->key)
          position->setCoords(first->key, first->value);
        else if (mCurveKey >= last->key)
          position->setCoords(last->key, last->value);
        else
        {
          if (mInterpolating)
          {
            GPCurveDataContainer::const_iterator it = mCurve->data()->constFindBegin(mCurveKey);
            if (it != mCurve->data()->constEnd()) // mCurveKey is not exactly on last iterator, but somewhere between iterators
            {
              GPCurveDataContainer::const_iterator prevIt = it;
              ++it; // won't advance to constEnd because we handled that case (mCurveKey >= last->key) before
              // interpolate between iterators around mCurveKey:
              double slope = 0;
              if (!qFuzzyCompare((double)it->key, (double)prevIt->key))
                slope = (it->value-prevIt->value)/(it->key-prevIt->key);
              position->setCoords(mCurveKey, (mCurveKey-prevIt->key)*slope+prevIt->value);

            } else // mCurveKey is exactly on last iterator (should actually be caught when comparing first/last keys, but this is a failsafe for fp uncertainty)
              position->setCoords(it->key, it->value);
          }
          else
          {
            auto begin = mCurve->data()->begin();
            auto dist = fabs(begin->key - mCurveKey);
            auto pos = QPointF(begin->key, begin->value);
            begin++;
            for(auto it = begin; it != mCurve->data()->end(); it++)
            {
              auto tmpDist = fabs(it->key - mCurveKey);
              if(tmpDist < dist)
              {
                dist = tmpDist;
                pos = QPointF(it->key, it->value);
              }
            }
            position->setCoords(pos);
          }
        }
      } else if (mCurve->data()->size() == 1)
      {
        GPCurveDataContainer::const_iterator it = mCurve->data()->constBegin();
        position->setCoords(it->key, it->value);
      } else
        qDebug() << Q_FUNC_INFO << "curve has no data";
    } else
      qDebug() << Q_FUNC_INFO << "curve not contained in GraphicsPlot instance (anymore)";
  }
}

/*! \internal

  Returns the pen that should be used for drawing lines. Returns mPen when the item is not selected
  and mSelectedPen when it is.
*/
QPen GPCurveTracerItem::mainPen() const
{
  return mSelected ? mSelectedPen : mPen;
}

/*! \internal

  Returns the brush that should be used for drawing fills of the item. Returns mBrush when the item
  is not selected and mSelectedBrush when it is.
*/
QBrush GPCurveTracerItem::mainBrush() const
{
  return mSelected ? mSelectedBrush : mBrush;
}
