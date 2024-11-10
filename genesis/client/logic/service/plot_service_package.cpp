#include "plot_service_package.h"

#include "service_locator.h"
#include "igraphics_plot_axis_modificator.h"
#include "logic/plot_modificators/triplot_axis_modificator.h"
#include "ui/plots/graphics_interactive_model_plot.h"
#include "ui/plots/triplot/triplot.h"
#include "igraphics_plot_cortesian_transform.h"
#include "logic/plot_modificators/triplot_cortesian_transform.h"

namespace Service
{
  PlotServicePackage::PlotServicePackage()
  {
    ServiceLocator::Instance().RegisterService<Triplot, IGraphicsPlotAxisModificator, TriplotAxisModificator>();
    ServiceLocator::Instance().RegisterService<Triplot, IGraphicsPlotCortesianTransform, TriplotCortesianTransform>();
  }
}