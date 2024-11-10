#include "color_item_model.h"

#include <QPixmap>
#include <QVariant>

ColorItemModel::ColorItemModel(QObject* parent)
  : QAbstractListModel(parent)
{
  for (int color = 0; color < GPShapeItem::DefaultColor::LastColor; color++)
    mColors.append((GPShapeItem::DefaultColor)color);
}

int ColorItemModel::columnCount(const QModelIndex&) const
{
  return 1;
}

int ColorItemModel::rowCount(const QModelIndex&) const
{
  return mColors.size();
}

QVariant ColorItemModel::data(const QModelIndex& index, int role) const
{
  const auto row = index.row();
  if (row > rowCount(index))
    return QVariant();

  if (role == Qt::DecorationRole)
  {
    QPixmap cp(16, 16);
    cp.fill(GPShapeItem::ToColor(mColors[row]));
    return cp;
  }
  if (role == Qt::DisplayRole && !onlyColor)
    return GPShapeItem::ToString(mColors[row]);
  if (role == Qt::UserRole)
    return QVariant::fromValue(GPShapeItem::ToColor(mColors[row]));
  return QVariant();
}

int ColorItemModel::getIndex(const QColor &color)
{
  return GPShapeItem::FromColor(color);
}

void ColorItemModel::showOnlyColor(bool show)
{
  onlyColor = show;
}


