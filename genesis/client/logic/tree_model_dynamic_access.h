#pragma once

#include "tree_model_dynamic.h"

////////////////////////////////////////////////////
//// Tree model dynamic / Access
class TreeModelDynamicAccess : public TreeModelDynamic
{
  Q_OBJECT

public:
  enum Constant
  {
    ConstantAccessView = 1,
    ConstantAccessEdit = 2,
    ConstantAccessFull = 3,
  };

  enum Column
  {
    ColumnId,
    ColumnAccess,

    ColumnLast
  };

public:
  TreeModelDynamicAccess(QObject* parent);
  ~TreeModelDynamicAccess();

public:
  //// Handle
  virtual void ApplyContextUser(const QString& dataId, const QVariant& data) override;

  //// Dynamic
  virtual void Reset() override;

  virtual bool canFetchMore(const QModelIndex& parent) const override;
  virtual void fetchMore(const QModelIndex& parent) override;

  virtual void FetchMore(QPointer<TreeModelItem> item) override;
};
