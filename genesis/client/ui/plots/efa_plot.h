#pragma once

#include <graphicsplot/graphicsplot_extended.h>

class EFAPlot : public GraphicsPlotExtended
{
  Q_OBJECT
public:
  EFAPlot(QWidget *parentWidget = nullptr);
  virtual ~EFAPlot(){};

  void AppendCurve(const QVector<double>& keys, const QVector<double>& values, const QColor& color);
};
