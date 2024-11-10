#pragma once

#include "tree_model_dynamic.h"

////////////////////////////////////////////////////
//// Tree model dynamic / JobFunctions
class TreeModelDynamicJobFunctions : public TreeModelDynamic
{
  Q_OBJECT

public:
  enum Column
  {
    ColumnId,
    ColumnJobFunction,

    ColumnLast
  };

public:
  TreeModelDynamicJobFunctions(QObject* parent);
  ~TreeModelDynamicJobFunctions();

public:
  //// Handle
  virtual void ApplyContextUser(const QString &dataId, const QVariant &data) override;

  //// Dynamic
  virtual void Reset() override;

  virtual bool canFetchMore(const QModelIndex& parent) const override;
  virtual void fetchMore(const QModelIndex& parent) override;

  virtual void FetchMore(QPointer<TreeModelItem> item) override;
};
