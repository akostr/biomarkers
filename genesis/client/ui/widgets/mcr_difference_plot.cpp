#include "mcr_difference_plot.h"

#include "ui/plots/mcr_difference_plot_tiped.h"
#include "logic/models/mcr_analysis_response_model.h"
#include "genesis_style/style.h"

namespace
{
  const static QColor CurveColor(0, 32, 51);
  const static QColor AxisColor(113, 130, 141);
  const static QColor GridColor(204, 217, 224);
}

namespace Widgets
{
  McrDifferencePlot::McrDifferencePlot(QWidget* parent)
    : PlotWidget(parent)
  {
    SetupUi();
    ConnectPlotSignals();
  }

  void McrDifferencePlot::SetXAxisLabel(const QString& xAxisLabel)
  {
    XAxisLabel = xAxisLabel;
  }

  void McrDifferencePlot::SetYAxisLabel(const QString& yAxisLabel)
  {
    YAxisLabel = yAxisLabel;
  }

  void McrDifferencePlot::SetModelComponentName(const std::string& componentName)
  {
    ComponentName = componentName;
  }

  void McrDifferencePlot::CancelExclude()
  {
    Plot->CancelExclude();
  }

  bool McrDifferencePlot::IsSampleExcluded() const
  {
    if (Plot)
      return !Plot->GetExcludedCurves().isEmpty();
    return false;
  }

  void McrDifferencePlot::SetupUi()
  {
    const auto box = findChild<QVBoxLayout*>();
    Plot = new McrDifferencePlotTiped(this);
    Plot->setZoomModifiers(Qt::ControlModifier);
    Plot->setAttribute(Qt::WA_NoMousePropagation, false);
    Plot->xAxis->setLabelColor(AxisColor);
    Plot->xAxis->setLabelFont(Style::Genesis::Fonts::RegularText());
    Plot->xAxis->setTickLabelColor(AxisColor);
    Plot->xAxis->setTickLabelFont(Style::Genesis::Fonts::AxisTicks());

    Plot->yAxis->setLabelFont(Style::Genesis::Fonts::RegularText());
    Plot->yAxis->setLabelColor(AxisColor);
    Plot->yAxis->setTickLabelColor(AxisColor);
    Plot->yAxis->setTickLabelFont(Style::Genesis::Fonts::AxisTicks());

    box->addWidget(Plot);

    emit PlotChanged();
  }

  void McrDifferencePlot::ConnectPlotSignals()
  {
    connect(ObjectModel.get(), &ObjectWithModel::ModelChanged, this, &McrDifferencePlot::ModelChanged);
    if (Plot)
    {
      connect(Plot, &McrDifferencePlotTiped::BuildOriginRestoredAction, this, &McrDifferencePlot::BuildOriginRestoredCurve);
      connect(Plot, &McrDifferencePlotTiped::CurvesExcluded, this, &McrDifferencePlot::ExcludeCurvesFromModel);
      connect(Plot, &McrDifferencePlotTiped::ExcludedEmpty, this, &McrDifferencePlot::ResetExcludeCurves);
      connect(Plot, &McrDifferencePlotTiped::BuildForNewModel, this, &McrDifferencePlot::BuildForNewModel);
    }
  }

