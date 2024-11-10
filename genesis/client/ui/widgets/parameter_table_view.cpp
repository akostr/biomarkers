#include "parameter_table_view.h"

#include "ui_parameter_table_view.h"
#include "ui/item_models/analysis_data_table_model.h"
#include "logic/markup/genesis_markup_enums.h"
#include "ui/itemviews/datatable_item_delegate.h"
#include "ui/controls/double_column_delegate.h"
#include <genesis_style/style.h>
#include "logic/known_json_tag_names.h"

#include <QScopeGuard>

using namespace Models;
using namespace GenesisMarkup;
using namespace Control;

namespace Widgets
{
  ParameterTableView::ParameterTableView(QWidget* parent)
    : QWidget(parent)
  {
    SetupUi();
  }

  ParameterTableView::~ParameterTableView()
  {
    delete AnalysisModel;
    delete WidgetUi;
  }

  void ParameterTableView::FillParameterComboBoxShort()
  {
    WidgetUi->ParametersComboBox->clear();
    Parameters.clear();
    Parameters =
    {
      { tr("Height"), PeakHeight },
      { tr("Area"), PeakArea },
    };
    WidgetUi->ParametersComboBox->addItems(Parameters.keys());
  }

  void ParameterTableView::FillParameterComboBoxFull()
  {
    WidgetUi->ParametersComboBox->clear();
    Parameters.clear();
    Parameters =
    {
      { tr("Height"), PeakHeight },
      { tr("Area"), PeakArea },
      { tr("Covats"), PeakCovatsIndex },
      { tr("Ret time"), PeakRetentionTime },
    };
    WidgetUi->ParametersComboBox->addItems(Parameters.keys());
  }

  void ParameterTableView::FillCustomParameterComboBox()
  {
    WidgetUi->ParametersComboBox->clear();
    Parameters.clear();
    Parameters =
    {
      { tr("Custom"), PeakMarkerWindow},
    };
    WidgetUi->ParametersComboBox->addItems(Parameters.keys());
  }

  void ParameterTableView::SetCheckBoxVisible(bool isVisible)
  {
    WidgetUi->PeakCheckBox->setVisible(isVisible);
    WidgetUi->SampleCheckBox->setVisible(isVisible);
    if (AnalysisModel)
    {
      AnalysisModel->SetShowCheckboxes(isVisible);
    }
    UpdatePeakCheckBoxLabel();
    UpdateSampleCheckBoxLabel();
  }

  void ParameterTableView::SetPeaksCheckBoxVisible(bool isVisible)
  {
    WidgetUi->PeakCheckBox->setVisible(isVisible);
    if (AnalysisModel)
    {
      AnalysisModel->SetShowPeakCheckboxes(isVisible);
    }
    UpdatePeakCheckBoxLabel();
  }

  void ParameterTableView::SetCheckBoxChecked()
  {
    WidgetUi->PeakCheckBox->setChecked(true);
    WidgetUi->SampleCheckBox->setChecked(true);
  }

  void ParameterTableView::SetHideYConcentration(bool isHide)
  {
    // YConcentration column
    WidgetUi->tableView->setColumnHidden(1, isHide);
    if(AnalysisModel)
      AnalysisModel->SetShowYConcentration(!isHide);
    WidgetUi->tableView->setItemDelegateForColumn(1, new DoubleColumnDelegate());
    WidgetUi->tableView->update();
  }

  void ParameterTableView::AllowEmptyConcentration(bool allow)
  {
    if (AnalysisModel)
      AnalysisModel->SetAllowEmptyConcentrations(allow);
  }

  void ParameterTableView::SetModel(AnalysisDataTableModel* model)
  {
    DisconnectSignalsToModel();
    SetPeakState(Qt::Unchecked);
    SetSampleState(Qt::Unchecked);
    AnalysisModel = model;
    ProxyModel = new QSortFilterProxyModel(this);
    ProxyModel->setSourceModel(AnalysisModel);
    ConnectSignalsToModel();
    WidgetUi->tableView->setModel(ProxyModel);
    AnalysisModel->SetCurrentParameter(Parameters[WidgetUi->ParametersComboBox->currentText()]);
  }

  void ParameterTableView::SetSampleState(Qt::CheckState state)
  {
    WidgetUi->SampleCheckBox->setCheckState(state);
  }

  void ParameterTableView::SetPeakState(Qt::CheckState state)
  {
    WidgetUi->PeakCheckBox->setCheckState(state);
  }

  AnalysisDataTableModel* ParameterTableView::GetModel() const
  {
      return AnalysisModel;
  }

  QString ParameterTableView::GetParameterType() const
  {
    switch (Parameters[WidgetUi->ParametersComboBox->currentText()])
    {
    case PeakHeight:
      return JsonTagNames::ValuesHeightData;
    case PeakArea:
      return JsonTagNames::ValuesAreaData;
    case PeakCovatsIndex:
      return JsonTagNames::ValuesCovatsData;
    case PeakRetentionTime:
      return JsonTagNames::ValuesRetTimeData;
    case PeakMarkerWindow:
      return JsonTagNames::ValuesCustomData;
    default:
      return "";
    }
  }

