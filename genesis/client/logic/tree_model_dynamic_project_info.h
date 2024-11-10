#pragma once

#include "tree_model_dynamic.h"

////////////////////////////////////////////////////
//// Tree model dynamic / ProjectInfo
class TreeModelDynamicProjectInfo : public TreeModelDynamic
{
  Q_OBJECT

public:
  enum Column
  {
    ColumnId,
    ColumnName,
    ColumnParentId,
    ColumnParentName,
//    ColumnAuthorFullName,
    ColumnDateCreated,
    ColumnStatus,
    ColumnChromatogramCount,
    ColumnGroup,
    ColumnComment,
    ColumnCodeWord,

    ColumnLast
  };

public:
  TreeModelDynamicProjectInfo(QObject* parent);
  ~TreeModelDynamicProjectInfo();

public:
  //// Handle
  virtual void ApplyContextProject(const QString &dataId, const QVariant &data) override;

  //// Dynamic
  virtual void Reset() override;

  virtual bool canFetchMore(const QModelIndex& parent) const override;
  virtual void fetchMore(const QModelIndex& parent) override;

  virtual void FetchMore(QPointer<TreeModelItem> item) override;
};
