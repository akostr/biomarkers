#pragma once
#ifndef PLS_SLOPE_OFFSET_PLOT_WIDGET_H
#define PLS_SLOPE_OFFSET_PLOT_WIDGET_H

#include "pls_plot_widget.h"
#include "plot_toolbar_widget.h"

#include "ui/plots/slope_offset_plot.h"
#include "logic/structures/common_structures.h"

namespace Widgets
{
  class PlsSetupViewContext;

  class PlsSlopeOffsetPlotWidget : public PlsPlotWidget
  {
    Q_OBJECT
  public:
    explicit PlsSlopeOffsetPlotWidget(QWidget* parent = nullptr);
    ~PlsSlopeOffsetPlotWidget() = default;

    void SetPCMaxCount(int maxCount) override;

    void excludeItems(const std::vector<int>& items) const;
    void resetExcludeItems(const std::vector<int>& items) const;

  signals:
    void itemsExcluded(std::vector<int> ids);
    void excludedItemsReset(std::vector<int> ids);

  private:
    void onShowStatisticsStateChanged(bool showStatistics);
    void onOpenConcentrationTable(bool openConcentration);
    QMargins statisticWidgetMarginsFromAxisRect;

    std::map<int, QString> PlotTitlesGrad;
    std::map<int, QString> PlotTitlesProv;
    PlotToolbarWidgetPtr PlotToolBar = nullptr;
    PlsSetupViewContext* mToolbarContext = nullptr;
    QPointer<QWidget> StatisticsWidget = nullptr;
    QPointer<QHBoxLayout> StatisticsOverlayLayout = nullptr;
    QPointer<SlopeOffsetPlot> SlopeOffsetPlotWidget = nullptr;
    std::map<int, GPAbstractItem*> GradItems;
    std::map<int, GPAbstractItem*> ProvItems;
    Structures::ConcetrantionTable GetConcentrationTable() const;
    QList<GPAbstractItem*> GetItemsByFileId(const std::vector<int>& ids) const;

    void SetupUi();
    void ConnectSignals();
    void SetupPlot();
    void SetAxisForPC(int, int) override {};

    // QWidget interface
  protected:
    int CurrentComponent = 0;
    void showEvent(QShowEvent *event);
  };
}
#endif
