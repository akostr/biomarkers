#pragma once
#ifndef IGRAPHICS_PLOT_COORD_TRANSFORM_H
#define IGRAPHICS_PLOT_COORD_TRANSFORM_H

#include "logic/service/iservice.h"

#include <QVector>
#include <QPointF>

namespace Service
{
  class IGraphicsPlotCortesianTransform : public IService
  {
  public:
    virtual QVector<QPointF> toCortesian(const QVector<QPointF>& values) const = 0;
  };
}
#endif