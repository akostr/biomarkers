#include "common_item_fuctional.h"
#include <cmath>

namespace ic
{
constexpr int DmNone = 0;
constexpr int DeltaPixel = 5;
constexpr int HalfDeltaPixel = DeltaPixel / 2;
constexpr int RotateLineLength = 15;
constexpr int DefaultLayoutCount = 6;// дефолтные слои для отрисовки обычных элементов.
}

CommonItemFuctional::CommonItemFuctional(GraphicsPlot* parentPlot)
    : GPEXDraggingInterface()
    // , m_opacity(1.0)
    , m_rotate(0)
    , m_error(false)
    , m_defaultDragMode(GPItemConst::DmNone)
    , m_parentPlot(parentPlot)
{
  mDraggingPart = GPItemConst::DmNone;
}

// void CommonItemFuctional::setOpacity(double opacity)
// {
//   if (std::fabs(m_opacity - opacity) > std::numeric_limits<double>::epsilon())
//   {
//     m_opacity = opacity;
//     emit replot();
//   }
// }

void CommonItemFuctional::setRotate(double rotate)
{
  if (std::fabs(m_rotate - rotate) > std::numeric_limits<double>::epsilon())
  {
    m_rotate = rotate;
    setTransform(m_rotationCenter, m_rotate);
    emit replot();
  }
}

void CommonItemFuctional::setError(bool error)
{
  m_error = error;
  emit replot();
}

void CommonItemFuctional::DragStart(QMouseEvent* event, unsigned part)
{
  m_previousDragPos = event->pos();
  mDraggingPart = (m_defaultDragMode == GPItemConst::DmNone) ? part :m_defaultDragMode;
  m_startDragPos = m_previousDragPos;
  emit DragStarted(m_previousDragPos, mDraggingPart);
}

void CommonItemFuctional::DragMove(QMouseEvent* event)
{
  const QPointF newPos = event->pos();
  const QPointF delta = newPos - m_previousDragPos;
  m_previousDragPos = newPos;
  emit changedPosition(delta, mDraggingPart);
  emit DragMoved(delta);
}

void CommonItemFuctional::DragAccept(QMouseEvent* event)
{
  const QPointF newPos = event->pos();
  const QPointF delta = newPos - m_previousDragPos;
  m_previousDragPos = newPos;
  emit changedPosition(delta, mDraggingPart);
  emit DragAccepted(delta);
  qDebug() << "Drag finish";
}

void CommonItemFuctional::setTransform(QPointF center, double rotationAngle)
{
  m_rotate = rotationAngle;
  m_rotationCenter = center;
  QTransform rotate;
  rotate.translate(m_rotationCenter.x(), m_rotationCenter.y());
  rotate.rotate(m_rotate);
  rotate.translate(-m_rotationCenter.x(), -m_rotationCenter.y());
  m_rotateTransform = rotate;
  m_invertedTransform = m_rotateTransform.inverted();
  rotateChanged(m_rotate);
  emit replot();
}
