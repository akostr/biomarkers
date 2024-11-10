#pragma once
#ifndef SHAPE_ITEM_MODEL_H
#define SHAPE_ITEM_MODEL_H

#include <QAbstractListModel>
#include <QPainterPath>
#include "ui/plots/gp_items/gpshape.h"

class ShapeItemModel : public QAbstractListModel
{
  Q_OBJECT
public:
  explicit ShapeItemModel(QObject* parent = nullptr);
  ~ShapeItemModel() = default;

  int columnCount(const QModelIndex& parent = QModelIndex()) const override;
  int rowCount(const QModelIndex& parent) const override;
  QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
  int getIndex(const QPainterPath& path);

private:
  QList<GPShape::ShapeType> mShapes;
};
#endif
