#include "tree_model_presentation.h"

#include <QDate>
#include "tree_model_item.h"

////////////////////////////////////////////////////
//// Tree model presentation
TreeModelPresentation::TreeModelPresentation(TreeModel* parent)
  : QSortFilterProxyModel(parent)
  , Source(parent)
{
  resetCustomItemFilter();
  setSourceModel(parent);
  setSortRole(Qt::EditRole);
}

TreeModelPresentation::~TreeModelPresentation()
{
}

//// Column filtering
void TreeModelPresentation::SetAcceptedColumns(const QSet<int>& columns)
{
  AcceptedColumns = columns;
  invalidate();
}

void TreeModelPresentation::AddAcceptedColumn(int column)
{
  beginResetModel();//we need this because app crashes on
  //library table while switching action mode without it
  AcceptedColumns.insert(column);
  ////not works properly by some unknown reason
  //  QMetaObject::invokeMethod(this, &TreeModelPresentation::invalidate, Qt::QueuedConnection);
  invalidate();
  endResetModel();
}
////damn multithreading
void TreeModelPresentation::RemoveAcceptedColumn(int column)
{
  beginResetModel();//we need this because app crashes on
  //library table while switching action mode without it
  AcceptedColumns.remove(column);
  ////not works properly by some unknown reason
  //  QMetaObject::invokeMethod(this, &TreeModelPresentation::invalidate, Qt::QueuedConnection);
  invalidate();
  endResetModel();
}

void TreeModelPresentation::SetRejectedColumns(const QSet<int> &columns)
{
  RejectedColumns = columns;
  invalidate();
}

void TreeModelPresentation::AddRejectedColumn(int column)
{
  beginResetModel();//we need this because app crashes on
  //library table while switching action mode without it
  RejectedColumns.insert(column);
  ////not works properly by some unknown reason
  //  QMetaObject::invokeMethod(this, &TreeModelPresentation::invalidate, Qt::QueuedConnection);
  invalidate();
  endResetModel();
}

void TreeModelPresentation::RemoveRejectedColumn(int column)
{
  beginResetModel();//we need this because app crashes on
  //library table while switching action mode without it
  RejectedColumns.remove(column);
  ////not works properly by some unknown reason
  //  QMetaObject::invokeMethod(this, &TreeModelPresentation::invalidate, Qt::QueuedConnection);
  invalidate();
  endResetModel();
}

void TreeModelPresentation::AddRejectedRow(int row, const QPersistentModelIndex& parent)
{
  RejectedRowsMap[parent].insert(row);
  invalidate();
}

void TreeModelPresentation::SetRejectedRows(const QSet<int>& rows, const QPersistentModelIndex& parent)
{
  RejectedRowsMap[parent] = rows;
  invalidate();
}

void TreeModelPresentation::ClearRejectedRows()
{
  RejectedRowsMap.clear();
  invalidate();
}

//// Column filtering
bool TreeModelPresentation::filterAcceptsColumn(int sourceColumn, const QModelIndex& sourceParent) const
{
  if (!sourceParent.isValid())
  {
    return !RejectedColumns.contains(sourceColumn) && (AcceptedColumns.empty() || AcceptedColumns.contains(sourceColumn));
  }
  return QSortFilterProxyModel::filterAcceptsColumn(sourceColumn, sourceParent);
}

