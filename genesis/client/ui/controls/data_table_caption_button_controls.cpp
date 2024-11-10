#include "data_table_caption_button_controls.h"

#include "ui_data_table_caption_button_controls.h"
#include "genesis_style/style.h"
#include "logic/notification.h"
#include "logic/known_context_tag_names.h"

#include <QDialogButtonBox>

using namespace Constants;
using namespace Widgets;

namespace Controls
{
  DataTableCaptionButtonControls::DataTableCaptionButtonControls(QString tableTitle, QWidget* parent)
    : QWidget(parent)
  {
    TableName = tableTitle;
    SetupUi();
    ConnectActions();
    SetBuildCancelVisible(false);
  }

  DataTableCaptionButtonControls::~DataTableCaptionButtonControls()
  {
    delete ControlUi;
  }

  void DataTableCaptionButtonControls::SetupBuildSettings(AnalysisType type, const QString& tableTitle)
  {
    TableName = tableTitle;
    BuildContext->BuildPCAAction->setText(type == AnalysisType::PCA ? tr("Rebuild PCA")
                                                                    : tr("Build PCA"));
    BuildContext->BuildMCRAction->setText(type == AnalysisType::MCR ? tr("Rebuild MCR")
                                                                    : tr("Build MCR"));
    BuildContext->BuildPLSAction->setText(type == AnalysisType::PLS ? tr("Rebuild PLS")
                                                                    : tr("Build PLS"));
    BuildContext->BuildPLSPredictAction->setText(type == AnalysisType::PLSPredict ? tr("Rebuild PLS Predict")
                                                                                  : tr("PLS Predict"));

    QString modelName = tr("PCA model");
    switch (type)
    {
    case Constants::AnalysisType::MCR:
      modelName = tr("MCR model");
      break;
    case Constants::AnalysisType::PLS:
      modelName = tr("PLS model");
      break;
    case Constants::AnalysisType::PLSPredict:
      modelName = tr("PLS Predict model");
      break;
    default:
      break;
    }
    ControlUi->ReturnButton->setText(tr("Return to %1").arg(modelName));
    ShowControlForNewModel(type == AnalysisType::NONE);
    SetAnalysisType(AnalysisType::NONE);
  }

  void DataTableCaptionButtonControls::SetAnalysisType(AnalysisType type)
  {
    CurrentType = type;
    const auto isVisible = CurrentType != AnalysisType::NONE;
    ControlUi->Cancel->setVisible(isVisible);
    ControlUi->BuildModel->setVisible(isVisible);
    if (CurrentType != AnalysisType::PLSPredict)
    {
      ControlUi->BuildModel->setText(tr("build model %1").arg(GetTypeName()));
      ControlUi->CaptionLabel->setText(isVisible
        ? tr("Setup model %1").arg(GetTypeName())
        : TableName);
    }
    else
    {
      // Изменить тут падеж
      ControlUi->BuildModel->setText(tr("build model PLS Predict"));
      ControlUi->CaptionLabel->setText(isVisible
        ? tr("Setup model PLS Predict")
        : TableName);
    }

    ControlUi->SetupAnalysis->setVisible(!isVisible);
    emit AnalysisIndexChanged(CurrentType);
  }

  void DataTableCaptionButtonControls::ShowControlForNewModel(bool isNewModel)
  {
    ControlUi->ReturnButton->setVisible(!isNewModel);
    ControlUi->TableControl->setVisible(isNewModel);
  }

  void DataTableCaptionButtonControls::ShowReturnToMarkupAction(bool show)
  {
    Context->OpenMarkupAction->setVisible(show);
  }

  void DataTableCaptionButtonControls::ShowRatioMatrixAction(bool show)
  {
    Context->OpenRatioMatrixAction->setVisible(show);
  }

  void DataTableCaptionButtonControls::ShowCalculateCoefficientAction(bool show)
  {
    Context->CalculationCoefficientTableAction->setVisible(show);
  }

  void DataTableCaptionButtonControls::ShowBuildPlotAction(bool show)
  {
    Context->BuildPlotAction->setVisible(show);
  }

  void DataTableCaptionButtonControls::ShowTableControl(bool show)
  {
    ControlUi->TableControl->setVisible(show);
  }

