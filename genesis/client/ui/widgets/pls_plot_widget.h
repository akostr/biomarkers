#pragma once
#ifndef PLS_PLOT_WIDGET_H
#define PLS_PLOT_WIDGET_H

#include "plot_widget.h"

namespace Widgets
{
  class PlsPlotWidget : public PlotWidget
  {
    Q_OBJECT

  public:
    PlsPlotWidget(QWidget* parent = nullptr) : PlotWidget(parent) {}
    virtual ~PlsPlotWidget() = default;

    virtual void SetAxisForPC(int yPC, int xPC) = 0;
    virtual void SetPCMaxCount(int maxCount) = 0;
  };
  using PlsPlotWidgetPtr = QSharedPointer<PlsPlotWidget>;
}

#endif
