#include "markup_version_reference_model.h"

#include <QDateTime>
#include <QIcon>

using namespace Structures;

namespace Models
{
  MarkupVersionReferenceModel::MarkupVersionReferenceModel(QObject* parent)
    : QAbstractTableModel(parent)
    , HorizontalHeaders
    {
        tr("File name"),
        tr("Markup name"),
        tr("Comment"),
        //tr("Author"),
        tr("Date"),
    }
  {
  }

  int MarkupVersionReferenceModel::rowCount(const QModelIndex& parent) const
  {
    return static_cast<int>(Data.size());
  }

  int MarkupVersionReferenceModel::columnCount(const QModelIndex& parent) const
  {
    return static_cast<int>(HorizontalHeaders.size() + 1);
  }

  QVariant MarkupVersionReferenceModel::data(const QModelIndex& index, int role) const
  {
    if (role == Qt::CheckStateRole /*|| role == Qt::TextAlignmentRole*/)
      return QVariant();

    const auto row = index.row();
    if (row >= rowCount())
      return QVariant();
    if (role == Qt::DisplayRole || role == Qt::EditRole)
    {
      switch (index.column())
      {
      case 0:
        return QString::fromStdString(Data[row].Title);
      case 1:
        return QString::fromStdString(Data[row].MarkupTitle);
      case 2:
        return QString::fromStdString(Data[row].Comment);
      //case 3:
      //  return QString::fromStdString(Data[row].Author);
      case 3:
        return QDateTime::fromString(QString::fromStdString(Data[row].Changed), Qt::ISODate);
        //        case 5:
        //          return QIcon(IconPath);
      default:
        return QVariant();
      }
    }
    else
    {
      if (role == Qt::DecorationRole && index.column() == 4)
      {
        return QIcon(IconPath);
      }
      return QVariant();
    }
  }

  QVariant MarkupVersionReferenceModel::headerData(int section, Qt::Orientation orientation, int role) const
  {
    if (orientation != Qt::Horizontal || role != Qt::DisplayRole)
      return QVariant();

    if (section < static_cast<int>(HorizontalHeaders.size()))
      return HorizontalHeaders[section];
    return "";
  }

  void MarkupVersionReferenceModel::SetReferenceList(const ReferenceList& list)
  {
    beginResetModel();
    Data = list;
    endResetModel();
  }

  Qt::ItemFlags MarkupVersionReferenceModel::flags(const QModelIndex& index) const
  {
    if (!index.isValid())
      return Qt::ItemIsEnabled;

    if (index.data().canConvert<QIcon>())
      return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;

    return QAbstractItemModel::flags(index);
  }

  void MarkupVersionReferenceModel::setIconPath(const QString& icon_path)
  {
    IconPath = icon_path;
  }

  Reference MarkupVersionReferenceModel::at(int index) const
  {
    return Data[index];
  }
}
