#pragma once
#ifndef MCR_PLOT_TIPED_H
#define MCR_PLOT_TIPED_H

#include "graphicsplot/graphicsplot_extended.h"

class GPCurveTracerItem;
class CurveLabelWidget;
class McrPointInfo;
class McrDifferentMenuContext;

class McrDifferencePlotTiped final : public GraphicsPlotExtended
{
  Q_OBJECT

public:
  explicit McrDifferencePlotTiped(QWidget* parent = nullptr);
  ~McrDifferencePlotTiped() override;

  void SetDefaultAxisRanges(GPRange&& xRange, GPRange&& yRange);
  void ResizeAxisToDefault();
  void Clear();
  void CancelExclude();
  void CancelExclude(const QList<GPCurve*>& curves);
  void UpdateActionContextVisibility();

  GPCurve* addCurve(GPAxis* keyAxis, GPAxis* valueAxis) override;
  QList<GPCurve*> GetExcludedCurves() const;

signals :
  void BuildOriginRestoredAction();
  void CurvesExcluded();
  void ExcludedEmpty();
  void BuildForNewModel();

protected:
  const static inline double TracerSizeByDefault = 10;
  const static inline double DefaultAlpha = 0.2;
  const static inline int MinimumLength = 100;
  const static inline std::string_view BackLayer = "back";
  const static inline std::string_view FrontLayer = "front";
  const static inline std::string_view TickerLayer = "ticker";

  QWidget* FloatingMainWindget = nullptr;
  McrPointInfo* FloatingWidget = nullptr;
  std::map<GPCurve*, GPCurveTracerItem*> Tracers;
  std::map<GPCurve*, std::pair<CurveLabelWidget*, QLabel*>> m_legendItems;
  GPRange xRangeDefault;
  GPRange yRangeDefault;
  McrDifferentMenuContext* ContextMenu = nullptr;
  QList<GPCurve*> ExcludedCurves;

  void SetupUi();
  void ConnectSignals();
  void mouseMoveEvent(QMouseEvent* event) override;
  void leaveEvent(QEvent* event) override;
  void createTracer(GPCurve* curvePtr);
  void onCurveRemoved(QObject* obj = nullptr);
  void contextMenuEvent(QContextMenuEvent* event) override;
  void PlottableClicked(GPAbstractPlottable* plottable, int dataIndex, QMouseEvent* event);
  void ExcludeCurves();
  void ExcludeUnselectedCurves();
  void CancelExcludeCurvesFromAction();
  void ChangeCurveTransparency(GPCurve* curve, float alpha = 1);
  void mouseDoubleClickEvent(QMouseEvent* event) override;
};

#endif