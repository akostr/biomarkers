#include "pls_predict_table_item_model.h"

#include <QDateTime>
#include <QIcon>

namespace Model
{
  PlsPredictTableItemModel::PlsPredictTableItemModel(QObject* parent)
    :QAbstractTableModel(parent)
    , HeaderData
    {
      tr("Model name"),
      tr("Table"),
      tr("Applied model PLS"),
      tr("Comments"),
      tr("Create date"),
    }
  {
  }

  int PlsPredictTableItemModel::rowCount(const QModelIndex& parent) const
  {
    return static_cast<int>(PlsData.size());
  }

  int PlsPredictTableItemModel::columnCount(const QModelIndex& parent) const
  {
    return HeaderData.count(); // count memebers in PlsData
  }

  QVariant PlsPredictTableItemModel::data(const QModelIndex& index, int role) const
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
      return QString::fromStdString(PlsData[row].ParentTitle);
    case 3:
      return QString::fromStdString(PlsData[row].Comments);
    case 4:
      return QDateTime::fromString(QString::fromStdString(PlsData[row].CreateDate), Qt::ISODate);
    case 5:
      return QIcon(IconPath);
    default:
      return QVariant();
    }
  }

  QVariant PlsPredictTableItemModel::headerData(int section, Qt::Orientation orientation, int role) const
  {
    if (orientation != Qt::Horizontal || role != Qt::DisplayRole)
      return QVariant();

    if (section < HeaderData.size())
      return HeaderData[section];
    return "";
  }

  void PlsPredictTableItemModel::SetModelData(const std::vector<PlsModel>& new_data)
  {
    PlsData = new_data;
  }

  void PlsPredictTableItemModel::AppendItem(const PlsModel& item)
  {
    beginInsertRows(QModelIndex(), rowCount(), rowCount());
    PlsData.emplace_back(item);
    endInsertRows();
  }

  Qt::ItemFlags PlsPredictTableItemModel::flags(const QModelIndex& index) const
  {
    if (!index.isValid())
      return Qt::ItemIsEnabled;

    if (index.data().canConvert<QIcon>())
      return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;

    return QAbstractItemModel::flags(index);
  }

  void PlsPredictTableItemModel::setIconPath(const QString& icon_path)
  {
    IconPath = icon_path;
  }

  PlsModel PlsPredictTableItemModel::at(int index) const
  {
    return PlsData.at(static_cast<size_t>(index));
  }
}
