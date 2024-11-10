#pragma once
#ifndef DOUBLE_COLUMN_DELEGATE_H
#define DOUBLE_COLUMN_DELEGATE_H

#include <QStyledItemDelegate>
#include <QWidget>

namespace Control
{
  class DoubleColumnDelegate : public QStyledItemDelegate
  {
    Q_OBJECT
  public:
    explicit DoubleColumnDelegate(QObject* parent = nullptr);
    ~DoubleColumnDelegate() = default;

    // QAbstractItemDelegate interface
    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const;
    void setEditorData(QWidget* editor, const QModelIndex& index) const;
    void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const;

  };
}
#endif