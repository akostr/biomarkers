#pragma once

#include "../graphicsplot_extended.h"

namespace GPItemConst
{
constexpr int DmNone = 0;
constexpr int DeltaPixel = 5;
constexpr int HalfDeltaPixel = DeltaPixel / 2;
constexpr int RotateLineLength = 15;
constexpr int DefaultLayoutCount = 6;// дефолтные слои для отрисовки обычных элементов.
}

class CommonItemFuctional : public GPEXDraggingInterface
{
public:
  explicit CommonItemFuctional(GraphicsPlot* parentPlot);
  // virtual double opacity() { return m_opacity; }
  virtual double rotate() { return m_rotate; }

  // virtual void setOpacity(double opacity);
  virtual void setRotate(double rotate);

  void setDragMode(int mode) { m_defaultDragMode = mode; };
  void setError(bool error);

// signals:
  virtual void rotateChanged(double) = 0;
  virtual void changedPosition(QPointF , unsigned part) = 0;
  virtual void DragStarted(QPointF, int) = 0;
  virtual void DragMoved(QPointF) = 0;
  virtual void DragAccepted(QPointF) = 0;

  virtual void replot() = 0;

protected:
  virtual void DragStart(QMouseEvent*, unsigned part) override;
  virtual void DragMove(QMouseEvent*) override;
  virtual void DragAccept(QMouseEvent*) override;

public:
  template <typename T>
  T pixelToCoord(const T& point) const
  {
    double x = m_parentPlot->xAxis->pixelToCoord(point.x());
    double y = m_parentPlot->yAxis->pixelToCoord(point.y());
    return T(x, y);
  }
  template <typename T>
  T coordToPixel(const T& point) const
  {
    double x = m_parentPlot->xAxis->coordToPixel(point.x());
    double y = m_parentPlot->yAxis->coordToPixel(point.y());
    return T(x, y);
  }
  template<>
  QRectF pixelToCoord(const QRectF& pixelRect) const
  {
    auto rect = pixelRect;
    rect.setTopLeft(pixelToCoord(rect.topLeft()));
    rect.setBottomRight(pixelToCoord(rect.bottomRight()));
    return rect;
  }
  template<>
  QRectF coordToPixel(const QRectF& coordRect) const
  {
    auto rect = coordRect;
    rect.setTopLeft(coordToPixel(rect.topLeft()));
    rect.setBottomRight(coordToPixel(rect.bottomRight()));
    return rect;
  }

  virtual void setTransform(QPointF center, double moveRotationAngle);

protected:
  // double m_opacity;
  double m_rotate;
  bool m_error;
  int m_defaultDragMode;
  QPointF m_startDragPos;
  QPointF m_previousDragPos;
  QPointF m_lastDragPos;
  QPointF m_rotationCenter;
  GraphicsPlot* m_parentPlot;

  QTransform m_rotateTransform;
  QTransform m_invertedTransform;
};
