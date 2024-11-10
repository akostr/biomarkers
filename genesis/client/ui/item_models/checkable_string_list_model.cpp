#include "checkable_string_list_model.h"

using namespace Structures;

namespace Models
{
  CheckableStringListModel::CheckableStringListModel(QObject* parent)
    : QAbstractTableModel(parent)
  {
  }

  QVariant CheckableStringListModel::data(const QModelIndex& index, int role) const
  {
    if (!index.isValid())
      return QVariant();

    switch (role)
    {
    case Qt::CheckStateRole:
      return checkedItems.contains(index) ? Qt::Checked : Qt::Unchecked;
    case Qt::DisplayRole:
    {
      auto it = TestData.begin();
      std::advance(it, index.row());
      return it->second;
    }
    default:
      return QVariant();
    }
  }

  bool CheckableStringListModel::setData(const QModelIndex& index, const QVariant& value, int role)
  {
    if (role == Qt::CheckStateRole && value != Qt::PartiallyChecked)
    {
      if (value == Qt::Checked)
      {
        checkedItems.insert(index);
      }
      else
      {
        checkedItems.remove(index);
      }
      emit dataChanged(index, index, { role });
      return true;
    }
    return QAbstractTableModel::setData(index, value, role);
  }

  Qt::ItemFlags CheckableStringListModel::flags(const QModelIndex& index) const
  {
    return (QAbstractTableModel::flags(index)
      | Qt::ItemIsUserCheckable
      | Qt::ItemIsEnabled)
      & ~Qt::ItemIsSelectable;
  }

  QString CheckableStringListModel::SelectedNames() const
  {
    QStringList names;

    for (const auto& item : checkedItems)
    {
      names << item.data(Qt::DisplayRole).toString();
    }
    return names.join(";");
  }

  int CheckableStringListModel::rowCount(const QModelIndex& parent) const
  {
    return static_cast<int>(TestData.size());
  }

  int CheckableStringListModel::columnCount(const QModelIndex& parent) const
  {
    return 1;
  }

  std::vector<int> CheckableStringListModel::GetCheckedData() const
  {
    std::vector<int> list;
    list.reserve(checkedItems.size());
    std::transform(checkedItems.begin(), checkedItems.end(), std::back_inserter(list),
      [&](const QPersistentModelIndex& item)
      {
        auto it = TestData.begin();
        std::advance(it, item.row());
        return it->first;
      });
    return list;
  }

  void CheckableStringListModel::SetCheckedItems(const std::vector<int>& items)
  {
    for (const auto& id : items)
    {
      const auto iterator = TestData.find(id);
      auto findex = std::distance(TestData.begin(), iterator);
      setData(index(findex, 0), Qt::Checked, Qt::CheckStateRole);
    }
  }

  void CheckableStringListModel::SetTestData(const std::map<int, QString>& items)
  {
    beginResetModel();
    TestData = items;
    endResetModel();
  }

  int CheckableStringListModel::SelectedCount() const
  {
    return checkedItems.count();
  }
}