#pragma once

#include "tree_model_dynamic.h"

////////////////////////////////////////////////////
//// Tree model dynamic / FileInfos
class TreeModelDynamicFileInfos : public TreeModelDynamic
{
  Q_OBJECT

public:
  enum Column
  {
    ColumnId,
    ColumnTitle,
    ColumnDateTime,
    ColumnWellTitle,
    ColumnFieldTitle,
    ColumnLayerTitle,

    ColumnLast
  };

public:
  TreeModelDynamicFileInfos(QObject* parent, int fileId);
  ~TreeModelDynamicFileInfos();

public:
  //// Handle
  virtual void ApplyContextUser(const QString& dataId, const QVariant& data) override;

  //// Dynamic
  virtual void Reset() override;

  virtual bool canFetchMore(const QModelIndex& parent) const override;
  virtual void fetchMore(const QModelIndex& parent) override;

  virtual void FetchMore(QPointer<TreeModelItem> item) override;

public:
  int FileId;
};
