#pragma once

#include "tree_model_dynamic.h"

////////////////////////////////////////////////////
//// Tree model dynamic / FileTypes
class TreeModelDynamicFileTypes : public TreeModelDynamic
{
  Q_OBJECT

public:
  enum Constant
  {
    ConstantFileTypeBase      = 1,
    ConstantFileTypeAuxiliary = 2,
  };

  enum Column
  {
    ColumnId,
    ColumnFileType,

    ColumnLast
  };

public:
  TreeModelDynamicFileTypes(QObject* parent);
  ~TreeModelDynamicFileTypes();

public:
  //// Handle
  virtual void ApplyContextUser(const QString& dataId, const QVariant& data) override;

  //// Dynamic
  virtual void Reset() override;

  virtual bool canFetchMore(const QModelIndex& parent) const override;
  virtual void fetchMore(const QModelIndex& parent) override;

  virtual void FetchMore(QPointer<TreeModelItem> item) override;
};