//// Row filtering impl
bool TreeModelPresentation::filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const
{
  if (!sourceParent.isValid())
  {
    if (Source)
    {
      if (RejectedRowsMap.contains(sourceParent) &&
        RejectedRowsMap[sourceParent].contains(sourceRow))
        return false;
      auto sourceInd = Source->index(sourceRow, 0, sourceParent);
      auto item = Source->GetItem(sourceInd);
      for (auto fiter = ItemDataFilters.constKeyValueBegin(); fiter != ItemDataFilters.constKeyValueEnd(); fiter++)
      {
        if (!fiter->second.isValid())
          continue;
        if (item->GetData(fiter->first) != fiter->second)
          return false;
      }
      if(!CustomItemFilter(item))
        return false;
      for (int c = 0; c < Source->columnCount(); c++)
      {
        QVariant filterData = ColumnFilters.value(c);
        if (filterData.isValid())
        {
          ColumnInfo ci = Source->GetColumnInfo(c);
          switch (ci.Filter)
          {
          case ColumnInfo::FilterTypeTextValuesList:
          {
            QVariantList filterDataList = filterData.toList();
            if (!filterDataList.empty())
            {
              QSet<QString> filterDataListStrings;
              for (auto& fd : filterDataList)
                filterDataListStrings << fd.toString();

              QVariant data = Source->data(Source->index(sourceRow, c), Qt::DisplayRole);
              if (!filterDataListStrings.contains(data.toString()))
              {
                return false;
              }
            }
          }
          break;

          case ColumnInfo::FilterTypeTextSubstring:
          {
            QString substring = filterData.toString().toLower();
            if (!substring.isEmpty())
            {
              QString data = Source->data(Source->index(sourceRow, c), Qt::DisplayRole).toString().toLower();
              if (!data.contains(substring))
              {
                return false;
              }
            }
          }
          break;

          case ColumnInfo::FilterTypeDateRange:
          {
            QVariantList filterDataList = filterData.toList();
            if (filterDataList.size() == 2)
            {
              QDate dateStart = filterDataList.first().toDate();
              QDate dateEnd = filterDataList.last().toDate();
              if (dateStart.isValid() && dateEnd.isValid())
              {
                QVariant data = Source->data(Source->index(sourceRow, c), Qt::EditRole);
                if (data
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
                  .typeId()
#else
                  .type()
#endif
                  == QMetaType::QDate)
                {
                  QDate d = data.toDate();
                  if (d < dateStart)
                    return false;
                  if (d > dateEnd)
                    return false;
                }
                else if (data
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
                  .typeId()
#else
                  .type()
#endif
                  == QMetaType::QDateTime)
                {
                  QDate d = data.toDateTime().date();
                  if (d < dateStart)
                    return false;
                  if (d > dateEnd)
                    return false;
                }
              }
            }
          }
          break;

          case ColumnInfo::FilterTypeIntegerRange:
          {
            QVariantList filterDataList = filterData.toList();
            if (filterDataList.size() == 2)
            {
              int mn = filterDataList.first().toInt();
              int mx = filterDataList.last().toInt();
              QVariant data = Source->data(Source->index(sourceRow, c), Qt::EditRole);
              int v = data.toInt();
              if (v < mn)
                return false;
              if (v > mx)
                return false;
            }
          }
          break;

          default:
            break;
          }
        }
      }
    }
    return true;
  }
  return QSortFilterProxyModel::filterAcceptsRow(sourceRow, sourceParent);
}

TreeModelItem* TreeModelPresentation::GetItem(const QModelIndex& presentationIndex)
{
  return presentationIndex.isValid()
    ? static_cast<TreeModelItem*>(mapToSource(presentationIndex).internalPointer())
    : nullptr;
}

//// Get actions for item
QList<ActionInfo> TreeModelPresentation::GetItemActions(const QModelIndex& presentationIndex)
{
  return presentationIndex.isValid()
    ? Source->GetItemActions(mapToSource(presentationIndex))
    : QList<ActionInfo>();
}

void TreeModelPresentation::TriggerAction(const QString& actionId, TreeModelItem* item)
{
  //// Both source & proxy shall emit
  Source->TriggerAction(actionId, item);

  emit ActionTriggered(actionId, item);
}

void TreeModelPresentation::setCustomItemFilter(ItemCustomFilter filter)
{
  CustomItemFilter = filter;
  invalidate();
}

void TreeModelPresentation::resetCustomItemFilter()
{
  CustomItemFilter = [](TreeModelItem*)->bool{return true;};
  invalidate();
}

QVariant TreeModelPresentation::GetColumnFilter(int column) const
{
  return ColumnFilters.value(column);
}

void TreeModelPresentation::SetColumnFilter(int column, const QVariant& filter)
{
  ColumnFilters[column] = filter;
  invalidateFilter();
  emit FilterChanged();
}

QVariant TreeModelPresentation::GetRowFilter() const
{
  return RowFilter;
}

void TreeModelPresentation::SetRowFilter(const QVariant& filter)
{
  RowFilter = filter;
  invalidateFilter();
  emit FilterChanged();
}

void TreeModelPresentation::SetItemDataFilter(QString dataId, QVariant data)
{
  ItemDataFilters[dataId] = data;
  invalidate();
}

void TreeModelPresentation::RemoveItemDataFilter(QString dataId)
{
  ItemDataFilters.remove(dataId);
  invalidate();
}

void TreeModelPresentation::ClearItemsDataFilters()
{
  ItemDataFilters.clear();
  invalidate();
}

int TreeModelPresentation::mapColumnFromSource(int column) const
{
  auto& a = AcceptedColumns;
  if (a.isEmpty())
    return column;
  if (!a.contains(column))
    return -1;
  int mapped = 0;
  for (int i = 0; i < Source->columnCount() && i < column; i++)
  {
    if (a.contains(i))
      mapped++;
  }
  return mapped;
}

////////////////////////////////////////////////////
//// Single column proxy model
TreeModelPresentationSingleColumn::TreeModelPresentationSingleColumn(TreeModel* model, int column)
  : TreeModelPresentation(model)
{
  SetAcceptedColumns({ column });
}

TreeModelPresentationSingleColumn::~TreeModelPresentationSingleColumn()
{
}
