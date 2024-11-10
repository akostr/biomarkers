#pragma once
#ifndef PLOT_LEGEND_WIDGET_H
#define PLOT_LEGEND_WIDGET_H

#include <QWidget>

namespace Ui
{
  class PlotLegendWidget;
}

class CountsPlot;
class FlowLayout;

class PlotLegendWidget : public QWidget
{
  Q_OBJECT

public:
  PlotLegendWidget(CountsPlot* plot, QWidget* parent = nullptr);
  ~PlotLegendWidget();

  void OnLegendChanged();

private:
  FlowLayout* ShapeLegendLayout;
  FlowLayout* ColorLegendLayout;
  FlowLayout* ExceptionsLegendLayout;

private:
  Ui::PlotLegendWidget* WidgetUi = nullptr;
  CountsPlot* Plot = nullptr;

  void SetupUi();
  void ConnectSignals();
  void ShowEditDialog();

  void RemoveWidgetsFrom(QLayout* layout);
};
#endif
