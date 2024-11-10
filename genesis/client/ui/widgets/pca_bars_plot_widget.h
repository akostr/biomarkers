#pragma once
#ifndef PCA_BARS_PLOT_H
#define PCA_BARS_PLOT_H

#include "pca_plot_widget.h"

class GraphicsPlotExtended;
class GPBars;
class GPCurve;
namespace Widgets
{
  class PcaBarsPlotWidget final : public PcaPlotWidget
  {
    Q_OBJECT
  public:
    explicit PcaBarsPlotWidget(QWidget* parent = nullptr);
    ~PcaBarsPlotWidget() override = default;
    void setModel(QPointer<AnalysisEntityModel> model) override;

    bool eventFilter(QObject* target, QEvent* event) override;

    void invalidateBars();

    // QWidget interface
  protected:
    void showEvent(QShowEvent *event) override;

  private:
    void setupBarsPlot();
    void onModelDataChanged(const QHash<int, QVariant>& roleDataMap);
    void onModelReset();

  private:
    GPBars* mBars = nullptr;
    GPCurve* mCurve = nullptr;
    GraphicsPlotExtended* mPlot = nullptr;
  };
}
#endif
