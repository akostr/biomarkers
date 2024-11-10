#pragma once
#ifndef PLS_RMSE_PLOT_WIDGET_H
#define PLS_RMSE_PLOT_WIDGET_H

#include "pls_plot_widget.h"

namespace Widgets
{
  class PlsRMSEPlotWidget : public PlsPlotWidget
  {
    Q_OBJECT

  public:
    PlsRMSEPlotWidget(QWidget* parent);
    ~PlsRMSEPlotWidget() = default;

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