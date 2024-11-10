#pragma once
#ifndef TRIPLOT_CORTESIAN_TRANSFORM_H
#define TRIPLOT_CORTESIAN_TRANSFORM_H

#include "logic/service/igraphics_plot_cortesian_transform.h"

namespace Service
{
  class TriplotCortesianTransform final : public IGraphicsPlotCortesianTransform
  {
  public:
    /// <summary>
    /// На вход идет QPointF где x это первая координата плота, y вторая.
    /// Учитываем что сумма координат точки равна K
    /// </summary>
    /// <param name="values"></param>
    /// <returns></returns>
    QVector<QPointF> toCortesian(const QVector<QPointF>& values) const override;
  };
}
#endif