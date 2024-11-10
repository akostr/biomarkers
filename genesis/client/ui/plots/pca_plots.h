#ifndef PCA_PLOTS_H
#define PCA_PLOTS_H

#pragma once

#include <graphicsplot/graphicsplot_extended.h>

class AnalysisPlot : public GraphicsPlotExtended
{
  Q_OBJECT
public:
  AnalysisPlot(QWidget *parentWidget = nullptr);
  virtual ~AnalysisPlot(){};

  void mouseMoveEvent(QMouseEvent *e) override;
  void mousePressEvent(QMouseEvent *e) override;
  void mouseReleaseEvent(QMouseEvent *e) override;

  void SetDefaultAxisRanges(const GPRange& xRange, const GPRange& yRange);
  void ResizeAxisToDefault();
  void SetXAxisLabel(const QString& label);
  void SetYAxisLabel(const QString& label);

  virtual int  GetPointsCount() { return 0; };

  void SetXComponent(double value);
  double GetXComponent() const;

  void SetYComponent(double value);
  double GetYComponent() const;

  void SetMenu(QPointer<QMenu> newMenu);
  QPointer<QMenu> GetMenu() const;

protected slots:
  void processRectSelection(const QRect& rect, QMouseEvent *event) override;

protected:
  virtual void handleHighlighting(QPointF plotPos){};
  virtual void clearData(){};

  double XComponent = 0;
  double YComponent = 0;

private:
  GPRange xRangeDefault;
  GPRange yRangeDefault;

  // QWidget interface
protected:
  void mouseDoubleClickEvent(QMouseEvent *event) override;
};

#endif
