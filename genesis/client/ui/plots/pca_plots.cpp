#include "pca_plots.h"

#include <ui/plots/gp_items/sample_info_plate.h>
#include <ui/plots/gp_items/gpshape_with_label_item.h>

AnalysisPlot::AnalysisPlot(QWidget* parentWidget)
  : GraphicsPlotExtended(parentWidget)
{
  setFlag(GP::AAMagnifier, false);
  if (!mSelectionRect)
  {
    setSelectionRect(new GPSelectionRect(this));
  }
  auto rect = selectionRect();
  rect->setPen(QPen(QColor(0, 120, 210), 2));
  rect->setBrush(QBrush(QColor(0, 32, 51, 13)));
  setSelectionRectMode(GP::srmSelect);
}

void AnalysisPlot::mouseMoveEvent(QMouseEvent* e)
{
  handleHighlighting(e->pos());
  GraphicsPlotExtended::mouseMoveEvent(e);
}

void AnalysisPlot::mousePressEvent(QMouseEvent* e)
{
  if (e->modifiers().testFlag(Qt::ControlModifier))
  {
    setSelectionRectMode(GP::srmNone);
  }
  else
  {
    setSelectionRectMode(GP::srmSelect);
  }
  GraphicsPlotExtended::mousePressEvent(e);
}

void AnalysisPlot::mouseReleaseEvent(QMouseEvent* e)
{
  if (selectionRectMode() != GP::srmSelect)
    setSelectionRectMode(GP::srmSelect);
  GraphicsPlotExtended::mouseReleaseEvent(e);
}

void AnalysisPlot::SetDefaultAxisRanges(const GPRange &xRange, const GPRange &yRange)
{
  xRangeDefault = xRange;
  yRangeDefault = yRange;
  ResizeAxisToDefault();
}

void AnalysisPlot::ResizeAxisToDefault()
{
  xAxis->setRange(xRangeDefault);
  yAxis->setRange(yRangeDefault);
  queuedReplot();
}

void AnalysisPlot::processRectSelection(const QRect& rect, QMouseEvent* event)
{
  bool selectionStateChanged = false;

  if (mInteractions.testFlag(GP::iSelectItems))
  {
    QList<GPAbstractItem*> selections;
    QRectF rectF(rect.normalized());
    if (GPAxisRect* affectedAxisRect = axisRectAt(rectF.topLeft()))
    {
      // determine plottables that were hit by the rect and thus are candidates for selection:
      for (const auto& item : mItems)
      {
        if (auto plate = dynamic_cast<GPShapeWithLabelItem*>(item))
        {
          if (rectF.intersects(plate->boundingRect()))
            selections.append(item);
        }
      }

      bool additive = event->modifiers().testFlag(mMultiSelectModifier);
      // deselect all other layerables if not additive selection:
      if (!additive)
      {
        // emit deselection except to those plottables who will be selected afterwards:
        for (const auto& item : selectedItems())
        {
          if (!selections.contains(item))
          {
            item->setSelected(false);
            selectionStateChanged = true;
          }
        }
      }

      // go through selections in reverse (largest selection first) and emit select events:
      for (auto& item : selections)
      {
        if (!item->selected())
        {
          item->setSelected(true);
          selectionStateChanged = true;
        }
      }
    }
  }

  if (selectionStateChanged)
  {
    emit selectionChangedByUser();
    replot(rpQueuedReplot);
  }
  else if (mSelectionRect)
  {
    mSelectionRect->layer()->replot();
  }
}


void AnalysisPlot::SetXAxisLabel(const QString& label)
{
  xAxis->setLabel(label);
}

void AnalysisPlot::SetYAxisLabel(const QString& label)
{
  yAxis->setLabel(label);
}

void AnalysisPlot::SetXComponent(double value)
{
  XComponent = value;
}

double AnalysisPlot::GetXComponent() const
{
  return XComponent;
}

void AnalysisPlot::SetYComponent(double value)
{
  YComponent = value;
}

double AnalysisPlot::GetYComponent() const
{
  return YComponent;
}

void AnalysisPlot::SetMenu(QPointer<QMenu> newMenu)
{
  ContextMenu = newMenu;
}

QPointer<QMenu> AnalysisPlot::GetMenu() const
{
  return ContextMenu;
}

void AnalysisPlot::mouseDoubleClickEvent(QMouseEvent *event)
{
  ResizeAxisToDefault();
  GraphicsPlotExtended::mouseDoubleClickEvent(event);
}
