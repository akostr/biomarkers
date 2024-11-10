#include "combobox_item_delegate.h"

#include <QComboBox>

ComboboxItemDelegate::ComboboxItemDelegate(QObject* parent)
  :QStyledItemDelegate(parent)
{
}

QWidget* ComboboxItemDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
  const auto editor = new QComboBox(parent);
  editor->setModel(mModel);
  return editor;
}

void ComboboxItemDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const
{
  if (const auto line = static_cast<QComboBox*>(editor))
  {
    const auto value = index.model()->data(index, Qt::EditRole).toInt();
    line->setCurrentIndex(value);
  }
}

void ComboboxItemDelegate::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
{
  if (const auto line = static_cast<QComboBox*>(editor))
  {
    model->setData(index, line->currentIndex(), Qt::EditRole);
  }
}

void ComboboxItemDelegate::setModel(QAbstractItemModel* model)
{
  mModel = model;
}
