#include "custom_axis.h"

#include "custom_axis_painter_private.h"
#include "custom_grid.h"

CustomAxis::CustomAxis(GPAxisRect* parent, AxisType type)
  : GPAxis(parent, type)
{
  delete mAxisPainter;
  mAxisPainter = new CustomAxisPainterPrivate(parentPlot());
  delete mGrid;
  mGrid = new CustomGrid(this);
  mGrid->setVisible(false);
}
