#include "triplot_axis_modificator.h"

#include "graphicsplot/graphicsplot.h"

#include "ui/plots/triplot/custom_axis.h"

namespace
{
  const auto kTriplotAxes = { GPAxis::atRight, GPAxis::atBottom, GPAxis::atLeft };
}

namespace Service
{
  void TriplotAxisModificator::ApplyModificator(GraphicsPlot* plot) const
  {
    if (const auto rect = plot->axisRect())
    {
      plot->setAntialiasedElement(GP::AntialiasedElement::aeAxes, true);

      const auto axis = plot->findChildren<CustomAxis*>();
      if (!axis.isEmpty())
      {
        setVisible(plot, true);
        return;
      }

      for (auto pos : kTriplotAxes)
      {
        auto axis = rect->addAxis(pos, new CustomAxis(rect, pos));
        axis->setRange(0, 100);
      }

      const auto axes = rect->axes(GPAxis::atLeft);
      if (auto leftAxis = rect->axis(GPAxis::atLeft, axes.size() - 1))
        leftAxis->setRangeReversed(true);
    }
  }

  void TriplotAxisModificator::UndoModificator(GraphicsPlot* plot) const
  {
    setVisible(plot, false);
  }

  void TriplotAxisModificator::setVisible(GraphicsPlot* plot, bool showAxis) const
  {
    const auto axis = plot->findChildren<CustomAxis*>();
    for (const auto& item : axis)
      item->setVisible(showAxis);
  }
}