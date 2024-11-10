#include "efa_plot.h"

EFAPlot::EFAPlot(QWidget* parentWidget)
  : GraphicsPlotExtended(parentWidget)
{
  xAxis->setLabel(tr("Sample number"));
  yAxis->setLabel(tr("Eigenvalue"));
}

void EFAPlot::AppendCurve(const QVector<double>& keys, const QVector<double>& values, const QColor& color)
{
  auto curvePtr = GraphicsPlotExtended::addCurve(xAxis, yAxis);
  curvePtr->addData(keys, values);

  auto pen = curvePtr->pen();
  pen.setColor(color);
  curvePtr->setPen(pen);
}
