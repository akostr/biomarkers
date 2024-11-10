#pragma once
#ifndef FILTERED_TABLE_VIEW_H
#define FILTERED_TABLE_VIEW_H

#include <QWidget>

class QHeaderView;
class QAbstractItemDelegate;
class QAbstractItemModel;
class CommonTableView;

namespace Ui
{
  class FilteredTableWidget;
}

class FilteredTableWidget : public QWidget
{
  Q_OBJECT
public:
  explicit FilteredTableWidget(QWidget* parent = nullptr);
  ~FilteredTableWidget() = default;

  CommonTableView* table() const;

private:
  Ui::FilteredTableWidget* ui = nullptr;

  void SetupUi();
  void ConnectSignals();
  void FilterEditingFinished();
};
#endif