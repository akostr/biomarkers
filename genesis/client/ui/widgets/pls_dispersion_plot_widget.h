#pragma once
#ifndef PLS_DISPERSION_PLOT_WIDGET_H
#define PLS_DISPERSION_PLOT_WIDGET_H

#include "pls_plot_widget.h"

namespace Widgets
{
  class PlsDispersionPlotWidget : public PlsPlotWidget
  {
    Q_OBJECT

  public:
    PlsDispersionPlotWidget(QWidget* parent = nullptr);
    ~PlsDispersionPlotWidget() = default;

    void SetAxisForPC(int yPC, int xPC) override {};
    void SetPCMaxCount(int maxCount) override {};

  private:
    void SetupUi();
    void ConnectSignals();
    void UpdateDataFromModel();

    QSharedPointer<GPAxisTickerText> CreateTicker();
  };
}
#endif