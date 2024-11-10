#ifndef TREEMODELDYNAMICPROJECTFRAGMENTS_H
#define TREEMODELDYNAMICPROJECTFRAGMENTS_H

#include "tree_model_dynamic.h"

class TreeModelDynamicProjectFragments : public TreeModelDynamic
{
  Q_OBJECT
public:
  enum Column
  {
    ColumnProjectId,
//    ColumnFileId,
    ColumnFileName,
//    ColumnMZ,
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
  explicit TreeModelDynamicProjectFragments(QObject *parent = nullptr);
  ~TreeModelDynamicProjectFragments() = default;
  QList<TreeModelItem*> getCheckedItems(TreeModelItem* parent = nullptr);
  int getCheckableCount();

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

  int nonGroupItemsCount(TreeModelItem* parent = nullptr);
  void setChildrenCheckState(Qt::CheckState state, TreeModelItem *parent);

  TreeModelItem* getChildItemByFileId(int fileId, TreeModelItem* item, bool recursive = false);

  static bool isItemIsGroup(const TreeModelItem *item);
  void setCheckedByMZValue(const QString& value, int state);

private:
  void sortIntoGroups(QPointer<TreeModelItem> item);
  void cleanupEmptyGroups(TreeModelItem *item);
  void parseIonsNamesToIonsLists(TreeModelItem *item);
  bool isItemIsGroup(const QModelIndex &index) const;
  void setIonNameCheckedFilter(QString name);
  void renameItems(TreeModelItem *item);

  void preprocessFetch(QVariantMap& variantMap);
  void postprocessFetch(TreeModelItem *item);

  // QAbstractItemModel interface
public:
  void resetIonNameCheckedFilter();
  QVariant data(const QModelIndex &index, int role) const override;
  static const QColor m_defaultGroupMemberColor;
  QList<ActionInfo> m_defaultGroupActions;
  bool setData(const QModelIndex &index, const QVariant &value, int role);
  Qt::ItemFlags flags(const QModelIndex &index) const;
  void setMarkupPickModeEnabled(bool isInMarkupPickMode);
  void enterPassportMode();
  void exitPassportMode();


  bool isInMarkupPickMode() const;

private:
  QList<ActionInfo> m_chromatogrammActions;
  QList<ActionInfo> m_fragmentActions;
  QList<ActionInfo> m_fragmentWithParentActions;
  bool m_isInMarkupPickMode;
  bool m_isInPassportPickMode;
  QString ionReferenceName;
  QMap<int, QPixmap> m_iconsToColumnsMap;//[intvalue, pixmap]
};


#endif // TREEMODELDYNAMICPROJECTFRAGMENTS_H
