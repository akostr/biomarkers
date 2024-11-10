#pragma once
#ifndef NUMERIC_DATA_TABLE_H
#define NUMERIC_DATA_TABLE_H

#include <QWidget>
#include <QSortFilterProxyModel>

#include "logic/markup/genesis_markup_forward_declare.h"
#include "logic/markup/i_markup_tab.h"
#include "ui/item_models/numeric_data_table_model.h"

namespace Ui
{
  class NumericDataTable;
}
namespace Widgets
{
  class NumericDataTable : public IMarkupTab
  {
    Q_OBJECT

  public:
    NumericDataTable(QWidget* parent = nullptr);
    ~NumericDataTable();

    void Clear();
    void SetPassportData(const QList<AbstractEntityDataModel::ConstDataPtr>& passport);

  private:
    Ui::NumericDataTable* WidgetUi = nullptr;
    QMap<QString, int> Parameters;
    Models::NumericDataTableModel* Model = nullptr;
    QSortFilterProxyModel* SortModel = nullptr;

    // IMarkupTab interface
  public:
    void setMarkupModel(GenesisMarkup::MarkupModelPtr model) override;
    bool isVisibleOnStep(GenesisMarkup::StepInfo step) override;
  };
}
#endif
