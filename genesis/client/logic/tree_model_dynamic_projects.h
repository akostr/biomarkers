#pragma once

#include "tree_model_dynamic.h"

////////////////////////////////////////////////////
//// Tree model dynamic / projects
class TreeModelDynamicProjects : public TreeModelDynamic
{
  Q_OBJECT

public:
  enum Column
  {
    ColumnLocked,
    ColumnCodeWord,
    ColumnId,
    ColumnName,
    ColumnChromatogramCount,
    ColumnStatus,
    ColumnComment,
    ColumnDateCreated,

    ColumnLast
  };

public:
  TreeModelDynamicProjects(QObject* parent);
  ~TreeModelDynamicProjects();

public:
  //// Handle
  virtual void ApplyContextUser(const QString &dataId, const QVariant &data) override;

  //// Dynamic
  virtual void Reset() override;

  virtual bool canFetchMore(const QModelIndex& parent) const override;
  virtual void fetchMore(const QModelIndex& parent) override;

  virtual void FetchMore(QPointer<TreeModelItem> item) override;
  QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

private:
  QIcon getCircleIcon(QString colorName) const;
  mutable QMap<QString, QIcon> mCirclesIconsCache;
  QIcon mLockIcon;

  // QAbstractItemModel interface
public:
  int columnCount(const QModelIndex &parent) const override;
};
