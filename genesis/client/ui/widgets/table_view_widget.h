#pragma once
#ifndef TABLE_VIEW_WIDGET_H
#define TABLE_VIEW_WIDGET_H

#include <QWidget>
#include <QAbstractTableModel>
#include <QPointer>
#include <QSortFilterProxyModel>
#include <QAbstractItemDelegate>
#include <QHeaderView>

namespace Ui
{
  class TableViewWidget;
}

namespace Widgets
{
  class TableViewWidget : public QWidget
  {
    Q_OBJECT

  public:
    explicit TableViewWidget(QWidget* parent = nullptr);
    ~TableViewWidget();

    void SetModel(QAbstractTableModel* model);
    void SetItemDelegate(QAbstractItemDelegate* delegate);
    int SelectedRow() const;
    QAbstractItemModel* Model() const;

    void SetResizeMode(QHeaderView::ResizeMode mode);
    void SetResizeSections(QHeaderView::ResizeMode mode);
    void SetStretchLastSection(bool mode);
    void setColumnHidden(int column, bool isHidden);

  signals:
    void DoubleClicked(const QModelIndex& index);

  private:
    Ui::TableViewWidget* WidgetUi = nullptr;
    QPointer<QSortFilterProxyModel> SortFilter = nullptr;

    void SetupUi();
  };
}
#endif
