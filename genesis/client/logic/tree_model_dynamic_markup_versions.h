#pragma once

#include "tree_model_dynamic.h"

class TreeModelDynamicMarkupVersions : public TreeModelDynamic
{
  Q_OBJECT

public:
  enum Column
  {
    ColumnTitle,
    ColumnCreated,
    ColumnUserName,
    ColumnParentVersionTitle,

    ColumnLast
  };

public:
  TreeModelDynamicMarkupVersions(QObject* parent);
  ~TreeModelDynamicMarkupVersions();

public:
  //// Handle
  virtual void ApplyContextMarkup(const QString &dataId, const QVariant &data) override;

  //// Dynamic
  virtual void Reset() override;

  virtual bool canFetchMore(const QModelIndex& parent) const override;
  virtual void fetchMore(const QModelIndex& parent) override;

  virtual void FetchMore(QPointer<TreeModelItem> item) override;
};
