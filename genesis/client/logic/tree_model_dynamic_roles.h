//#pragma once

//#include "tree_model_dynamic.h"

//////////////////////////////////////////////////////
////// Tree model dynamic / Roles
//class TreeModelDynamicRoles : public TreeModelDynamic
//{
//  Q_OBJECT

//public:
//  enum Constant
//  {
//    ConstantRoleAuthor      = 1,
//    ConstantRoleParticipant = 2,
//  };

//  enum Column
//  {
//    ColumnId,
//    ColumnRole,

//    ColumnLast
//  };

//public:
//  TreeModelDynamicRoles(QObject* parent);
//  ~TreeModelDynamicRoles();

//public:
//  //// Handle
//  virtual void ApplyContextUser(GenesisContextPtr context) override;

//  //// Dynamic
//  virtual void Reset() override;

//  virtual bool canFetchMore(const QModelIndex& parent) const override;
//  virtual void fetchMore(const QModelIndex& parent) override;

//  virtual void FetchMore(QPointer<TreeModelItem> item) override;
//};
