#ifndef TREEMODELDYNAMICLIBRARYCOMPOUNDSADDITIONAL_H
#define TREEMODELDYNAMICLIBRARYCOMPOUNDSADDITIONAL_H

#include "tree_model_dynamic_library_compounds.h"

class TreeModelDynamicLibraryCompoundsAdditional : public TreeModelDynamicLibraryCompounds
{
  Q_OBJECT
public:
  explicit TreeModelDynamicLibraryCompoundsAdditional(QObject *parent = nullptr, ElementType type = ElementType::COMPOUND);
  void FindRelated(const QString& shortTitle, const QString& fullTitle);
  void ClearRelated();
  void LoadRelated(const QVariantMap& data);
  // TreeModelDynamic interface
  void FetchMore(QPointer<TreeModelItem> item) override;
  void FetchMore(QPointer<TreeModelItem> item, const QString& shortTitle, const QString& fullTitle);
  // TreeModel interface
  void Clear() override;
  // QAbstractItemModel interface
  QVariant data(const QModelIndex &index, int role) const override;

  QModelIndex SubHeaderIndex() const;
  TreeModelItem* MainItem();

signals:
  void subHeaderIndexChanged(const QModelIndex& newSubHeaderIndex);

private:
  TreeModelItem* mMainItem = nullptr;
  QModelIndex mSubHeaderIndex;


};

#endif // TREEMODELDYNAMICLIBRARYCOMPOUNDSADDITIONAL_H
