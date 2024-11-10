#include "import_xls_proxy_model.h"

ImportXlsProxyModel::ImportXlsProxyModel(QObject *parent)
  : QSortFilterProxyModel{parent}
{}

void ImportXlsProxyModel::setUnhiddableSourceRows(const QSet<int> rows)
{
  mUnhiddableSourceRows = rows;
  if(mHiddenSourceRows.intersects(mUnhiddableSourceRows))
  {
    mHiddenSourceRows.subtract(mUnhiddableSourceRows);
    invalidate();
  }
}

void ImportXlsProxyModel::setUnhiddableColumns(const QSet<int> columns)
{
  mUnhiddableSourceColumns = columns;
  if(mHiddenSourceColumns.intersects(mUnhiddableSourceColumns))
  {
    mHiddenSourceColumns.subtract(mUnhiddableSourceColumns);
    invalidate();
  }
}

void ImportXlsProxyModel::setSourceColumnHidden(int c, bool isHidden)
{
  if(mUnhiddableSourceColumns.contains(c))
    return;
  if(isHidden && mHiddenSourceColumns.contains(c))
    return;
  if(!isHidden && !mHiddenSourceColumns.contains(c))
    return;
  if(isHidden)
    mHiddenSourceColumns.insert(c);
  else
    mHiddenSourceColumns.remove(c);
  invalidate();
}

void ImportXlsProxyModel::setSourceRowHidden(int r, bool isHidden)
{
  if(mUnhiddableSourceRows.contains(r))
    return;
  if(isHidden && mHiddenSourceRows.contains(r))
    return;
  if(!isHidden && !mHiddenSourceRows.contains(r))
    return;
  if(isHidden)
    mHiddenSourceRows.insert(r);
  else
    mHiddenSourceRows.remove(r);
  invalidate();
}

QList<int> ImportXlsProxyModel::hiddenColumns() const
{
  QList<int> res{mHiddenSourceColumns.begin(), mHiddenSourceColumns.end()};
  std::sort(res.begin(), res.end());
  return res;
}

QList<int> ImportXlsProxyModel::hiddenRows() const
{
  QList<int> res{mHiddenSourceRows.begin(), mHiddenSourceRows.end()};
  std::sort(res.begin(), res.end());
  return res;
}


bool ImportXlsProxyModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
  if(QSortFilterProxyModel::filterAcceptsRow(source_row, source_parent))
  {
    return !mHiddenSourceRows.contains(source_row);
  }
  return false;
}
bool ImportXlsProxyModel::filterAcceptsColumn(int source_column, const QModelIndex &source_parent) const
{
  if(QSortFilterProxyModel::filterAcceptsColumn(source_column, source_parent))
  {
    return !mHiddenSourceColumns.contains(source_column);
  }
  return false;
}
