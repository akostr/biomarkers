#include "shape_widget.h"

#include "ui/plots/gp_items/gpshape.h"

#include <QPainter>

ShapeWidget::ShapeWidget(QWidget* parent)
  :QWidget(parent)
  , mShape(GPShape::GetPath(GPShape::Square))
  , mPen(Qt::NoPen)
{
}

void ShapeWidget::setShapeBrush(const QBrush& brush)
{
  mBrush = brush;
}

void ShapeWidget::setShapePen(const QPen& pen)
{
  mPen = pen;
}

void ShapeWidget::setShape(int shape)
{
  mShape = GPShape::GetPath((GPShape::ShapeType)shape);
  // update();
}

void ShapeWidget::setShape(const QPainterPath& shape)
{
  mShape = shape;
  // update();
}

void ShapeWidget::paintEvent(QPaintEvent* e)
{
  QPainter p(this);
  p.setRenderHint(QPainter::Antialiasing);
  p.save();
  p.setPen(mPen);
  p.setBrush(mBrush);
  p.scale(width(), height());
  p.drawPath(mShape);
  p.restore();
}
