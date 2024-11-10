#pragma once
#ifndef MCR_CONCENTRATION_PLOT_H
#define MCR_CONCENTRATION_PLOT_H

#include "plot_widget.h"

namespace Widgets
{
  class MCRCalculationPlot : public PlotWidget
  {
    Q_OBJECT

  public:
    MCRCalculationPlot(QWidget* parent = nullptr);

    std::map<size_t, QColor> GetColors() const;
    void SetColors(const std::map<size_t, QColor>& colors);

    void SetXAxisLabel(const QString& xAxisLabel);
    void SetYAxisLabel(const QString& yAxisLabel);

    void SetModelComponentName(const std::string& componentName);

  signals:
    void PlotChanged();

  protected:
    std::string ComponentName;
    QString XAxisLabel;
    QString YAxisLabel;

    QVBoxLayout* LegendLayout = nullptr;

    void SetupUi();
    void ConnectSignals();
    void mouseDoubleClickEvent(QMouseEvent* event) override;
    void ModelChanged();
  };
}
#endif
