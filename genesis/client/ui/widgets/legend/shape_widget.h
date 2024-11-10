#pragma once
#ifndef SHAPE_WIDGET_H
#define SHAPE_WIDGET_H

#include <QWidget>
#include <QPen>
#include <QBrush>
#include <QPainterPath>

class ShapeWidget : public QWidget
{
  Q_OBJECT
public:
  explicit ShapeWidget(QWidget* parent = nullptr);
  virtual ~ShapeWidget() = default;

  void setShapeBrush(const QBrush& brush);
  void setShapePen(const QPen& pen);
  void setShape(int shape);
  void setShape(const QPainterPath& shape);

protected:
  void paintEvent(QPaintEvent* e) override;

private:
  QPainterPath mShape;
  QBrush mBrush;
  QPen mPen;
};

#endif