  void DataTableCaptionButtonControls::SetActiveModule(int module)
  {
    BuildContext->SetActiveModule(module);
    Context->BuildPlotAction->setVisible(module == Names::ModulesContextTags::MBiomarkers);
  }

  void DataTableCaptionButtonControls::SetEnabledCalculateCoefficientAction(bool isEnabled)
  {
    Context->CalculationCoefficientTableAction->setEnabled(isEnabled);

  }

  void DataTableCaptionButtonControls::SetEnabledBuildPlotAction(bool isEnabled)
  {
    Context->BuildPlotAction->setEnabled(isEnabled);
  }


  void DataTableCaptionButtonControls::SetupUi()
  {
    ControlUi = new Ui::DataTableCaptionButtonControls();
    ControlUi->setupUi(this);
    ControlUi->CaptionLabel->setStyleSheet(Style::Genesis::GetH1());
    Context = new DataTableContext(ControlUi->TableControl);
    ControlUi->TableControl->setProperty("menu_secondary", true);
    ControlUi->TableControl->setMenu(Context);
    ControlUi->ReturnButton->setIcon(QIcon(":/resource/icons/icon_button_lt.png"));
    ControlUi->BuildModel->setProperty("blue", true);
    ControlUi->SetupAnalysis->setProperty("menu_secondary", true);

    BuildContext = new BuildModelContext(ControlUi->SetupAnalysis);
    ControlUi->SetupAnalysis->setMenu(BuildContext);
  }

  void DataTableCaptionButtonControls::ConnectActions()
  {
    connect(Context->OpenRatioMatrixAction, &QAction::triggered, this,
      &DataTableCaptionButtonControls::OpenMatrix);

    connect(Context->CalculationCoefficientTableAction, &QAction::triggered,
            this, &DataTableCaptionButtonControls::CalculationCoefficientTable);

    connect(Context->BuildPlotAction, &QAction::triggered,
            this, &DataTableCaptionButtonControls::CreatePlot);
    connect(Context->ExportAction, &QAction::triggered, this,
      &DataTableCaptionButtonControls::Export);

    connect(Context->EditAction, &QAction::triggered, this,
            &DataTableCaptionButtonControls::Edit);

    connect(Context->RemoveAction, &QAction::triggered, this,
            &DataTableCaptionButtonControls::Remove);

    connect(ControlUi->ReturnButton, &QPushButton::clicked, this,
      &DataTableCaptionButtonControls::ReturnTo);

    connect(Context->OpenMarkupAction, &QAction::triggered, this,
      &DataTableCaptionButtonControls::ToMarkupData);

    connect(BuildContext->BuildPCAAction, &QAction::triggered, this,
      [&]() { SetAnalysisType(AnalysisType::PCA); });

    connect(BuildContext->BuildMCRAction, &QAction::triggered, this,
      [&]() { SetAnalysisType(AnalysisType::MCR); });

    connect(BuildContext->BuildPLSAction, &QAction::triggered, this,
      [&]() { SetAnalysisType(AnalysisType::PLS); });

    connect(BuildContext->BuildPLSPredictAction, &QAction::triggered, this,
      [&]() { SetAnalysisType(AnalysisType::PLSPredict); });

    connect(ControlUi->BuildModel, &QPushButton::clicked, this,
      [&]() { emit BuildModel(CurrentType); });

    connect(ControlUi->Cancel, &QPushButton::clicked, this,
      [&]() { SetAnalysisType(AnalysisType::NONE); });

    connect(ControlUi->Cancel, &QPushButton::clicked, this,
      &DataTableCaptionButtonControls::Cancel);
  }

  void DataTableCaptionButtonControls::SetBuildCancelVisible(bool isVisible)
  {
    ControlUi->BuildModel->setVisible(isVisible);
    ControlUi->Cancel->setVisible(isVisible);
  }

  QString DataTableCaptionButtonControls::GetTypeName() const
  {
    switch (CurrentType)
    {
    case Constants::AnalysisType::PCA:
      return tr("PCA");
    case Constants::AnalysisType::MCR:
      return tr("MCR");
    case Constants::AnalysisType::PLS:
      return tr("PLS");
    case Constants::AnalysisType::PLSPredict:
      return tr("PLS Predict");
    default:
      return "";
    }
  }
}
