#ifndef IMPORT_XLS_PROXY_MODEL_H
#define IMPORT_XLS_PROXY_MODEL_H

#include <QSortFilterProxyModel>

class ImportXlsProxyModel : public QSortFilterProxyModel
{
  Q_OBJECT
public:
  explicit ImportXlsProxyModel(QObject *parent = nullptr);
  void setUnhiddableSourceRows(const QSet<int> rows);
  void setUnhiddableColumns(const QSet<int> columns);
  void setSourceColumnHidden(int c, bool isHidden = true);
  void setSourceRowHidden(int r, bool isHidden = true);

  QList<int> hiddenColumns() const;
  QList<int> hiddenRows() const;

private:
  QSet<int> mUnhiddableSourceRows;
  QSet<int> mUnhiddableSourceColumns;
  QSet<int> mHiddenSourceColumns;
  QSet<int> mHiddenSourceRows;

  // QSortFilterProxyModel interface
protected:
  bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;
  bool filterAcceptsColumn(int source_column, const QModelIndex &source_parent) const override;
};

#endif // IMPORT_XLS_PROXY_MODEL_H
