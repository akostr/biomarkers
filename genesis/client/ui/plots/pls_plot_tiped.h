#ifndef PLSPLOTTIPED_H
#define PLSPLOTTIPED_H

#include <graphicsplot/graphicsplot_extended.h>

#include <QMap>

class GPCurveTracerItem;
class CurveLabelWidget;

class PlsPlotTiped : public GraphicsPlotExtended
{
  Q_OBJECT

public:
  PlsPlotTiped(QWidget *parent = nullptr, const unsigned &flags = GP::AADefault);
  ~PlsPlotTiped() override;

  void SetDefaultAxisRanges(const GPRange& xRange, const GPRange& yRange);
  void ResizeAxisToDefault();

private:
  void onCurveRemoved(QObject *obj = nullptr);
  void addLegendItem(GPCurve* curvePtr);
  GPCurveTracerItem* createTracerForCurve(GPCurve* curvePtr);

  QMap<GPCurve*, GPCurveTracerItem*> m_tracers;
  QMap<GPCurve*, QPair<CurveLabelWidget*, QLabel*>> m_legendItems;
  QMap<GPCurve*, QMetaObject::Connection> m_connections;

  QPointer<QWidget> m_tipWidget;
  QPointer<QLabel> m_tipCaptionLabel;
  QPointer<QFormLayout> m_tipCurvesForm;

  GPRange xRangeDefault;
  GPRange yRangeDefault;

  // QWidget interface
protected:
  void mouseMoveEvent(QMouseEvent *event) override;
  void leaveEvent(QEvent *event) override;

  // GraphicsPlot interface
public:
  GPCurve *addCurve(GPAxis *keyAxis, GPAxis *valueAxis) override;

protected:
  void mouseDoubleClickEvent(QMouseEvent* event) override;
};
#endif // PLSPLOTTIPED_H
