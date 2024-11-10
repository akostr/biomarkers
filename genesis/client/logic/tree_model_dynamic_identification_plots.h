#ifndef TREEMODELDYNAMICIDENTIFICATIONPLOTS_H
#define TREEMODELDYNAMICIDENTIFICATIONPLOTS_H

#include "tree_model_dynamic.h"
#include "tree_model_item.h"

class TreeModelDynamicIdentificationPlots : public TreeModelDynamic
{
  Q_OBJECT

public:
  enum Column
  {
    ColumnTitle,
    ColumnTableTitle,
    ColumnComment,
    ColumnCreatedDate,
    ColumnLast
  };
  explicit TreeModelDynamicIdentificationPlots(QObject *parent = nullptr);
  void ApplyContextProject(const QString& dataId, const QVariant& data) override;
  void ApplyContextUser(const QString &dataId, const QVariant &data) override;

  virtual bool canFetchMore(const QModelIndex& parent) const override;
  virtual void fetchMore(const QModelIndex& parent) override;
  virtual void FetchMore(QPointer<TreeModelItem> item) override;

  bool isItemIsGroup(QPointer<TreeModelItem> item) const;

  virtual QList<ActionInfo> GetItemActions(const QModelIndex& index) override;

private:
  void sortIntoGroups(QPointer<TreeModelItem> item);

private:
  bool m_checkableOnlyWithFragments;
};

#endif // TREEMODELDYNAMICIDENTIFICATIONPLOTS_H
