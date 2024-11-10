#pragma once

#include "tree_model_dynamic.h"

////////////////////////////////////////////////////
//// Tree model dynamic / ProjectUsers
class TreeModelDynamicProjectUsers : public TreeModelDynamic
{
  Q_OBJECT

public:
  enum Column
  {
    ColumnFullName,
    ColumnRole,
    ColumnAccess,

    ColumnLast
  };

public:
  TreeModelDynamicProjectUsers(QObject* parent);
  ~TreeModelDynamicProjectUsers();

public:
  //// Handle
  virtual void ApplyContextProject(const QString &dataId, const QVariant &data) override;

  //// Dynamic
  virtual void Reset() override;

  virtual bool canFetchMore(const QModelIndex& parent) const override;
  virtual void fetchMore(const QModelIndex& parent) override;

  virtual void FetchMore(QPointer<TreeModelItem> item) override;
};
