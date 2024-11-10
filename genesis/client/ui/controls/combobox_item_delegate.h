#pragma once
#ifndef COMBOBOX_ITEM_DELEGATE_H
#define COMBOBOX_ITEM_DELEGATE_H

#include <QStyledItemDelegate>
#include <QWidget>
#include <QAbstractItemModel>

class ComboboxItemDelegate : public QStyledItemDelegate
{
  Q_OBJECT
public:
  explicit ComboboxItemDelegate(QObject* parent = nullptr);
  ~ComboboxItemDelegate() = default;

  QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const;
  void setEditorData(QWidget* editor, const QModelIndex& index) const;
  void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const;
  void setModel(QAbstractItemModel* model);
private:
  QAbstractItemModel* mModel = nullptr;
};
#endif