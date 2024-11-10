#pragma once

#include "tree_model_dynamic.h"

////////////////////////////////////////////////////
//// Tree model dynamic / FieldWells
class TreeModelDynamicFieldWells : public TreeModelDynamic
{
  Q_OBJECT

public:
  enum Column
  {
    ColumnId,
    ColumnFieldWell,

    ColumnLast
  };

public:
  TreeModelDynamicFieldWells(QObject* parent, int fieldId);
  ~TreeModelDynamicFieldWells();

  int GetFieldId() const { return FieldId; }

public:
  //// Handle
  virtual void ApplyContextUser(const QString& dataId, const QVariant& data) override;

  //// Dynamic
  virtual void Reset() override;

  virtual bool canFetchMore(const QModelIndex& parent) const override;
  virtual void fetchMore(const QModelIndex& parent) override;

  virtual void FetchMore(QPointer<TreeModelItem> item) override;

private:
  int FieldId;
};
