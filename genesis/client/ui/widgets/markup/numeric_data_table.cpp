#include "numeric_data_table.h"

#include "ui_numeric_data_table.h"
#include "ui/itemviews/datatable_item_delegate.h"
#include "logic/markup/genesis_markup_enums.h"
#include "logic/markup/markup_data_model.h"
#include "logic/markup/markup_step_controller.h"
#include <genesis_style/style.h>
#include <QSortFilterProxyModel>

using namespace Models;
using namespace GenesisMarkup;

namespace Widgets
{
  NumericDataTable::NumericDataTable(QWidget* parent)
    : IMarkupTab(parent)
  {
    WidgetUi = new Ui::NumericDataTable();
    WidgetUi->setupUi(this);
    WidgetUi->tableView->setStyleSheet("QTableView::item{ padding : 2px; }");
    WidgetUi->tableView->setProperty("dataDrivenColors", true);
    WidgetUi->tableView->horizontalHeader()->setSortIndicatorShown(false);
    WidgetUi->tableView->verticalHeader()->setSortIndicatorShown(false);
    WidgetUi->ParameterLabel->setStyleSheet(Style::Genesis::Fonts::RegularTextSelectList());
    Parameters =
    {
      { tr("Height"), PeakHeight },
      { tr("Area"), PeakArea },
      { tr("Covats"), PeakCovatsIndex },
      { tr("Ret time"), PeakRetentionTime },
    };
    WidgetUi->ParameterComboBox->addItems(Parameters.keys());
    Model = new NumericDataTableModel(this);
    WidgetUi->tableView->setItemDelegate(new DataTableItemDelegate(WidgetUi->tableView));
    WidgetUi->tableView->verticalHeader()->hide();
    SortModel = new QSortFilterProxyModel(this);
    SortModel->setSourceModel(Model);
    SortModel->setSortRole(NumericDataTableModel::ChromaIdRole);
    WidgetUi->tableView->setSortingEnabled(false);
    WidgetUi->tableView->setModel(SortModel);
    Model->SetCurrentParameter(Parameters[WidgetUi->ParameterComboBox->currentText()]);

    connect(WidgetUi->ParameterComboBox, &QComboBox::currentTextChanged, this,
            [&](const QString& key)
            {
              Model->SetCurrentParameter(Parameters[key]);
              WidgetUi->tableView->resizeColumnsToContents();
            });
  }

  NumericDataTable::~NumericDataTable()
  {
    delete Model;
    delete WidgetUi;
  }

  void NumericDataTable::Clear()
  {
    Model->Clear();
  }

  void NumericDataTable::setMarkupModel(GenesisMarkup::MarkupModelPtr model)
  {
    Model->SetModel(model);
    if(model != nullptr)
      SetPassportData(model->controller()->passportsModel().getEntities(DataPassport));
    else
      SetPassportData({});
  }

  void NumericDataTable::SetPassportData(const QList<AbstractEntityDataModel::ConstDataPtr>& passport)
  {
    Model->SetPassportData(passport);
    WidgetUi->tableView->model()->sort(0);
    WidgetUi->tableView->resizeColumnsToContents();
  }
}


bool Widgets::NumericDataTable::isVisibleOnStep(GenesisMarkup::StepInfo step)
{
  return step.guiInteractions.testFlag(GenesisMarkup::SIGDigitalDataTable);
}
