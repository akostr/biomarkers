#include "pls_table_item_model.h"

#include <QDateTime>
#include <QIcon>

namespace Model
{
  PlsTableItemModel::PlsTableItemModel(QObject* parent)
    :QAbstractTableModel(parent)
    , HeaderData
    {
      tr("Model name"),
      tr("Table"),
      tr("PC count"),
      tr("Dimensions"),
      tr("Comments"),
      tr("Create date"),
    }
  {
  }

  int PlsTableItemModel::rowCount(const QModelIndex& parent) const
  {
    return static_cast<int>(PlsData.size());
  }

  int PlsTableItemModel::columnCount(const QModelIndex& parent) const
  {
    return HeaderData.count() + 1; // count memebers in PlsData
  }

  QVariant PlsTableItemModel::data(const QModelIndex& index, int role) const
  {
    if (role == Qt::CheckStateRole || role == Qt::TextAlignmentRole)
      return QVariant();

    const auto row = index.row();
    if (row >= rowCount())
      return QVariant();

    switch (index.column())
    {
    case 0:
      return QString::fromStdString(PlsData[row].Name);
    case 1:
      return QString::fromStdString(PlsData[row].TableTitle);
    case 2:
      return static_cast<int>(PlsData[row].PCCount);
    case 3:
      return QString("%1 * %2")
        .arg(PlsData[row].SamplesCount)
        .arg(PlsData[row].PeaksCount);
    case 4:
      return QString::fromStdString(PlsData[row].Comments);
    case 5:
      return QDateTime::fromString(QString::fromStdString(PlsData[row].CreateDate), Qt::ISODate);
    case 6:
      return QIcon(IconPath);
    default:
      return QVariant();
    }
  }

  QVariant PlsTableItemModel::headerData(int section, Qt::Orientation orientation, int role) const
  {
    if (orientation != Qt::Horizontal || role != Qt::DisplayRole)
      return QVariant();

    if (section < HeaderData.size())
      return HeaderData[section];
    return "";
  }

  void PlsTableItemModel::SetModelData(const std::vector<PlsModel>& new_data)
  {
    PlsData = new_data;
  }

  void PlsTableItemModel::AppendItem(const PlsModel& item)
  {
    beginInsertRows(QModelIndex(), rowCount(), rowCount());
    PlsData.emplace_back(item);
    endInsertRows();
  }

  Qt::ItemFlags PlsTableItemModel::flags(const QModelIndex& index) const
  {
    if (!index.isValid())
      return Qt::ItemIsEnabled;

    if (index.data().canConvert<QIcon>())
      return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;

    return QAbstractItemModel::flags(index);
  }

  void PlsTableItemModel::setIconPath(const QString& icon_path)
  {
    IconPath = icon_path;
  }

  PlsModel PlsTableItemModel::at(int index) const
  {
    return PlsData.at(static_cast<size_t>(index));
  }
}
