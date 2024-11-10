#pragma once
#ifndef PLS_SLOPE_OFFSEC_COEFFICIENT_PLOT_WIDGET
#define PLS_SLOPE_OFFSEC_COEFFICIENT_PLOT_WIDGET

#include "plot_widget.h"

namespace Widgets
{
  class MultiplePlotWidget : public PlotWidget
  {
    Q_OBJECT

  public:
    MultiplePlotWidget(QWidget* parent = nullptr);
    ~MultiplePlotWidget() = default;

    void AppendPlots(const std::list<PlotWidget*>& list);

  private:
    QVBoxLayout* ScrollLayout = nullptr;

    // QWidget interface
  protected:
    void resizeEvent(QResizeEvent *event);
  };
}
#endif
