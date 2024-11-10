#pragma once
#ifndef TREE_MODEL_DYNAMIC_LIBRARY_CONNECTIONS_H
#define TREE_MODEL_DYNAMIC_LIBRARY_CONNECTIONS_H

#include "tree_model_dynamic.h"
#include "logic/enums.h"
#include <unordered_map>

using LibraryConnection::ElementType;

class TreeModelDynamicLibraryCompounds : public TreeModelDynamic
{
  Q_OBJECT
public:
  enum Column
  {
    CHECKED,
    FAVOURITES,
    GROUP,
    SHORT_TITLE,
    FULL_TITLE,
    CLASSIFIER_TITLE,
    COMPOUND_CLASS_TITLE,
    TIC_SIM,
    MZ,
    FORMULA,
    CHEMICAL_FORMULA,
    SPECIFIC,
    DIMENSION,
    KOVATS_INDEX,
    KOVATS_INDEX_DESCRIPTION,
    ANALYSIS_METHODOLOGY,
    LITERATURE_SOURCE,
    BIBLIOGRAPHICAL_REFERENCE,
    MASS_SPECTRUM,
    INTENSITY_SPECTRUM,
    PDF_ATTACHED,
    XLSX_ATTACHED,
    SPECIFIC_ID,

    PDF_TITLE,
    XLSX_TITLE,
    PDF_SIZE,
    PDF_LOAD_DATE,
    XLSX_SIZE,
    XLSX_LOAD_DATE,
    ALL_GROUPS_TITLES,
    LIBRARY_GROUP_ID,
    LIBRARY_ELEMENT_ID,
    CLASSIFIER_ID,
    COMPOUND_CLASS_TYPE_ID,
    TEMPLATE_TYPE_ID,
    TEMPLATE_X_AXIS_ID,
    TEMPLATE_Y_AXIS_ID,
    TEMPLATE_Z_AXIS_ID,
    TEMPLATE_OBJECTS,
    TEMPLATE_X_AXIS_TITLE,
    TEMPLATE_Y_AXIS_TITLE,
    TEMPLATE_Z_AXIS_TITLE,
    TEMPLATE_AXES_TITLES,
    LAST_COLUMN,
  };

public:
  explicit TreeModelDynamicLibraryCompounds(ElementType type = ElementType::COMPOUND, QObject* parent = nullptr);
  virtual void postProcessFetch(QPointer<TreeModelItem> item);

  int checkedCount() const;
  void setDisabledByGroup(const QString& groupName);
  void ResetChecked();
  QList<int> CheckedItemIds() const;
  QList<QPair<int, QString>> CheckedIdWithNames() const;
  void setHeaderCheckBoxesVisible(bool visible);
  void setOverallCheckState(Qt::CheckState state);
  void setBlockingCheckStateEvents(bool blocked);
  bool isItemDisabled(int libraryElementId) const;
  void setSystemCompoundsDisabled(bool disabled);
  void getTemplateObjectsFromServer(int libraryGroupId);
  TreeModelItem *GetItemByLibraryGroupId(int libraryGroupId);

  // QAbstractItemModel interface
public:
  QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
  bool setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role) override;
  QVariant data(const QModelIndex& index, int role) const override;
  bool setData(const QModelIndex& index, const QVariant& data, int role) override;
  virtual bool canFetchMore(const QModelIndex& parent) const override;
  virtual void fetchMore(const QModelIndex& parent) override;
  Qt::ItemFlags flags(QModelIndex const& index) const;

  // TreeModelDynamic interface
public:
  virtual void FetchMore(QPointer<TreeModelItem> item) override;

signals:
  void HeaderCheckStateChanged(Qt::Orientation orientation, Qt::CheckState state);
  void templateObjectsUploaded(int groupId, bool isOk);

private:
  void InitColumns();

protected:
  ElementType Type = ElementType::COMPOUND;
  QList<bool> CheckedStates;
  QString DisabledGroupName;
  QHash<int, QSet<QString>> ElementGroups;
  QMap<int, QPointer<TreeModelItem>> SystemElementsMap;
  Qt::CheckState OverallCheckState = Qt::Unchecked;
  bool ShowHeaderCheckboxes = false;
  bool blockCheckStateEvents = false;
  bool SystemCompoundsDisabled = false;

  // TreeModel interface
public:
  void Reset() override;
};
#endif
