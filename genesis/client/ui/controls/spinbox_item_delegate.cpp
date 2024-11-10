#include "spinbox_item_delegate.h"

#include <QSpinBox>

SpinboxItemDelegate::SpinboxItemDelegate(QObject* parent)
  :QStyledItemDelegate(parent)
{
}

QWidget* SpinboxItemDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
  auto editor = new QSpinBox(parent);
  return editor;
}

void SpinboxItemDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const
{
  if (const auto line = static_cast<QSpinBox*>(editor))
  {
    const auto value = index.model()->data(index, Qt::DisplayRole).toInt();
    line->setValue(value);
  }
}

void SpinboxItemDelegate::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
{
  if (const auto line = static_cast<QSpinBox*>(editor))
  {
    model->setData(index, line->value());
  }
}
