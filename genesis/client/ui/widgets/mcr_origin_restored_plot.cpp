#include "mcr_origin_restored_plot.h"

#include "logic/models/mcr_analysis_response_model.h"
#include "ui/plots/mcr_origin_restored_plot_tiped.h"
#include "genesis_style/style.h"

namespace
{
  const static QColor AxisColor(113, 130, 141);
  const static QColor GridColor(204, 217, 224);
}

namespace Widgets
{
  McrOriginRestoredPlot::McrOriginRestoredPlot(QWidget* parent)
    : PlotWidget(parent)
  {
    SetupUi();
    connect(Plot, &McrOriginRestoredPlotTiped::ExcludePrepeared, this, &McrOriginRestoredPlot::PeaksExcluded);
  }

  McrOriginRestoredPlot::~McrOriginRestoredPlot()
  {
    delete Plot;
  }

  void McrOriginRestoredPlot::SetCurveIndex(const std::vector<int>& ids)
  {
    const auto model = ObjectModel->GetModel();
    if (Plot == nullptr || model == nullptr)
      return;

    Plot->clearItems();
    Plot->clearCurves();
    Plot->ResetLegendItems();

    const auto originals = model->Get<MCRCalculationList>(McrAnalysisResponseModel::Originals);
    const auto restored = model->Get<MCRCalculationList>(McrAnalysisResponseModel::Reconstructions);
    const auto analysisData = model->Get<AnalysisData>(McrAnalysisResponseModel::AnalysisData);

    DoubleVector xAxis(originals.front().Values.size());
    std::iota(xAxis.begin(), xAxis.end(), 1);
    for (const auto& curveId : ids)
    {
      const auto it = std::find_if(analysisData.Data.T_Title.begin(), analysisData.Data.T_Title.end(),
        [&](const AnalysisMetaInfo& info) { return info.SampleId == curveId; });
      if (it == analysisData.Data.T_Title.end())
        continue;

      const auto curveName = it->FileName.c_str();
      const auto index = std::distance(analysisData.Data.T_Title.begin(), it);
      AppendCurve(Plot, xAxis, originals[index].Values, GetOriginColor(), 2, GPCurve::lsLine, curveName);
      AppendCurve(Plot, xAxis, restored[index].Values, GetRestoredColor(), 2, GPCurve::lsLine, curveName);
    }
    Plot->xAxis->setLabel(tr("Variance number"));
    Plot->yAxis->setLabel(tr("Intensive"));

    Plot->xAxis->setLabelColor(AxisColor);
    Plot->xAxis->setLabelFont(Style::Genesis::Fonts::RegularText());
    Plot->xAxis->setTickLabelColor(AxisColor);
    Plot->xAxis->setTickLabelFont(Style::Genesis::Fonts::AxisTicks());

    Plot->yAxis->setLabelFont(Style::Genesis::Fonts::RegularText());
    Plot->yAxis->setLabelColor(AxisColor);
    Plot->yAxis->setTickLabelColor(AxisColor);
    Plot->yAxis->setTickLabelFont(Style::Genesis::Fonts::AxisTicks());

    if (!originals.empty())
      Plot->SetDefaultAxisRanges(CreateRangeFromVector(xAxis, 0), CreateRangeFromVector(originals.front().Values));
  }

  QColor McrOriginRestoredPlot::GetOriginColor() const
  {
    return QColor(102, 121, 133);
  }

  QColor McrOriginRestoredPlot::GetRestoredColor() const
  {
    return QColor(243, 67, 56);
  }

  void McrOriginRestoredPlot::CancelExclude()
  {
    const auto model = ObjectModel->GetModel();
    if (!model)
      return;
    const auto pids = model->Get<IntVector>(McrAnalysisIdentityModel::Pids);
    model->Exec<AnalysisTable>(McrAnalysisIdentityModel::AnalysisTable,
      [&](AnalysisTable& table)
      {
        table.CheckedValues.MarkersIds = pids;
      });
    if (Plot)
      Plot->ClearExcludedPeaks();
    emit ExcludeIsEmpty();
  }

  bool McrOriginRestoredPlot::IsPeakExcluded() const
  {
    return !Plot->GetExcludedPeaks().isEmpty();
  }

  void McrOriginRestoredPlot::SetupUi()
  {
    const auto box = findChild<QVBoxLayout*>();
    Plot = new McrOriginRestoredPlotTiped(this);
    Plot->setZoomModifiers(Qt::ControlModifier);
    Plot->setAttribute(Qt::WA_NoMousePropagation, false);
    box->addWidget(Plot);
    connect(ObjectModel.get(), &ObjectWithModel::ModelChanged, this, &McrOriginRestoredPlot::ModelChanged);
  }

  void McrOriginRestoredPlot::ModelChanged()
  {
    if (Plot)
    {
      Plot->clearItems();
      Plot->clearCurves();
      Plot->ResetLegendItems();
    }
  }

  void McrOriginRestoredPlot::PeaksExcluded()
  {
    const auto model = ObjectModel->GetModel();
    if (!model)
      return;

    // pids started with 0
    const auto pids = model->Get<IntVector>(McrAnalysisIdentityModel::Pids);
    const auto peaks = Plot->GetExcludedPeaks(); // started with 1
    if (peaks.isEmpty())
      return;

    IntVector checkedPeaks;
    checkedPeaks.reserve(pids.size() - peaks.size());
    for (size_t index = 0; index < pids.size(); index++)
    {
      if (peaks.contains(index + 1)) // + offset between pids and peaks
        continue;
      checkedPeaks.append(pids[index]);
    }
    model->Exec<AnalysisTable>(McrAnalysisIdentityModel::AnalysisTable,
      [&](AnalysisTable& table)
      {
        table.CheckedValues.MarkersIds = checkedPeaks;
      });

    emit OnPointsExcluded();
  }
}
