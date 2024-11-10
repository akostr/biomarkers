#pragma once
#ifndef PLS_COEFFICIENT_PLOT_H
#define PLS_COEFFICIENT_PLOT_H

#include "graphicsplot/graphicsplot_extended.h"

namespace Widgets
{
  class SlopeOffsetContext;
}

class PlsCoefficientPlot final : public GraphicsPlotExtended
{
  Q_OBJECT

public:
  explicit PlsCoefficientPlot(QWidget* parent = nullptr);
  ~PlsCoefficientPlot() override;

  void SetDefaultAxisRanges(const GPRange& xRange, const GPRange& yRange);
  void ResizeAxisToDefault();

  GPCurve* addCurve(GPAxis* keyAxis, GPAxis* valueAxis) override;
  void AddExcludedPeaks(const std::vector<int>& peaks);
  void CancelExcludePeaks(const std::vector<int>& peaks);
  bool ExcludedExists();
  QVector<int> GetSelectedExcludedPeaks();
  void ClearExcludedPeaks();
  void CancelSelectedExcludePeaks();
  void PickForNewModel();
  void HideBuildForNewModel();

signals:
  void ExcludePrepeared();

private:
  Widgets::SlopeOffsetContext* ContextMenu = nullptr;

  GPRange xRangeDefault;
  GPRange yRangeDefault;

  void SetupUi();
  void ConnectSignals();

  void contextMenuEvent(QContextMenuEvent* event) override;
  void mousePressEvent(QMouseEvent* e) override;
  void mouseReleaseEvent(QMouseEvent* e) override;

  void ExcludeSelectedPoints();
};
#endif