#pragma once
#ifndef PLS_COUNT_PLOT_WIDGET_H
#define PLS_COUNT_PLOT_WIDGET_H

#include <QBoxLayout>

#include "logic/structures/pca_data_structures.h"

#include "pls_plot_widget.h"
#include "plot_toolbar_widget.h"
#include "plot_legend_widget.h"

class CountsPlot;

namespace Widgets
{
  class PlsSetupViewContext;
  class PlsCountPlotWidget : public PlsPlotWidget
  {
    Q_OBJECT

  public:
    explicit PlsCountPlotWidget(QWidget* parent = nullptr);
    ~PlsCountPlotWidget() = default;

    void SetAxisForPC(int xPC,int yPC) override;
    void SetPCMaxCount(int maxCount) override;

    std::vector<int> GetExcludedSampleIds() const;
    bool HasExcludedItems() const;

    void excludeItems(const std::vector<int>& ids) const;
    void resetExcludeItems(const std::vector<int>& ids) const;

  signals:
    void excludedStateChanged(bool hasExcluded);
    void AxisChanged(int yPC, int xPC);
    void itemsExcluded(std::vector<int> items);
    void excludedItemsReset(std::vector<int> items);

  private:
    CountsPlot* Plot = nullptr;
    PlotToolbarWidgetPtr PlotToolBar = nullptr;
    PlsSetupViewContext* mToolbarContext = nullptr;
    PlotLegendWidget* PlotLegend = nullptr;

    int MaxCount = -1;
    int xAxis = 0;
    int yAxis = 0;
    void SetupCountPlot();
    void ConnectSignals();
    void OnExcludedStateChanged();

    void ShowPickAxis();

    void SetupCustomItemStyle();
    void rescaleXAxisToYAxis();
    void rescaleYAxisToXAxis();
  };
}
#endif