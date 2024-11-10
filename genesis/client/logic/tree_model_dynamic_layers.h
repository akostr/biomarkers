#pragma once

#include "tree_model_dynamic.h"

////////////////////////////////////////////////////
//// Tree model dynamic / Layers
class TreeModelDynamicLayers : public TreeModelDynamic
{
  Q_OBJECT

public:
  enum Column
  {
    ColumnId,
    ColumnLayer,

    ColumnLast
  };

public:
  TreeModelDynamicLayers(QObject* parent);
  ~TreeModelDynamicLayers();

public:
  //// Handle
  virtual void ApplyContextUser(const QString &dataId, const QVariant &data) override;

  //// Dynamic
  virtual void Reset() override;

  virtual bool canFetchMore(const QModelIndex& parent) const override;
  virtual void fetchMore(const QModelIndex& parent) override;

  virtual void FetchMore(QPointer<TreeModelItem> item) override;
};
