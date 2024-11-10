#pragma once
#ifndef TABLE_IDENTITY_MODEL_ACTIONS_DELEGATE_H
#define TABLE_IDENTITY_MODEL_ACTIONS_DELEGATE_H

#include <QStyledItemDelegate>
#include <QTableView>
#include <QPainter>
#include <QMenu>

namespace Widgets
{
  class TableIdentityModelActionsDelegate : public QStyledItemDelegate
  {
    Q_OBJECT

  public:
    TableIdentityModelActionsDelegate(QAbstractItemView* parent = nullptr);
    ~TableIdentityModelActionsDelegate() = default;

    virtual QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    virtual void updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    virtual void setEditorData(QWidget* editor, const QModelIndex& index) const override;

    void SetMenu(QMenu* menu);

  private:

    mutable QPoint MenuPosition;
    QMenu* Actions = nullptr;
  };
}
#endif