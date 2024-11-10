#ifndef TREEMODELDYNAMICANALYZES_H
#define TREEMODELDYNAMICANALYZES_H

#include "tree_model_dynamic.h"

class TreeModelDynamicAnalyzes : public TreeModelDynamic
{
  Q_OBJECT

public:
  enum Column
  {
    ColumnTitle,
    ColumnCreated,
    ColumnUserName,

    ColumnLast
  };

public:
  TreeModelDynamicAnalyzes(QObject *parent);
  ~TreeModelDynamicAnalyzes();

public:
  //// Handle
  virtual void ApplyContextMarkup(const QString& dataId, const QVariant& data) override;
  virtual void ApplyContextMarkupVersion(const QString &dataId, const QVariant &data) override;


  //// Dynamic
  virtual void Reset() override;

  virtual bool canFetchMore(const QModelIndex& parent) const override;
  virtual void fetchMore(const QModelIndex& parent) override;

  virtual void FetchMore(QPointer<TreeModelItem> item) override;
};

#endif // TREEMODELDYNAMICANALYZES_H
