#pragma once

#include <QStyledItemDelegate>
#include <QTableView>
#include <QMenu>
#include <QPointer>

////////////////////////////////////////////////////
//// Actions menu for tree model
class TableViewModelActionsColumnDelegate : public virtual QStyledItemDelegate
{
  Q_OBJECT

public:
  TableViewModelActionsColumnDelegate(QAbstractItemView* parent);
  ~TableViewModelActionsColumnDelegate();

  //// Impl
  virtual QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
  virtual void destroyEditor(QWidget* editor, const QModelIndex& index) const override;
  virtual void updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
  virtual void setEditorData(QWidget* editor, const QModelIndex& index) const override;


private:
  void Setup();

private:
  QPointer<QAbstractItemView>    View;
  mutable QPointer<QMenu> Menu;
  mutable QPoint          MenuPosition;
};
