#include "table_view_widget.h"

#include "ui_table_view_widget.h"
#include <ui/itemviews/tableview_headerview.h>

namespace Widgets
{
  TableViewWidget::TableViewWidget(QWidget* parent)
    : QWidget(parent)
  {
    SetupUi();
  }

  TableViewWidget::~TableViewWidget()
  {
    if(WidgetUi)
      delete WidgetUi;
  }

  void TableViewWidget::SetModel(QAbstractTableModel* model)
  {
    SortFilter = new QSortFilterProxyModel(model);
    SortFilter->setSourceModel(model);
    WidgetUi->tableView->setModel(SortFilter);
    WidgetUi->tableView->SetDefaultAlignment(Qt::AlignLeft | Qt::AlignVCenter);
  }

  void TableViewWidget::SetItemDelegate(QAbstractItemDelegate* delegate)
  {
    delegate->setParent(WidgetUi->tableView);
    WidgetUi->tableView->setItemDelegate(delegate);
  }

  int TableViewWidget::SelectedRow() const
  {
    const auto selectionModel = WidgetUi->tableView->selectionModel();
    const auto currentIndex = selectionModel->currentIndex();
    return currentIndex.row();
  }

  QAbstractItemModel* TableViewWidget::Model() const
  {
    return SortFilter->sourceModel();
  }

  void TableViewWidget::SetResizeMode(QHeaderView::ResizeMode mode)
  {
    if (const auto header = WidgetUi->tableView->horizontalHeader())
    {
      header->setSectionResizeMode(mode);
    }
  }

  void TableViewWidget::SetResizeSections(QHeaderView::ResizeMode mode)
  {
    if (const auto header = WidgetUi->tableView->horizontalHeader())
    {
      header->resizeSections(mode);
    }
  }

  void TableViewWidget::SetStretchLastSection(bool mode)
  {
    if (const auto header = WidgetUi->tableView->horizontalHeader())
    {
      header->setStretchLastSection(mode);
    }
  }

  void TableViewWidget::setColumnHidden(int column, bool isHidden)
  {
    WidgetUi->tableView->setColumnHidden(column, isHidden);
  }

  void TableViewWidget::SetupUi()
  {
    WidgetUi = new Ui::TableViewWidget();
    WidgetUi->setupUi(this);
    WidgetUi->tableView->setSortingEnabled(true);
    WidgetUi->tableView->sortByColumn(-1, Qt::AscendingOrder);
    WidgetUi->tableView->verticalHeader()->hide();
    WidgetUi->tableView->setSelectionMode(QAbstractItemView::SingleSelection);
    WidgetUi->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    WidgetUi->tableView->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::SelectedClicked);
    WidgetUi->tableView->resizeColumnsToContents();
    WidgetUi->tableView->setStyleSheet("QTableView::item{ padding : 2px; }");
    connect(WidgetUi->tableView, &QTableView::doubleClicked, this, &TableViewWidget::DoubleClicked);
  }
}
