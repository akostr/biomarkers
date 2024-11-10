#include "mcr_difference_plot_tiped.h"

#include "ui/plots/gp_items/gp_curve_tracer_item.h"
#include "ui/widgets/curve_label_widget.h"
#include "ui/widgets/mcr_point_info.h"
#include "ui/contexts/mcr_difference_menu_context.h"

namespace
{
  const static QColor SelectionDecoratorColor(187, 227, 250);
  const static QColor CurveColor(0, 32, 51);
}

McrDifferencePlotTiped::McrDifferencePlotTiped(QWidget* parent)
  : GraphicsPlotExtended(parent)
{
  SetupUi();
  ConnectSignals();
}

McrDifferencePlotTiped::~McrDifferencePlotTiped()
{
  Clear();
  delete ContextMenu;
}

void McrDifferencePlotTiped::SetDefaultAxisRanges(GPRange&& xRange, GPRange&& yRange)
{
  xRangeDefault = std::forward<GPRange>(xRange);
  yRangeDefault = std::forward<GPRange>(yRange);
  ResizeAxisToDefault();
}

void McrDifferencePlotTiped::ResizeAxisToDefault()
{
  xAxis->setRange(xRangeDefault);
  yAxis->setRange(yRangeDefault);
  queuedReplot();
}

void McrDifferencePlotTiped::Clear()
{
  for (const auto& item : Tracers)
    disconnect(item.first, &GPCurve::destroyed, this, &McrDifferencePlotTiped::onCurveRemoved);
  Tracers.clear();
  m_legendItems.clear();
  ExcludedCurves.clear();
}

void McrDifferencePlotTiped::CancelExclude()
{
  for (const auto& item : ExcludedCurves)
    ChangeCurveTransparency(item);

  ExcludedCurves.clear();
  emit ExcludedEmpty();
  UpdateActionContextVisibility();
}

void McrDifferencePlotTiped::CancelExclude(const QList<GPCurve*>& curves)
{
  for (const auto& item : curves)
  {
    ChangeCurveTransparency(item);
    ExcludedCurves.removeAll(item);
  }
}

void McrDifferencePlotTiped::ExcludeUnselectedCurves()
{
  if (!ExcludedCurves.isEmpty())
    return;
  const auto sCurves = selectedCurves();
  for (const auto& item : mCurves)
  {
    if (!sCurves.contains(item))
    {
      ExcludedCurves.append(item);
      ChangeCurveTransparency(item, DefaultAlpha);
    }
  }
  UpdateActionContextVisibility();
  emit BuildForNewModel();
}

GPCurve* McrDifferencePlotTiped::addCurve(GPAxis* keyAxis, GPAxis* valueAxis)
{
  auto curvePtr = GraphicsPlotExtended::addCurve(keyAxis, valueAxis);
  const auto decorator = curvePtr->selectionDecorator();
  auto pen = decorator->pen();
  pen.setColor(SelectionDecoratorColor);
  decorator->setPen(pen);
  curvePtr->setSelectionDecorator(decorator);
  createTracer(curvePtr);
  connect(curvePtr, &GPCurve::destroyed, this, &McrDifferencePlotTiped::onCurveRemoved);
  return curvePtr;
}

QList<GPCurve*> McrDifferencePlotTiped::GetExcludedCurves() const
{
  return ExcludedCurves;
}

void McrDifferencePlotTiped::SetupUi()
{
  addLayer(BackLayer.data());
  addLayer(FrontLayer.data());
  addLayer(TickerLayer.data());
  setFlag(GP::AAMagnifier, false);
  setInteraction(GP::Interaction::iSelectItems, true);
  setMultiSelectModifier(Qt::ShiftModifier);
  setInteraction(GP::Interaction::iSelectPlottables, true);

  FloatingMainWindget = new QWidget(this);
  auto l = new QVBoxLayout();
  FloatingMainWindget->setLayout(l);
  FloatingWidget = new McrPointInfo(this);
  FloatingMainWindget->setStyleSheet("QWidget {background-color: rgb(45, 50, 54);border-radius:2px;} .QLabel {color: rgb(201, 202, 204)}");
  FloatingMainWindget->setContentsMargins(9, 9, 9, 9);
  l->setContentsMargins(0, 0, 0, 0);
  l->addWidget(FloatingWidget);
  ContextMenu = new McrDifferentMenuContext(this);
  ContextMenu->CancelExcludeAction->setVisible(false);
}

void McrDifferencePlotTiped::ConnectSignals()
{
  connect(ContextMenu->BuildOriginRestoredAction, &QAction::triggered, this, &McrDifferencePlotTiped::BuildOriginRestoredAction);
  connect(ContextMenu->ExcludeFromModelAction, &QAction::triggered, this, &McrDifferencePlotTiped::ExcludeCurves);
  connect(ContextMenu->CancelExcludeAction, &QAction::triggered, this, &McrDifferencePlotTiped::CancelExcludeCurvesFromAction);
  connect(ContextMenu->BuildForNewModel, &QAction::triggered, this, &McrDifferencePlotTiped::ExcludeUnselectedCurves);
  connect(this, &GraphicsPlotExtended::plottableClick, this, &McrDifferencePlotTiped::PlottableClicked);
}

