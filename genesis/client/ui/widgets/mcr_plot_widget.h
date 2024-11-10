#pragma once
#ifndef MCR_PLOT_WIDGET_H
#define MCR_PLOT_WIDGET_H

#include <QWidget>

#include "logic/models/identity_model.h"
#include "plot_widget.h"

namespace Ui
{
  class McrPlotWidget;
}

class LegendWidget;

namespace Widgets
{
  class McrPlotWidget : public QWidget
  {
    Q_OBJECT
  public:
    explicit McrPlotWidget(QWidget* parent = nullptr);
    ~McrPlotWidget() override;

    void SetModel(const Model::IdentityModelPtr model);
    bool IsSamplesExcluded() const;
    bool IsPeaksExcluded() const;
    bool IsReadyForBuildModel() const;

  signals :
    void UpdateRecalcButtonVisible();

  protected:
    Ui::McrPlotWidget* WidgetUi = nullptr;
    IdentityModelPtr McrModel = nullptr;
    LegendWidget* RestoredLegend = nullptr;
    LegendWidget* OriginLegend = nullptr;

    void SetupUi();
    void ConnectSignals();
    void FillLegendItems();
    void AddLegendItem();
  };
}
#endif