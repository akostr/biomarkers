#pragma once
#ifndef MCR_ORIGIN_RESTORED_PLOT_TIPED_H
#define MCR_ORIGIN_RESTORED_PLOT_TIPED_H

#include "graphicsplot/graphicsplot_extended.h"

class GPCurveTracerItem;
class CurveLabelWidget;
class McrOriginRestoredMenuContext;

class McrOriginRestoredPlotTiped final : public GraphicsPlotExtended
{
  Q_OBJECT

public:
  explicit McrOriginRestoredPlotTiped(QWidget* parent = nullptr);
  ~McrOriginRestoredPlotTiped() override;

  void SetDefaultAxisRanges(const GPRange& xRange, const GPRange& yRange);
  void ResizeAxisToDefault();

  GPCurve* addCurve(GPAxis* keyAxis, GPAxis* valueAxis) override;
  QVector<int> GetExcludedPeaks();
  void ClearExcludedPeaks();
  void ExcludeRanges(const QVector<GPDataRange>& excluded);
  void ResetLegendItems();

signals:
  void ExcludePrepeared();

private:
  const static inline double TracerSizeByDefault = 10;
  const static inline double DefaultAlpha = 0.2;
  const static inline std::string_view BackLayer = "back";
  const static inline std::string_view FrontLayer = "front";
  const static inline std::string_view TickerLayer = "ticker";

  std::map<GPCurve*, GPCurveTracerItem*> Tracers;
  McrOriginRestoredMenuContext* ContextMenu = nullptr;
  GPRange xRangeDefault;
  GPRange yRangeDefault;

  QPointer<QWidget> m_tipWidget;
  QPointer<QLabel> m_tipCaptionLabel;
  QPointer<QFormLayout> m_tipCurvesForm;
  QMap<GPCurve*, QPair<CurveLabelWidget*, QLabel*>> m_legendItems;

  void SetupUi();
  void ConnectSignals();
  void contextMenuEvent(QContextMenuEvent* event) override;

  void mouseMoveEvent(QMouseEvent* event) override;
  void mousePressEvent(QMouseEvent* e) override;
  void mouseReleaseEvent(QMouseEvent* e) override;
  void CancelExclude();
  void BuildForNewModel();
  void leaveEvent(QEvent* event) override;
  void createTracer(GPCurve* curvePtr);
  void onCurveRemoved(QObject* obj = nullptr);
  void mouseDoubleClickEvent(QMouseEvent* event) override;
  void ExcludePeaks();
  void Clear();
};
#endif