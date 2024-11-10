#pragma once
#ifndef TREE_MODEL_DYNAMIC_CALCULATION_COEFFICIENT_TABLE_H
#define TREE_MODEL_DYNAMIC_CALCULATION_COEFFICIENT_TABLE_H

#include "tree_model_dynamic.h"

#include <QPointer>

class TreeModelItem;

class TreeModelDynamicCalculationCoefficientTable final : public TreeModelDynamic
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
    LAST_COLUMN,
  };

  explicit TreeModelDynamicCalculationCoefficientTable(int tableId, QObject* parent = nullptr);

  QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
  bool setHeaderData(int section, Qt::Orientation orientation, const QVariant& value, int role) override;
  QVariant data(const QModelIndex& index, int role) const override;
  bool setData(const QModelIndex& index, const QVariant& data, int role) override;

  void setOverallCheckState(Qt::CheckState state);

  virtual bool canFetchMore(const QModelIndex& parent) const override;
  virtual void fetchMore(const QModelIndex& parent) override;
  virtual void FetchMore(QPointer<TreeModelItem> item) override;
  virtual void postProcessFetch(QPointer<TreeModelItem> item);
  void setBlockingCheckStateEvents(bool blocked);
  int checkedCount() const;
  int getTableId() const;
  QList<int> getChecked() const;

signals:
  void HeaderCheckStateChanged(Qt::Orientation orientation, Qt::CheckState state);

private:
  int TableId = 0;
  QList<bool> CheckedStates;
  Qt::CheckState OverallCheckState = Qt::Unchecked;
  bool blockCheckStateEvents = false;
};
#endif