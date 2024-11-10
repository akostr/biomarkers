#pragma once

#include "tree_model_dynamic.h"

class TreeModelDynamicProjectMarkupsList : public TreeModelDynamic
{
  Q_OBJECT

public:
  enum Column
  {
    ColumnTitle,
    ColumnStatus,
    ColumnReference,
    ColumnCount,
    ColumnComment,
    ColumnChanged,

    ColumnLast
  };

public:
  TreeModelDynamicProjectMarkupsList(QObject* parent);
  ~TreeModelDynamicProjectMarkupsList();

public:
  //// Handle
  virtual void ApplyContextProject(const QString &dataId, const QVariant &data) override;

  //// Dynamic
  virtual void Reset() override;

  virtual bool canFetchMore(const QModelIndex& parent) const override;
  virtual void fetchMore(const QModelIndex& parent) override;

  virtual void FetchMore(QPointer<TreeModelItem> item) override;
  QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

private:
  QIcon getCircleIcon(QString colorName) const;
  mutable QMap<QString, QIcon> mCirclesIconsCache;
};
