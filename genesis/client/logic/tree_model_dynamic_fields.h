#pragma once

#include "tree_model_dynamic.h"

////////////////////////////////////////////////////
//// Tree model dynamic / Fields
class TreeModelDynamicFields : public TreeModelDynamic
{
  Q_OBJECT

public:
  enum Column
  {
    ColumnId,
    ColumnField,

    ColumnLast
  };

public:
  TreeModelDynamicFields(QObject* parent);
  ~TreeModelDynamicFields();

public:
  //// Handle
  virtual void ApplyContextUser(const QString& dataId, const QVariant& data) override;

  //// Dynamic
  virtual void Reset() override;

  virtual bool canFetchMore(const QModelIndex& parent) const override;
  virtual void fetchMore(const QModelIndex& parent) override;

  virtual void FetchMore(QPointer<TreeModelItem> item) override;
};
