#pragma once

#ifndef PLS_MULTIPLE_PLOT_WIDGET_H
#define PLS_MULTIPLE_PLOT_WIDGET_H

#include "pls_plot_widget.h"
#include "multiple_plot_widget.h"

namespace Widgets
{
  class PlsMultiplePlotWidget : public PlsPlotWidget
  {
    Q_OBJECT

  public:
    PlsMultiplePlotWidget(QWidget* parent);
    ~PlsMultiplePlotWidget() = default;

    void PlsAppendPlots(const std::list<PlotWidget*>& list);
    void SetAxisForPC(int yPC, int xPC);
    void SetPCMaxCount(int maxCount);

    void SetLabelText(const QString& labelText);
    void SetPCMaxCountValue(int value);

  signals :
    void PCMaxCountValueChanged(int value);

  private:
    MultiplePlotWidget* MultiPlotWidget = nullptr;

    void SetupUi();
    void ConnectActions();
    void ModelChanged();
  };
  using PlsMultiplePlotWidgetPtr = QSharedPointer<PlsMultiplePlotWidget>;
}
#endif