  void ParameterTableView::SetupUi()
  {
    WidgetUi = new Ui::ParameterTableView();
    WidgetUi->setupUi(this);
    WidgetUi->widgetWhiteBase->setProperty("style", "white_base");
    WidgetUi->ParametersLabel->setStyleSheet("background: none; padding-right: 6px;");
    WidgetUi->tableView->setStyleSheet("QTableView::item{ padding : 2px; }");
    WidgetUi->tableView->setProperty("dataDrivenColors", true);
    WidgetUi->tableView->setItemDelegate(new DataTableItemDelegate(WidgetUi->tableView));
    WidgetUi->tableView->horizontalHeader()->setSortIndicatorShown(true);
    WidgetUi->tableView->verticalHeader()->setSortIndicatorShown(false);
    WidgetUi->SampleCheckBox->setTristate(false);
    WidgetUi->PeakCheckBox->setTristate(false);
  }

  void ParameterTableView::ConnectSignalsToModel()
  {
    connect(WidgetUi->ParametersComboBox, &QComboBox::currentTextChanged,
      this, &ParameterTableView::SetCurrentParameterToModel);

    connect(WidgetUi->PeakCheckBox, &QCheckBox::stateChanged,
      this, &ParameterTableView::FillPeaksStateInModel);

    connect(WidgetUi->SampleCheckBox, &QCheckBox::stateChanged,
      this, &ParameterTableView::FillSamplesStateInModel);

    if(AnalysisModel)
      connect(AnalysisModel, &AnalysisDataTableModel::HeaderCheckStateChanged,
        this, &ParameterTableView::UpdateCheckBoxStateByHeaderModel);
  }

  void ParameterTableView::DisconnectSignalsToModel()
  {
    disconnect(WidgetUi->ParametersComboBox, &QComboBox::currentTextChanged,
      this, &ParameterTableView::SetCurrentParameterToModel);

    disconnect(WidgetUi->PeakCheckBox, &QCheckBox::stateChanged,
      this, &ParameterTableView::FillPeaksStateInModel);

    disconnect(WidgetUi->SampleCheckBox, &QCheckBox::stateChanged,
      this, &ParameterTableView::FillSamplesStateInModel);

    if(AnalysisModel)
      disconnect(AnalysisModel, &AnalysisDataTableModel::HeaderCheckStateChanged,
        this, &ParameterTableView::UpdateCheckBoxStateByHeaderModel);
  }

  void ParameterTableView::SetCurrentParameterToModel(const QString& text)
  {
    if (AnalysisModel)
      AnalysisModel->SetCurrentParameter(Parameters[text]);
    WidgetUi->tableView->resizeColumnsToContents();
    WidgetUi->tableView->update();
  }

  void ParameterTableView::FillPeaksStateInModel(int state)
  {
    if (AnalysisModel)
    {
      QScopeGuard updateLabel([&]() { UpdatePeakCheckBoxLabel(); });
      if (state == Qt::PartiallyChecked)
        return;
      AnalysisModel->FillPeaksChecked(state);
    }
  }

  void ParameterTableView::FillSamplesStateInModel(int state)
  {
    if (AnalysisModel)
    {
      QScopeGuard updateLabel([&]() { UpdateSampleCheckBoxLabel(); });
      if (state == Qt::PartiallyChecked)
        return;
      AnalysisModel->FillSamplesChecked(state);
    }
  }

  void ParameterTableView::UpdateCheckBoxStateByHeaderModel(Qt::Orientation orientation, Qt::CheckState state)
  {
    if (orientation == Qt::Vertical)
    {
      WidgetUi->SampleCheckBox->setCheckState(state);
      UpdateSampleCheckBoxLabel();
    }
    if (orientation == Qt::Horizontal)
    {
      WidgetUi->PeakCheckBox->setCheckState(state);
      UpdatePeakCheckBoxLabel();
    }
  }

  void ParameterTableView::UpdateSampleCheckBoxLabel()
  {
    if (AnalysisModel)
    {
      const auto checkedSamples = AnalysisModel->GetCheckedSamples();
      if (checkedSamples.size() > 0)
      {
        auto count = std::count(checkedSamples.begin(), checkedSamples.end(), true);
        WidgetUi->SampleCheckBox->setText(tr("PICKED %n SAMPLES", "", count));
        const auto it = std::find(checkedSamples.begin(), checkedSamples.end(), !checkedSamples.front());
        if (it != checkedSamples.end())
          WidgetUi->SampleCheckBox->setCheckState(Qt::PartiallyChecked);
      }
    }
  }

  void ParameterTableView::UpdatePeakCheckBoxLabel()
  {
    if (AnalysisModel)
    {
      const auto checkedPeaks = AnalysisModel->GetCheckedPeaks();
      if (checkedPeaks.size() > 0)
      {
        auto count = std::count(checkedPeaks.begin(), checkedPeaks.end(), true);
        WidgetUi->PeakCheckBox->setText(tr("PICKED %n PEAKS", "", count));
        const auto it = std::find(checkedPeaks.begin(), checkedPeaks.end(), !checkedPeaks.front());
        if (it != checkedPeaks.end())
          WidgetUi->PeakCheckBox->setCheckState(Qt::PartiallyChecked);
      }
    }
  }
}
