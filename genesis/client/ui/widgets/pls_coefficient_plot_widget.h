#pragma once
#ifndef PLS_COEFFICIENT_PLOT_WIDGET_H
#define PLS_COEFFICIENT_PLOT_WIDGET_H

#include "pls_plot_widget.h"

class PlsCoefficientPlot;

namespace Widgets
{
  class PlsCoefficientPlotWidget : public PlsPlotWidget
  {
    Q_OBJECT

  public:
    explicit PlsCoefficientPlotWidget(QWidget* parent = nullptr);
    ~PlsCoefficientPlotWidget() = default;

    void SetAxisForPC(int yPC, int xPC) override {};
    void SetPCMaxCount(int maxCount);
    void AddExcludedPeaks(const std::vector<int>& peaks);
    void CancelExcludePeaks(const std::vector<int>& peaks);
    void ClearExlcudePeaks();
    IntVector GetExcludedPeaks() const;

  signals:
    void OnPeaksExclude();

  private:
    void SetupUi();
    void ConnectSignals();
    void SwitchCoefficientView(GPCurve::LineStyle type= GPCurve::LineStyle::lsLine);
    void mouseDoubleClickEvent(QMouseEvent* event) override;
    void UpdateCancelButtonVisible();

  private:
    PlsCoefficientPlot* Plot = nullptr;
    QPointer<GPCurve> Curve;
    QPointer<QTabBar> Switcher;
    QPushButton* CancelExcludeButton = nullptr;
    GPRange XRange;
    GPRange YRange;
  };
}
#endif