void McrDifferencePlotTiped::mouseMoveEvent(QMouseEvent* event)
{
  GraphicsPlotExtended::mouseMoveEvent(event);
  const auto pos = event->pos();
  const auto xPos = xAxis->pixelToCoord(pos.x());
  const auto yPos = yAxis->pixelToCoord(pos.y());
  auto min = std::numeric_limits<double>::max();
  GPCurveTracerItem* nearest = nullptr;
  for (const auto& [curve, trace] : Tracers)
  {
    const auto length = std::abs(curve->dataMainValue(xPos) - yPos);
    if (length < min)
    {
      min = length;
      nearest = trace;
    }
    trace->setCurveKey(xPos);
    trace->setVisible(false);
  }
  if (nearest)
  {
    FloatingWidget->SetCaption(xAxis->tickLabel(nearest->position->key()));
    FloatingWidget->SetSampleName(nearest->curve()->name());
    FloatingWidget->SetComponentNumber(QString::number(nearest->position->value()));
    nearest->setVisible(true);
  }

  if ((FloatingMainWindget->pos() - pos).manhattanLength() > MinimumLength)
  {
    FloatingMainWindget->move(pos);
    FloatingMainWindget->adjustSize();
  }

  if (!FloatingMainWindget->isVisible())
    FloatingMainWindget->show();
  queuedReplot();
}

void McrDifferencePlotTiped::leaveEvent(QEvent* event)
{
  GraphicsPlotExtended::leaveEvent(event);
  FloatingMainWindget->hide();
}

void McrDifferencePlotTiped::createTracer(GPCurve* curvePtr)
{
  const auto tracer = new GPCurveTracerItem(this);
  tracer->setSize(TracerSizeByDefault);
  tracer->setPen(QPen(Qt::white, 2));
  tracer->setStyle(GPCurveTracerItem::TracerStyle::tsCircle);
  tracer->setCurve(curvePtr);
  tracer->setBrush(CurveColor);
  tracer->setVisible(false);
  tracer->setLayer(TickerLayer.data());
  Tracers.emplace(curvePtr, tracer);
}

void McrDifferencePlotTiped::onCurveRemoved(QObject* obj)
{
  auto curvePtr = static_cast<GPCurve*>(obj);
  if (curvePtr && !Tracers.empty())
  {
    disconnect(curvePtr, &GPCurve::destroyed, this, &McrDifferencePlotTiped::onCurveRemoved);
    Tracers.erase(curvePtr);
    ExcludedCurves.removeOne(curvePtr);
  }
}

void McrDifferencePlotTiped::contextMenuEvent(QContextMenuEvent* event)
{
  if (InDrawingMeasuringPolygon)
    return;

  if (!(Flags & GP::AAMenu))
    return;

  if (ZoomingAxisMoved)
    return GraphicsPlot::contextMenuEvent(event);

  if (ContextMenu)
  {
    const auto items = selectedCurves();
    ContextMenu->SetTitleCount(items.count());
    ContextMenu->BuildForNewModel->setVisible(ExcludedCurves.isEmpty());
    bool showExcludeAction = true;
    bool showCancelExclude = true;
    for (const auto& item : items)
    {
      if (ExcludedCurves.contains(item))
      {
        showExcludeAction = false;
        break;
      }
    }

    for (const auto& item : items)
    {
      if (!ExcludedCurves.contains(item))
      {
        showCancelExclude = false;
        break;
      }
    }
    ContextMenu->ExcludeFromModelAction->setVisible(showExcludeAction);
    ContextMenu->CancelExcludeAction->setVisible(showCancelExclude);
    ContextMenu->popup(event->globalPos());
  }
  else
    GraphicsPlot::contextMenuEvent(event);
}

void McrDifferencePlotTiped::PlottableClicked(GPAbstractPlottable* plottable, int dataIndex, QMouseEvent* event)
{
  if (const auto curve = qobject_cast<GPCurve*>(plottable))
  {
    for (const auto& item : layer(FrontLayer.data())->children())
      item->setLayer(BackLayer.data());
    curve->setLayer(FrontLayer.data());
  }
}

void McrDifferencePlotTiped::ExcludeCurves()
{
  const auto items = selectedCurves();
  for (const auto& item : items)
  {
    if (!ExcludedCurves.contains(item))
    {
      ExcludedCurves.append(item);
      ChangeCurveTransparency(item, DefaultAlpha);
    }
  }
  UpdateActionContextVisibility();
  emit CurvesExcluded();
}

void McrDifferencePlotTiped::CancelExcludeCurvesFromAction()
{
  const auto items = selectedCurves();
  for (const auto& item : items)
  {
    ChangeCurveTransparency(item);
    ExcludedCurves.removeOne(item);
  }

  UpdateActionContextVisibility();
  if (ExcludedCurves.isEmpty())
    emit ExcludedEmpty();
  else
    emit CurvesExcluded();
}

void McrDifferencePlotTiped::UpdateActionContextVisibility()
{
  ContextMenu->CancelExcludeAction->setVisible(!ExcludedCurves.isEmpty());
  ContextMenu->BuildForNewModel->setVisible(ExcludedCurves.isEmpty());
}

void McrDifferencePlotTiped::ChangeCurveTransparency(GPCurve* curve, float alpha)
{
  if (!curve)
    return;
  auto inactivePen = curve->pen();
  auto color = inactivePen.color();
  color.setAlphaF(alpha);
  inactivePen.setColor(color);
  curve->setPen(inactivePen);
}

void McrDifferencePlotTiped::mouseDoubleClickEvent(QMouseEvent* event)
{
  ResizeAxisToDefault();
  GraphicsPlotExtended::mouseDoubleClickEvent(event);
}
