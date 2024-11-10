#pragma once
#ifndef LEGEND_SHAPE_ITEM_H
#define LEGEND_SHAPE_ITEM_H

#include <QWidget>
#include <QPainterPath>

namespace Ui
{
  class LegendShapeItem;
}

class LegendShapeItem : public QWidget
{
  Q_OBJECT
public:
  explicit LegendShapeItem(QWidget* parent = nullptr);
  virtual ~LegendShapeItem();

  void setShape(const QPainterPath& shape);
  void setShapePen(const QPen& pen);
  void setShapeBrush(const QBrush& brush);
  void setText(const QString& text);

signals:
  void editClicked();

private:
  void setupUi();
  void connectSignals();

private:
  Ui::LegendShapeItem* mUi = nullptr;

  // QObject interface
public:
  bool event(QEvent *event) override;
};


#endif
