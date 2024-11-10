#pragma once
#ifndef MCR_DIFFERENCE_PLOT_H
#define MCR_DIFFERENCE_PLOT_H

#include "plot_widget.h"

class McrDifferencePlotTiped;

namespace Widgets
{
  class McrDifferencePlot final : public PlotWidget
  {
    Q_OBJECT

  public:
    explicit McrDifferencePlot(QWidget* parent = nullptr);
    virtual ~McrDifferencePlot() override = default;

    void SetXAxisLabel(const QString& xAxisLabel);
    void SetYAxisLabel(const QString& yAxisLabel);
    void SetModelComponentName(const std::string& componentName);
    void CancelExclude();
    bool IsSampleExcluded() const;

  protected:
    McrDifferencePlotTiped* Plot = nullptr;
    std::string ComponentName;
    QString XAxisLabel;
    QString YAxisLabel;
    std::map<QString, int> CurveByNameId;

    void SetupUi();
    void ConnectPlotSignals();
    void ModelChanged();
    void BuildOriginRestoredCurve();
    void ExcludeCurvesFromModel();
    void ResetExcludeCurves();
    void BuildForNewModel();

  signals:
    void CurveClicked(std::vector<int> ids);
    void OnCurvesExcluded();
    void PlotChanged();
    void ExcludedEmpty();
  };
}
#endif
