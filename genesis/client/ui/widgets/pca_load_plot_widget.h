#pragma once
#ifndef PCA_LOAD_PLOT_WIDGET_H
#define PCA_LOAD_PLOT_WIDGET_H

#include "pca_plot_widget.h"
#include "plot_toolbar_widget.h"
#include "ui/dialogs/web_dialog_pls_load_plot_pick_axis.h"
// #include "ui/plots/load_plot.h"

class AnalysisPlotRefactor;

namespace Widgets
{
  class PlsSetupViewContext;
  class PcaLoadPlotWidget final : public PcaPlotWidget
  {
    Q_OBJECT

  public:
    explicit PcaLoadPlotWidget(QWidget* parent);
    ~PcaLoadPlotWidget() override = default;
    void setModel(QPointer<AnalysisEntityModel> model) override;

    // void SetAxisForPC(int xPC, int yPC) override;

    // QStringList GetExcludedPeaks() const;
    // bool HasExcludedItems() const;
    // void ExcludePeaks(const QStringList& peaks);

  signals:
    // void AxisChanged(int yPC, int xPC);
    // void ExcludedChanged();

  private:
    void onPlotContextMenuRequested(QContextMenuEvent* event);
    void SetupLoadPlot();
    void ConnectSignals();
    // void OnExcludedStateChanged();
    void ShowPickAxis();
    void onEntityModelDataChanged(const QHash<int, QVariant>& roleDataMap);
    void onEntityModelChanged(uint32_t type, QUuid, uint32_t role, const QVariant&);
    void onEntityModelAboutToReset();
    void onEntityModelReset();
    void rescaleXAxisToYAxis();
    void rescaleYAxisToXAxis();

  private:
    PlotToolbarWidgetPtr PlotToolBar = nullptr;
    QCheckBox* mShowNamesCheckBox = nullptr;
    AnalysisPlotRefactor* mPlot = nullptr;
    QPointer<QMenu> mContextMenu;

    // Component ReplacePtoIndex(const Component& component);
  };
}
#endif