  void McrDifferencePlot::ModelChanged()
  {
    if (Plot == nullptr)
      return;

    CurveByNameId.clear();
    Plot->clearItems();
    Plot->clearCurves();
    Plot->CancelExclude();

    const auto model = ObjectModel->GetModel();
    if (model == nullptr)
      return;

    const auto concentrations = model->Get<MCRCalculationList>(ComponentName);
    if (concentrations.empty())
      return;

    const auto analysisData = model->Get<AnalysisData>(McrAnalysisResponseModel::AnalysisData);
    double yMax = 0;
    double yMin = 0;


    DoubleVector xAxis(concentrations.front().Values.size());
    std::iota(xAxis.begin(), xAxis.end(), 1);
    const auto xMax = xAxis.size();
    const auto xMin = *std::min_element(xAxis.begin(), xAxis.end());
    int index = 0;
    for (const auto& curve : concentrations)
    {
      const auto id = static_cast<int>(curve.Id) == -1
        ? curve.ComponentNumber
        : curve.Id;

      const auto legendItemName = static_cast<int>(id) > -1
        ? analysisData.Data.T_Title[index].FileName.c_str()
        : tr("Component %1").arg(id); //if you need to fix numeration to id+1 in legend, backend sholuld also fix numeration for concentrations_column

      AppendCurve(Plot, xAxis, curve.Values, CurveColor,
        1, GPCurve::LineStyle::lsLine, legendItemName);
      CurveByNameId.emplace(legendItemName, id);
      yMax = std::max(yMax, *std::max_element(curve.Values.begin(), curve.Values.end()));
      yMin = std::min(yMin, *std::min_element(curve.Values.begin(), curve.Values.end()));
      index++;
    }
    Plot->xAxis->setLabel(XAxisLabel);
    Plot->yAxis->setLabel(YAxisLabel);

    Plot->SetDefaultAxisRanges(GPRange(xMin, xMax), GPRange(yMin, yMax));
  }

  void McrDifferencePlot::BuildOriginRestoredCurve()
  {
    const auto items = Plot->selectedCurves();
    if (items.isEmpty())
      return;

    std::vector<int> selectedIds;
    selectedIds.reserve(items.size());
    for (const auto curve : items)
    {
      const auto it = CurveByNameId.find(curve->name());
      if (it != CurveByNameId.end())
        selectedIds.push_back(it->second);
    }
    emit CurveClicked(selectedIds);
  }

  void McrDifferencePlot::ExcludeCurvesFromModel()
  {
    ResetExcludeCurves();
    const auto model = ObjectModel->GetModel();
    const auto ecurves = Plot->GetExcludedCurves();
    std::vector<int> excludedIds;
    excludedIds.reserve(ecurves.size());
    for (const auto& curve : ecurves)
    {
      const auto it = CurveByNameId.find(curve->name());
      if (it != CurveByNameId.end())
        excludedIds.push_back(it->second);
    }
    model->Exec<AnalysisTable>(AnalysisIdentityModel::AnalysisTable,
      [&](AnalysisTable& table)
      {
        for (const auto& id : excludedIds)
          table.CheckedValues.SamplesIds.removeOne(id);
      });
    emit OnCurvesExcluded();
  }

  void McrDifferencePlot::ResetExcludeCurves()
  {
    const auto model = ObjectModel->GetModel();
    model->Exec<AnalysisTable>(AnalysisIdentityModel::AnalysisTable,
      [&](AnalysisTable& table)
      {
        for (const auto& [name, id] : CurveByNameId)
        {
          if (!table.CheckedValues.SamplesIds.contains(id))
            table.CheckedValues.SamplesIds.append(id);
        }
      });
    emit ExcludedEmpty();
  }

  void McrDifferencePlot::BuildForNewModel()
  {
    const auto model = ObjectModel->GetModel();
    const auto selectedCurves = Plot->selectedCurves();
    const auto excludedCurves = Plot->GetExcludedCurves();

    model->Exec<AnalysisTable>(AnalysisIdentityModel::AnalysisTable,
      [&](AnalysisTable& table)
      {
        for (const auto& curve : excludedCurves)
        {
          const auto it = CurveByNameId.find(curve->name());
          if (it != CurveByNameId.end())
            table.CheckedValues.SamplesIds.removeOne(it->second);
        }

        for (const auto& curve : selectedCurves)
        {
          const auto it = CurveByNameId.find(curve->name());
          if (it != CurveByNameId.end()
            && !table.CheckedValues.SamplesIds.contains(it->second))
              table.CheckedValues.SamplesIds.append(it->second);
        }
      });

    if (excludedCurves.isEmpty())
      emit ExcludedEmpty();
    else
      emit OnCurvesExcluded();
  }
}
