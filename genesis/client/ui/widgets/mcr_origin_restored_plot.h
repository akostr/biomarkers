#pragma once
#ifndef MCR_ORIGIN_RESTORED_PLOT_H
#define MCR_ORIGIN_RESTORED_PLOT_H

#include "plot_widget.h"

class McrOriginRestoredPlotTiped;

namespace Widgets
{
  class McrOriginRestoredPlot final : public PlotWidget
  {
    Q_OBJECT

  public:
    explicit McrOriginRestoredPlot(QWidget* parent = nullptr);
    ~McrOriginRestoredPlot() override;

    void SetCurveIndex(const std::vector<int>& index);
    QColor GetOriginColor() const;
    QColor GetRestoredColor() const;
    void CancelExclude();
    bool IsPeakExcluded() const;

  signals:
    void ExcludeIsEmpty();
    void OnPointsExcluded();

  private:
    McrOriginRestoredPlotTiped* Plot = nullptr;
    void SetupUi();
    void ModelChanged();
    void PeaksExcluded();
  };
}
#endif