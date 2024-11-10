#include "multi_select_combobox.h"

#include "ui/item_models/checkable_string_list_model.h"

#include <QLineEdit>

using namespace Models;
using namespace Structures;

namespace Controls
{
  MultiSelectComboBox::MultiSelectComboBox(QWidget* parent)
    : QComboBox(parent)
  {
    setEditable(true);
    const auto le = lineEdit();
    le->setReadOnly(true);
    setModel(new CheckableStringListModel());
  }

  void MultiSelectComboBox::AddTestData(const std::map<int, QString>& testData)
  {
    if (const auto item_model = qobject_cast<CheckableStringListModel*>(model()))
    {
      item_model->SetTestData(testData);
    }
  }

  void MultiSelectComboBox::SetSelectedItems(const std::vector<int>& selectedItems)
  {
    if (const auto item_model = qobject_cast<CheckableStringListModel*>(model()))
    {
      item_model->SetCheckedItems(selectedItems);
    }
  }

  std::vector<int> MultiSelectComboBox::SelectedItems() const
  {
    if (const auto item_model = qobject_cast<CheckableStringListModel*>(model()))
    {
      return item_model->GetCheckedData();
    }
    return {};
  }

  void MultiSelectComboBox::setModel(QAbstractItemModel* model)
  {
    QComboBox::setModel(model);
    connect(model, &QAbstractItemModel::dataChanged, this, &MultiSelectComboBox::CheckStateChanged);
  }

  void MultiSelectComboBox::CheckStateChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight,
    const QList<int>& roles)
  {
    if (roles.contains(Qt::CheckStateRole))
    {
      if (const auto item_model = qobject_cast<CheckableStringListModel*>(model()))
      {
        setCurrentText(item_model->SelectedNames());
      }
    }
  }
}