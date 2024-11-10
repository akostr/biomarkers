#include "double_column_delegate.h"

#include <QDoubleValidator>
#include <QLineEdit>

namespace Control
{
  DoubleColumnDelegate::DoubleColumnDelegate(QObject* parent)
    : QStyledItemDelegate(parent)
  {
  }

  QWidget* DoubleColumnDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const
  {
    QLineEdit* editor = new QLineEdit(parent);
    editor->setStyleSheet("QLineEdit{ padding : 0px; }");
    editor->setValidator(new QDoubleValidator);
    return editor;
  }

  void DoubleColumnDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const
  {
    QString value = index.model()->data(index, Qt::EditRole).toString();
    QLineEdit* line = static_cast<QLineEdit*>(editor);
    line->setText(value);
  }

  void DoubleColumnDelegate::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
  {
    QLineEdit* line = static_cast<QLineEdit*>(editor);
    QString value = line->text();
    model->setData(index, value);
  }
}