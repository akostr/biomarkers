#pragma once
#ifndef TRIPLOT_MODIFICATOR_H
#define TRIPLOT_MODIFICATOR_H

#include "logic/service/igraphics_plot_axis_modificator.h"

namespace Service
{
  class TriplotAxisModificator : public IGraphicsPlotAxisModificator
  {
  public:
    TriplotAxisModificator() = default;
    ~TriplotAxisModificator() = default;

    void ApplyModificator(GraphicsPlot* plot) const override;
    void UndoModificator(GraphicsPlot* plot) const override;

  private:
    void setVisible(GraphicsPlot* plot, bool showAxis) const;
  };
}
#endif