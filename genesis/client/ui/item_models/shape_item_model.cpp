#include "shape_item_model.h"
#include <QPixmap>
#include <QPainter>

ShapeItemModel::ShapeItemModel(QObject* parent)
  :QAbstractListModel(parent)
{
  for (int i = 0; i < GPShape::ShapeType::LastShape; i++)
    mShapes.append((GPShape::ShapeType)i);
}

int ShapeItemModel::columnCount(const QModelIndex& parent) const
{
  return 1;
}

int ShapeItemModel::rowCount(const QModelIndex& parent) const
{
  return mShapes.size();
}

QVariant ShapeItemModel::data(const QModelIndex& index, int role) const
{
  const auto row = index.row();
  if (row > rowCount(index))
    return QVariant();

  if (role == Qt::DecorationRole)
  {
    QPixmap cp(16,16);
    cp.fill(Qt::transparent);
    auto path = GPShape::GetPath(mShapes[row]);
    QPainter p(&cp);
    p.setPen(Qt::NoPen);
    p.setBrush(Qt::black);
    p.save();
    p.scale(16,16);
    p.drawPath(path);
    p.restore();
    return cp;
  }
  if (role == Qt::DisplayRole)
    return GPShape::ToString(mShapes[row]);
  if(role == Qt::UserRole)
    return QVariant::fromValue(GPShape::GetPath(mShapes[row]));
  return QVariant();
}

int ShapeItemModel::getIndex(const QPainterPath &path)
{
  return GPShape::ToShape(path);
}
