#ifndef TREEMODELDYNAMICPROJECTCHROMATOGRAMMS_H
#define TREEMODELDYNAMICPROJECTCHROMATOGRAMMS_H

#include "tree_model_dynamic.h"

////////////////////////////////////////////////////
//// Tree model dynamic / Project chromatogramms
class TreeModelDynamicProjectChromatogramms : public TreeModelDynamic
{
  Q_OBJECT

public:
  enum Column
  {
    ColumnProjectId,
//    ColumnFileId,
    ColumnFileName,
    ColumnSampleType,
    ColumnType,
    ColumnField,
    ColumnWellCluster,
    ColumnWell,
    ColumnLayer,
    ColumnDepth,
    ColumnDate,

    ColumnLast
  };

public:
  explicit TreeModelDynamicProjectChromatogramms(QObject *parent);
  ~TreeModelDynamicProjectChromatogramms();
  QList<TreeModelItem*> GetCheckedItems(TreeModelItem* parent = nullptr);

public:
  //// Handle
  virtual void ApplyContextUser(const QString &dataId, const QVariant &data) override;
  virtual void ApplyContextProject(const QString &dataId, const QVariant &data) override;

  //// Dynamic
  virtual void Reset() override;

  virtual bool canFetchMore(const QModelIndex& parent) const override;
  virtual void fetchMore(const QModelIndex& parent) override;

  virtual void FetchMore(QPointer<TreeModelItem> item) override;
  QList<ActionInfo> GetItemActions(const QModelIndex& index) override;

  int NonGroupItemsCount(TreeModelItem* parent = nullptr);
  void SetChildrenCheckState(Qt::CheckState state, TreeModelItem *parent);
  int GetCheckableCount();


  static bool isItemIsGroup(const TreeModelItem *item);

private:
  void sortIntoGroups(QPointer<TreeModelItem> item);
//  void setupItemFlags(QPointer<TreeModelItem> rootItem, bool forFragmentsExtraction);
  bool isItemIsGroup(const QModelIndex &index) const;


  // QAbstractItemModel interface
public:
  QVariant data(const QModelIndex &index, int role) const override;
  static const QColor DefaultGroupMemberColor;
  QList<ActionInfo> DefaultGroupActions;
  void setCheckableOnlyWithFragments(bool enabled);
  Qt::ItemFlags flags(const QModelIndex &index) const;

private:
  bool m_checkableOnlyWithFragments;
  QMap<int, QPixmap> m_iconsToColumnsMap;//[column, [intvalue, pixmap]
};

#endif // TREEMODELDYNAMICPROJECTCHROMATOGRAMMS_H
