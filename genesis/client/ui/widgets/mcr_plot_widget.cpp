#include "mcr_plot_widget.h"

#include "ui_mcr_plot_widget.h"

#include "logic/models/mcr_analysis_response_model.h"
#include "genesis_style/style.h"
#include "ui/chromotogram_page/legend_widget.h"

using namespace Model;

namespace
{
  const static QString LabelColor("QLabel { color : #0071B2; }");
  const static int MinimumItems = 3;
}

namespace Widgets
{
  McrPlotWidget::McrPlotWidget(QWidget* parent)
    : QWidget(parent)
  {
    SetupUi();
    ConnectSignals();
  }

  McrPlotWidget::~McrPlotWidget()
  {
    delete WidgetUi;
  }

  void McrPlotWidget::SetModel(const IdentityModelPtr model)
  {
    McrModel = model;
    WidgetUi->ConcentrationPlot->SetModel(model);
    const auto concentrations = model->Get<MCRCalculationList>(McrAnalysisResponseModel::Difference);
    WidgetUi->DifferencePlot->SetModel(model);
    WidgetUi->SpectralPlot->SetModel(model);
    FillLegendItems();
    WidgetUi->OriginalRestoredPlot->SetModel(model);
    model->Exec<MCRCalculatedParameters>(McrAnalysisIdentityModel::CalculatedParameters,
      [&](const MCRCalculatedParameters& parameters)
      {
        WidgetUi->CaptionLabel->setText(
          tr("Numeric values : Lof - %1, MSE - %2, Variance - %3, Iteration count - %4")
          .arg(parameters.Lof)
          .arg(parameters.MSE)
          .arg(parameters.Vairance)
          .arg(parameters.NIter));
      });
  }

  bool McrPlotWidget::IsSamplesExcluded() const
  {
    if (WidgetUi && WidgetUi->DifferencePlot)
      return WidgetUi->DifferencePlot->IsSampleExcluded();
    return false;
  }

  bool McrPlotWidget::IsPeaksExcluded() const
  {
    if (WidgetUi && WidgetUi->OriginalRestoredPlot)
      return WidgetUi->OriginalRestoredPlot->IsPeakExcluded();
    return false;
  }

  bool McrPlotWidget::IsReadyForBuildModel() const
  {
    bool isReady = false;
    McrModel->Exec<AnalysisTable>(McrAnalysisIdentityModel::AnalysisTable,
      [&](const AnalysisTable& table)
      {
        isReady = table.CheckedValues.SamplesIds.size() >= MinimumItems
          && table.CheckedValues.MarkersIds.size() >= MinimumItems;
      });
    return isReady;
  }

  void McrPlotWidget::SetupUi()
  {
    WidgetUi = new Ui::McrPlotWidget();
    WidgetUi->setupUi(this);
    WidgetUi->CaptionLabel->setStyleSheet(Style::Genesis::Fonts::RegularText());

    WidgetUi->cancelExcludeFromDifferenceButton->setVisible(false);
    WidgetUi->cancelExcludeFromOriginRestoreButton->setVisible(false);

    WidgetUi->ConcentrationPlotLabel->setStyleSheet(
      Style::Genesis::GetH3()
      + LabelColor);
    WidgetUi->ConcentrationPlot->SetXAxisLabel(tr("Sample"));
    WidgetUi->ConcentrationPlot->SetYAxisLabel(tr("Part"));
    WidgetUi->ConcentrationPlot->SetModelComponentName(McrAnalysisResponseModel::Concentrations);

    WidgetUi->SpectalPlotLabel->setStyleSheet(Style::Genesis::GetH3()
      + LabelColor);
    WidgetUi->SpectralPlot->SetXAxisLabel(tr("Variance number"));
    WidgetUi->SpectralPlot->SetYAxisLabel(tr("Intensive"));
    WidgetUi->SpectralPlot->SetModelComponentName(McrAnalysisResponseModel::Spectra);

    WidgetUi->DifferenceLabel->setStyleSheet(Style::Genesis::GetH3()
      + LabelColor);
    WidgetUi->DifferencePlot->SetXAxisLabel(tr("Variance number"));
    WidgetUi->DifferencePlot->SetYAxisLabel(tr("Intensive"));
    WidgetUi->DifferencePlot->SetModelComponentName(McrAnalysisResponseModel::Difference);

    WidgetUi->OriginalRestoredLabel->setStyleSheet(Style::Genesis::GetH3()
      + LabelColor);

    WidgetUi->legendLabel->setStyleSheet(Style::Genesis::GetH3());
  }

