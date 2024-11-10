#pragma once
#ifndef LEGEND_WIDGET_H
#define LEGEND_WIDGET_H

#include <QWidget>
#include <QPen>
#include <QPainterPath>

class GPShape;

class LegendWidget : public QWidget
{
  Q_OBJECT
public:
  LegendWidget(QWidget* parent = nullptr);
  void setShapeBrush(const QBrush& brush);
  void setShapePen(const QPen& pen);
  void setShape(int shape);
  void setShape(const QPainterPath& shape);
  void setText(const QString& text);
  void setFont(const QFont& font);
  void setSpacing(int spacing);
  int spacing();

protected:
  void paintEvent(QPaintEvent* e) override;
  QSize sizeHint() const override;
  void updateShapeSize();

private:
  QPainterPath mShape;
  QSizeF mSize;
  QFont mFont;
  QString mText;
  QBrush mBrush;
  QPen mPen;
  QFontMetricsF mFontMetrics;
  int mSpacing;
};

#endif
