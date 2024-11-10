#pragma once
#ifndef PLS_LOAD_PLOT_WIDGET_H
#define PLS_LOAD_PLOT_WIDGET_H

#include "pls_plot_widget.h"
#include "plot_toolbar_widget.h"

#include "ui/dialogs/web_dialog_pls_load_plot_pick_axis.h"
#include "logic/structures/pca_data_structures.h"

class LoadPlot;

namespace Widgets
{
  class PlsSetupViewContext;
  using namespace Dialogs;

  class PlsLoadPlotWidget : public PlsPlotWidget
  {
    Q_OBJECT

  public:
    explicit PlsLoadPlotWidget(QWidget* parent = nullptr);
    ~PlsLoadPlotWidget() = default;

    void SetAxisForPC(int xPC, int yPC) override;
    void SetPCMaxCount(int maxCount) override;

    QStringList GetExcludedPeaks() const;
    bool HasExcludedItems() const;
    void ExcludePeaks(const IntVector& peaks);
    void CancelExclude();

  signals:
    void excludedStateChanged(bool hasExcluded);
    void AxisChanged(int yPC, int xPC);
    void AllItemsExcluded();
    void ResetExcludedPeaksNumbers(std::vector<int> peaks);

  private:
    int xAxis = 0;
    int yAxis = 0;

    int MaxCount = -1;
    SelectedTypeView TypeView = SelectedTypeView::DIMENSION2D;
    SpectralGraphicType GraphicLineType = SpectralGraphicType::DOT_STYLE;
    LoadPlot* Plot = nullptr;
    PlotToolbarWidgetPtr PlotToolBar = nullptr;
    QCheckBox* mShowNamesCheckBox = nullptr;
    GPShapeWithLabelItem* ConcentrationPoint = nullptr;

    void SetupLoadPlot();
    void ConnectSignals();
    void OnExcludedStateChanged(bool hasExcluded);
    void ShowPickAxis();
    void ResetExcludedPeaks(QStringList names);
    void rescaleXAxisToYAxis();
    void rescaleYAxisToXAxis();

    Component ReplacePtoIndex(const Component& component);
  };
}
#endif