  void McrPlotWidget::ConnectSignals()
  {
    connect(WidgetUi->DifferencePlot, &McrDifferencePlot::CurveClicked,
      WidgetUi->OriginalRestoredPlot, &McrOriginRestoredPlot::SetCurveIndex);

    connect(WidgetUi->DifferencePlot, &McrDifferencePlot::CurveClicked,
      this, &McrPlotWidget::AddLegendItem);

    connect(WidgetUi->DifferencePlot, &McrDifferencePlot::OnCurvesExcluded, this, &McrPlotWidget::UpdateRecalcButtonVisible);
    connect(WidgetUi->DifferencePlot, &McrDifferencePlot::OnCurvesExcluded,
      this,
      [&]()
      {
        WidgetUi->cancelExcludeFromDifferenceButton->setVisible(true);
      });

    connect(WidgetUi->DifferencePlot, &McrDifferencePlot::ExcludedEmpty, this, &McrPlotWidget::UpdateRecalcButtonVisible);
    connect(WidgetUi->DifferencePlot, &McrDifferencePlot::ExcludedEmpty,
      this,
      [&]()
      {
        WidgetUi->cancelExcludeFromDifferenceButton->setVisible(false);
      });

    connect(WidgetUi->cancelExcludeFromDifferenceButton, &QPushButton::clicked, this, &McrPlotWidget::UpdateRecalcButtonVisible);
    connect(WidgetUi->cancelExcludeFromDifferenceButton, &QPushButton::clicked, WidgetUi->DifferencePlot, &McrDifferencePlot::CancelExclude);
    connect(WidgetUi->cancelExcludeFromOriginRestoreButton, &QPushButton::clicked, this, &McrPlotWidget::UpdateRecalcButtonVisible);
    connect(WidgetUi->cancelExcludeFromOriginRestoreButton, &QPushButton::clicked, WidgetUi->OriginalRestoredPlot, &McrOriginRestoredPlot::CancelExclude);
    connect(WidgetUi->OriginalRestoredPlot, &McrOriginRestoredPlot::OnPointsExcluded, this, &McrPlotWidget::UpdateRecalcButtonVisible);
    connect(WidgetUi->OriginalRestoredPlot, &McrOriginRestoredPlot::OnPointsExcluded, this, [&]()
      {
        WidgetUi->cancelExcludeFromOriginRestoreButton->setVisible(true);
      });
    connect(WidgetUi->OriginalRestoredPlot, &McrOriginRestoredPlot::ExcludeIsEmpty, this, &McrPlotWidget::UpdateRecalcButtonVisible);
    connect(WidgetUi->OriginalRestoredPlot, &McrOriginRestoredPlot::ExcludeIsEmpty, this, [&]()
      {
        WidgetUi->cancelExcludeFromOriginRestoreButton->setVisible(false);
      });
  }

  void McrPlotWidget::FillLegendItems()
  {
    qDeleteAll(WidgetUi->legendGridLayout->findChildren<LegendWidget*>());
    const auto colors = WidgetUi->ConcentrationPlot->GetColors();
    const static int colCount = 3;
    int indexer = 0;
    bool isEtalon = false;
    McrModel->Exec<MCRParameters>(McrAnalysisIdentityModel::InputParameters,
      [&](const MCRParameters& params) { isEtalon = params.Method == "etalon"; });
    const auto data = McrModel->Get<AnalysisData>(McrAnalysisIdentityModel::AnalysisData);
    for (const auto& [id, color] : colors)
    {
      auto item = new LegendWidget();
      item->setContentsMargins(5, 5, 5, 5);
      item->setShape(GPShape::ShapeType::Square);
      item->setShapeBrush(color);
      item->setShapePen(Qt::NoPen);
      item->setSpacing(5);
      const QString legendTitle = (isEtalon)
        ? QString::fromStdString(data.Data.T_Title[id].FileName)
        : tr("Component %1").arg(id); //if you need to fix numeration to id+1 in legend, backend sholuld also fix numeration for concentrations_column
      item->setText(legendTitle);
      WidgetUi->legendGridLayout->addWidget(item, indexer / colCount, indexer % colCount);
      indexer++;
    }
  }

  void McrPlotWidget::AddLegendItem()
  {
    if (!OriginLegend)
    {
      OriginLegend = new LegendWidget(this);
      OriginLegend->setContentsMargins(5, 5, 5, 5);
      OriginLegend->setShape(GPShape::ShapeType::Square);
      OriginLegend->setShapePen(Qt::NoPen);
      OriginLegend->setSpacing(5);
      WidgetUi->legendGridLayout->addWidget(OriginLegend);
    }
    OriginLegend->setShapeBrush(WidgetUi->OriginalRestoredPlot->GetOriginColor());
    OriginLegend->setText(tr("Origin profile"));

    if (!RestoredLegend)
    {
      RestoredLegend = new LegendWidget(this);
      RestoredLegend->setContentsMargins(5, 5, 5, 5);
      RestoredLegend->setShape(GPShape::ShapeType::Square);
      RestoredLegend->setShapePen(Qt::NoPen);
      RestoredLegend->setSpacing(5);
      WidgetUi->legendGridLayout->addWidget(RestoredLegend);
    }
    RestoredLegend->setShapeBrush(WidgetUi->OriginalRestoredPlot->GetRestoredColor());
    RestoredLegend->setText(tr("Restored profile"));
  }
}
