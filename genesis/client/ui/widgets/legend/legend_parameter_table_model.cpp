#include "legend_parameter_table_model.h"

#include <QPixmap>

LegendParameterTableModel::LegendParameterTableModel(QObject* parent)
  : QAbstractTableModel(parent)
{
}

int LegendParameterTableModel::rowCount(const QModelIndex& parent) const
{
  return static_cast<int>(mData.size());
}

int LegendParameterTableModel::columnCount(const QModelIndex& parent) const
{
  return 4;
}

QVariant LegendParameterTableModel::data(const QModelIndex& index, int role) const
{
  if (role == Qt::EditRole)
  {
    switch (index.column())
    {
    case 0:
      return mData[index.row()].title;
    case 1:
      return GPShapeItem::FromColor(mData[index.row()].color);
    case 2:
      return GPShape::ToShape(mData[index.row()].shape.path);
    default:
      break;
    }
  }

  if (role == Qt::ToolTipRole)
  {
    return mData[index.row()].tooltip;
  }

  if (role == Qt::DisplayRole)
  {
    switch (index.column())
    {
    case 0:
      return mData[index.row()].title;
    case 1:
      return GPShapeItem::ToString(GPShapeItem::FromColor(mData[index.row()].color));
    case 2:
      return GPShape::ToString(GPShape::ToShape(mData[index.row()].shape.path));
    case 3:
      return mData[index.row()].shape.pixelSize;
    default:
      break;
    }
  }

  if (role == Qt::DecorationRole)
  {
    if (mHideDecorationIndex.isValid() && index == mHideDecorationIndex)
    {
        return QVariant();
    }
    switch (index.column())
    {
    case 1:
    {
      QPixmap cp(16, 16);
      cp.fill(mData[index.row()].color);
      return cp;
    }
    case 2:
    {
      QPixmap cp(16, 16);
      cp.fill(Qt::transparent);
      QPainter p(&cp);
      p.setPen(Qt::NoPen);
      p.setBrush(Qt::black);
      p.save();
      p.scale(16, 16);
      p.drawPath(mData[index.row()].shape.path);
      p.restore();
      return cp;
    }
    default:
      break;
    }
  }
  return QVariant();
}

bool LegendParameterTableModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
  if (static_cast<size_t>(index.row()) > mData.size())
    return false;
  if (role == Qt::EditRole)
  {
    switch (index.column())
    {
    case 0:
      mData[index.row()].title = value.toString();
      return true;
    case 1:
      mData[index.row()].color = GPShapeItem::GetCycledColor(value.toInt());
      mHideDecorationIndex = QModelIndex();
      return true;
    case 2:
      mData[index.row()].shape.path = GPShape::GetPath(static_cast<GPShape::ShapeType>(value.toInt()));
      mHideDecorationIndex = QModelIndex();
      return true;
    case 3:
      mData[index.row()].shape.pixelSize = value.toInt();
      return true;
    default:
      break;
    }
  }
  return false;
}

Qt::ItemFlags LegendParameterTableModel::flags(const QModelIndex& index) const
{
  return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;
}

void LegendParameterTableModel::setLegendData(QList<AnalysisEntity::LegendGroup>& data)
{
  beginResetModel();
  mData = data;
  endResetModel();
}

QList<AnalysisEntity::LegendGroup> LegendParameterTableModel::getLegendData() const
{
  return mData;
}

void LegendParameterTableModel::hideIndexDecorations(const QModelIndex& index)
{
  mHideDecorationIndex = index;
}
