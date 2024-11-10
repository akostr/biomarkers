//#pragma once

//#include "tree_model_dynamic.h"

//////////////////////////////////////////////////////
////// Tree model dynamic / OtherUsers
//class TreeModelDynamicOtherUsers : public TreeModelDynamic
//{
//  Q_OBJECT

//public:
//  enum Column
//  {
//    ColumnId,
//    ColumnFullName,
//    ColumnEmail,
//    ColumnPhone,
//    ColumnFunction,

//    ColumnLast
//  };

//public:
//  TreeModelDynamicOtherUsers(QObject* parent);
//  ~TreeModelDynamicOtherUsers();

//public:
//  //// Handle
//  virtual void ApplyContextUser(GenesisContextPtr context) override;

//  //// Dynamic
//  virtual void Reset() override;

//  virtual bool canFetchMore(const QModelIndex& parent) const override;
//  virtual void fetchMore(const QModelIndex& parent) override;

//  virtual void FetchMore(QPointer<TreeModelItem> item) override;
//};
