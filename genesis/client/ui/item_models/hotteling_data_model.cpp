#include "hotteling_data_model.h"

#include <QColor>

using namespace Structures;

namespace Models
{
  HottelingDataModel::HottelingDataModel(QObject* parent)
    : QAbstractTableModel(parent)
  {
  }

  int HottelingDataModel::rowCount(const QModelIndex& parent) const
  {
    return static_cast<int>(Headers.size());
  }

  int HottelingDataModel::columnCount(const QModelIndex& parent) const
  {
    return static_cast<int>(Headers.size());
  }

  QVariant HottelingDataModel::data(const QModelIndex& index, int role) const
  {
    if(role == Qt::DisplayRole)
      return QString::fromStdString(TableData[index.row()][index.column()]);

    if (role == Qt::BackgroundRole)
      return QColor(QString::fromStdString(Colors[index.row()][index.column()]));

    return QVariant();
  }

  QVariant HottelingDataModel::headerData(int section, Qt::Orientation orientation, int role) const
  {
    if ( role != Qt::DisplayRole)
      return QVariant();

    if(section >= Headers.size())
      return QVariant();

    return QString::fromStdString(Headers[section]);
  }

  void HottelingDataModel::SetHeaderData(const StringList& headers)
  {
    Headers = headers;
  }

  void HottelingDataModel::SetColors(const StringList2D& colors)
  {
    Colors = colors;
  }

  void HottelingDataModel::SetTableData(const StringList2D& tableData)
  {
    beginResetModel();
    TableData = tableData;
    endResetModel();
  }
}