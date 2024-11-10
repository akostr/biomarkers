#pragma once
#ifndef COLOR_ITEM_MODEL_H
#define COLOR_ITEM_MODEL_H

#include <QAbstractListModel>
#include <QColor>
#include "ui/plots/gp_items/gpshape_item.h"

class ColorItemModel : public QAbstractListModel
{
  Q_OBJECT
public:
  explicit ColorItemModel(QObject* parent = nullptr);
  ~ColorItemModel() = default;

  int columnCount(const QModelIndex& parent = QModelIndex()) const override;
  int rowCount(const QModelIndex& parent) const override;
  QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
  int getIndex(const QColor &color);
  void showOnlyColor(bool onlyColor);

private:
  QList<GPShapeItem::DefaultColor> mColors;
  bool onlyColor = false;
};
#endif
