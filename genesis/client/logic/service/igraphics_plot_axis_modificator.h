#pragma once
#ifndef IGRAPHICS_PLOT_AXIS_MODIFICATOR_H
#define IGRAPHICS_PLOT_AXIS_MODIFICATOR_H

#include "iservice.h"

class GraphicsPlot;

namespace Service
{
  class IGraphicsPlotAxisModificator : public IService
  {
  public:
    virtual ~IGraphicsPlotAxisModificator() = default;

    virtual void ApplyModificator(GraphicsPlot* plot) const = 0;
    virtual void UndoModificator(GraphicsPlot* plot) const = 0;
  };
}

#endif