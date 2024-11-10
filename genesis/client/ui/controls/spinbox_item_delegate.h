#pragma once
#ifndef SPINBOX_ITEM_DELEGATE_H
#define SPINBOX_ITEM_DELEGATE_H

#include <QStyledItemDelegate>
#include <QWidget>

class SpinboxItemDelegate : public QStyledItemDelegate
{
  Q_OBJECT
public:
  explicit SpinboxItemDelegate(QObject* parent = nullptr);
  ~SpinboxItemDelegate() = default;

  QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const;
  void setEditorData(QWidget* editor, const QModelIndex& index) const;
  void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const;
};
#endif