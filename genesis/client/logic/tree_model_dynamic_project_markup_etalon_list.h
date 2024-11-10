#pragma once
#ifndef TREE_MODEL_DYNAMIC_PROJECT_MARKUP_ETALON_LIST_H
#define TREE_MODEL_DYNAMIC_PROJECT_MARKUP_ETALON_LIST_H

#include "tree_model_dynamic.h"

class TreeModelDynamicProjectMarkupEtalonList : public TreeModelDynamic
{
  Q_OBJECT

public:
  enum Column
  {
    ColumnTitle,
    ColumnMarkup,
    ColumnComment,
    ColumnCreated,
    ColumnLast
  };

public:
  explicit TreeModelDynamicProjectMarkupEtalonList(QObject* parent = nullptr);
  ~TreeModelDynamicProjectMarkupEtalonList() override = default;

protected:
  void ApplyContextProject(const QString &dataId, const QVariant &data) override;

  virtual bool canFetchMore(const QModelIndex& parent) const override;
  virtual void fetchMore(const QModelIndex& parent) override;

  virtual void FetchMore(QPointer<TreeModelItem> item) override;
};
#endif