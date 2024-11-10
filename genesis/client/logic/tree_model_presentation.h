#pragma once

#include "tree_model.h"

#include <QSortFilterProxyModel>
#include <QSet>


using ItemCustomFilter = std::function<bool(TreeModelItem* item)>;

////////////////////////////////////////////////////
//// Tree model presentation
class TreeModelPresentation : public QSortFilterProxyModel
{
  Q_OBJECT

public:
  TreeModelPresentation(TreeModel* parent);
  ~TreeModelPresentation();

  //// Column filtering
  void SetAcceptedColumns(const QSet<int>& columns);
  void AddAcceptedColumn(int column);
  void RemoveAcceptedColumn(int column);
  void SetRejectedColumns(const QSet<int>& columns);
  void AddRejectedColumn(int column);
  void RemoveRejectedColumn(int column);

  //// Row filtering
  void AddRejectedRow(int row, const QPersistentModelIndex& parent);
  void SetRejectedRows(const QSet<int>& rows, const QPersistentModelIndex& parent);
  void ClearRejectedRows();

  //// Column filtering impl
  virtual bool filterAcceptsColumn(int sourceColumn, const QModelIndex& sourceParent) const override;

  //// Row filtering impl
  virtual bool filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const override;

  //// Get item from presentation index
  TreeModelItem* GetItem(const QModelIndex& presentationIndex);

  //// Get actions for item
  virtual QList<ActionInfo> GetItemActions(const QModelIndex& presentationIndex);

  //// Notifications
  void TriggerAction(const QString& actionId, TreeModelItem* item);

  void setCustomItemFilter(ItemCustomFilter filter);
  void resetCustomItemFilter();

  //// Filtering
  QVariant GetColumnFilter(int column) const;
  void SetColumnFilter(int column, const QVariant& filter);

  QVariant GetRowFilter() const;
  void SetRowFilter(const QVariant& filter);

  void SetItemDataFilter(QString dataId, QVariant data);
  void RemoveItemDataFilter(QString dataId);
  void ClearItemsDataFilters();
  int mapColumnFromSource(int column) const;

signals:
  void ActionTriggered(const QString& actionId, TreeModelItem* item);
  void FilterChanged();

protected:
  //// Source
  QPointer<TreeModel> Source;

  //// Column filtering
  QSet<int> AcceptedColumns;
  QSet<int> RejectedColumns;

  QMap<QPersistentModelIndex, QSet<int>> RejectedRowsMap;
  QMap<QString, QVariant> ItemDataFilters;

  ///// Row filtering
  QMap<int, QVariant> ColumnFilters;
  QVariant RowFilter;
  ItemCustomFilter CustomItemFilter;
};

////////////////////////////////////////////////////
//// Single column proxy model
class TreeModelPresentationSingleColumn : public TreeModelPresentation
{
  Q_OBJECT

public:
  TreeModelPresentationSingleColumn(TreeModel* model, int column);
  ~TreeModelPresentationSingleColumn();
};
