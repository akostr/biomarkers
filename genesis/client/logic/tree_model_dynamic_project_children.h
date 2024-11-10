#pragma once

#include "tree_model_dynamic.h"

////////////////////////////////////////////////////
//// Tree model dynamic / ProjectChildren
class TreeModelDynamicProjectChildren : public TreeModelDynamic
{
  Q_OBJECT

public:
  enum Column
  {
    ColumnId,
    ColumnName,
    /*ColumnParentName,
    ColumnChromatogramCount,*/
    //// ColumnStatus,
    /*ColumnAuthorFullName,
    ColumnRole,
    ColumnAccess,*/
    ColumnDateCreated,

    ColumnLast
  };

public:
  TreeModelDynamicProjectChildren(QObject* parent);
  ~TreeModelDynamicProjectChildren();

public:
  //// Handle
  virtual void ApplyContextUser(const QString &dataId, const QVariant &data) override;
  virtual void ApplyContextProject(const QString &dataId, const QVariant &data) override;

  //// Dynamic
  virtual void Reset() override;

  virtual bool canFetchMore(const QModelIndex& parent) const override;
  virtual void fetchMore(const QModelIndex& parent) override;

  virtual void FetchMore(QPointer<TreeModelItem> item) override;
};
