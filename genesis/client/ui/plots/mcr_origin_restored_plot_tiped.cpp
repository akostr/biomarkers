#include "mcr_origin_restored_plot_tiped.h"

#include "ui/contexts/mcr_origin_restored_menu_context.h"
#include "ui/plots/gp_items/gp_curve_tracer_item.h"
#include "ui/plots/gp_items/gp_highlight_curve.h"
#include "genesis_style/style.h"
#include "ui/widgets/curve_label_widget.h"

McrOriginRestoredPlotTiped::McrOriginRestoredPlotTiped(QWidget* parent)
  : GraphicsPlotExtended(parent)
{
  SetupUi();
  ConnectSignals();
}

McrOriginRestoredPlotTiped::~McrOriginRestoredPlotTiped()
{
  Clear();
  delete ContextMenu;
}

void McrOriginRestoredPlotTiped::SetDefaultAxisRanges(const GPRange& xRange, const GPRange& yRange)
{
  xRangeDefault = xRange;
  yRangeDefault = yRange;
  ResizeAxisToDefault();
}

void McrOriginRestoredPlotTiped::ResizeAxisToDefault()
{
  xAxis->setRange(xRangeDefault);
  yAxis->setRange(yRangeDefault);
  queuedReplot();
}

GPCurve* McrOriginRestoredPlotTiped::addCurve(GPAxis* keyAxis, GPAxis* valueAxis)
{
  auto curvePtr = new GPHighlightCurve(keyAxis, valueAxis);
  curvePtr->setSelectable(GP::SelectionType::stMultipleDataRanges);
  createTracer(curvePtr);
  connect(curvePtr, &GPCurve::destroyed, this, &McrOriginRestoredPlotTiped::onCurveRemoved);
  return curvePtr;
}

QVector<int> McrOriginRestoredPlotTiped::GetExcludedPeaks()
{
  const auto curves = selectedCurves();
  QList<GPDataRange> overallRanges;
  for (const auto& curve : curves)
  {
    if (const auto highlightCurve = qobject_cast<GPHighlightCurve*>(curve))
    {
      const auto ranges = highlightCurve->GetExcludedRanges();
      for (const auto& range : ranges)
      {
        const auto it = std::find_if(overallRanges.begin(), overallRanges.end(),
          [&](const GPDataRange& orange) { return orange.intersects(range); });
        if (it != overallRanges.end())
        {
          const auto expandedRange = it->expanded(range);
          it->setBegin(expandedRange.begin());
          it->setEnd(expandedRange.end());
        }
        else
        {
          std::copy(ranges.begin(), ranges.end(),
            std::back_inserter(overallRanges));
        }
      }
    }
  }

  int size = 0;
  for (const auto& range : overallRanges)
    size += range.size();

  QVector<int> excludedPeaks;
  excludedPeaks.reserve(size);
  for (const auto& range : overallRanges)
  {
    const auto currentRange = range.adjusted(1, 1);
    QVector<int> peakIds(currentRange.size());
    std::iota(peakIds.begin(), peakIds.end(), currentRange.begin());
    excludedPeaks.append(peakIds);
  }

  return excludedPeaks;
}

void McrOriginRestoredPlotTiped::ClearExcludedPeaks()
{
  for (const auto& curve : mCurves)
  {
    if (auto hcurve = qobject_cast<GPHighlightCurve*>(curve))
    {
      hcurve->ClearExcluded();
    }
  }
}

void McrOriginRestoredPlotTiped::ExcludeRanges(const QVector<GPDataRange>& excluded)
{
  for (const auto& curve : mCurves)
  {
    if (auto hcurve = qobject_cast<GPHighlightCurve*>(curve))
    {
      for(const auto& range :excluded)
        hcurve->AddRangeForExclude(range);
    }
  }
  emit ExcludePrepeared();
}

void McrOriginRestoredPlotTiped::ResetLegendItems()
{
  Clear();
  QLayoutItem* wItem = nullptr;
  while ((wItem = m_tipCurvesForm->layout()->takeAt(0)) != 0)
    delete wItem;
}

void McrOriginRestoredPlotTiped::SetupUi()
{
  addLayer(BackLayer.data());
  addLayer(FrontLayer.data());
  addLayer(TickerLayer.data());
  setFlag(GP::AAMagnifier, false);
  setInteraction(GP::Interaction::iMultiSelect, true);
  setInteraction(GP::Interaction::iSelectPlottables, true);
  setMultiSelectModifier(Qt::ShiftModifier);
  setSelectionRectMode(GP::srmSelect);
  if (!mSelectionRect)
  {
    setSelectionRect(new GPSelectionRect(this));
  }
  auto rect = selectionRect();
  rect->setPen(QPen(QColor(0, 120, 210), 2));
  rect->setBrush(QBrush(QColor(0, 32, 51, 13)));
  setSizePolicy(QSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum));

  ContextMenu = new McrOriginRestoredMenuContext(this);

  m_tipWidget = new QWidget(this);
  m_tipWidget->setAttribute(Qt::WA_TransparentForMouseEvents);
  m_tipWidget->setContentsMargins(9, 9, 9, 9);
  m_tipWidget->setStyleSheet(Style::ApplySASS(
    "QWidget {background-color: rgb(45, 50, 54); border-radius:2px;}"
    "QLabel{ font: @SecondaryTextFont; color: #FFFFFF; }"
  ));
  auto layout = new QVBoxLayout();
  layout->setContentsMargins(0, 0, 0, 0);
  layout->setSpacing(0);
  m_tipWidget->setLayout(layout);
  m_tipCaptionLabel = new QLabel(tr("Please select curve from difference plot"));
  layout->addWidget(m_tipCaptionLabel);
  m_tipCurvesForm = new QFormLayout();
  m_tipCurvesForm->setVerticalSpacing(0);
  m_tipCurvesForm->setHorizontalSpacing(0);
  m_tipCurvesForm->setSpacing(0);
  m_tipCurvesForm->setFieldGrowthPolicy(QFormLayout::FieldsStayAtSizeHint);
  m_tipCurvesForm->setLabelAlignment(Qt::AlignVCenter | Qt::AlignHCenter);
  m_tipWidget->setVisible(false);
  layout->addLayout(m_tipCurvesForm);
}

void McrOriginRestoredPlotTiped::ConnectSignals()
{
  connect(ContextMenu->ExcludeAction, &QAction::triggered, this, &McrOriginRestoredPlotTiped::ExcludePeaks);
  connect(ContextMenu->CancelExcludeAction, &QAction::triggered, this, &McrOriginRestoredPlotTiped::CancelExclude);
  connect(ContextMenu->BuildForNewModel, &QAction::triggered, this, &McrOriginRestoredPlotTiped::BuildForNewModel);
}

void McrOriginRestoredPlotTiped::contextMenuEvent(QContextMenuEvent* event)
{
  if (InDrawingMeasuringPolygon)
    return;

  if (!(Flags & GP::AAMenu))
    return;

  if (ZoomingAxisMoved)
    return GraphicsPlot::contextMenuEvent(event);

  if (ContextMenu)
  {
    const auto hcurve = qobject_cast<GPHighlightCurve*>(mCurves.front());
    const auto excludedIsEmpty = hcurve->GetExcludedRanges().isEmpty();
    const auto selectedIsExcluded = hcurve->IsSelectedAreExcluded();
    ContextMenu->ExcludeAction->setVisible(!selectedIsExcluded);
    ContextMenu->CancelExcludeAction->setVisible(selectedIsExcluded);
    ContextMenu->BuildForNewModel->setVisible(excludedIsEmpty);
    ContextMenu->popup(event->globalPos());
  }
  else
    GraphicsPlot::contextMenuEvent(event);
}

void McrOriginRestoredPlotTiped::mouseMoveEvent(QMouseEvent* event)
{
  GraphicsPlotExtended::mouseMoveEvent(event);
  const auto pos = event->pos();
  const auto xPos = xAxis->pixelToCoord(pos.x());
  for (const auto& [curve, trace] : Tracers)
  {
    const auto color = curve->pen().color();
    trace->setBrush(color);
    trace->setCurveKey(xPos);
    trace->setVisible(true);
    m_legendItems[curve].first->setColor(curve->pen().color());
    m_legendItems[curve].first->setText(curve->name());
    m_legendItems[curve].second->setText(QString::number(trace->position->value()));
    m_tipCaptionLabel->setText(xAxis->tickLabel(trace->position->key()));
  }

  m_tipWidget->move(pos);

  if (!m_tipWidget->isVisible())
  {
    m_tipWidget->show();
    m_tipWidget->adjustSize();
  }
  queuedReplot();
}

void McrOriginRestoredPlotTiped::mousePressEvent(QMouseEvent* e)
{
  auto mode = e->modifiers().testFlag(Qt::ControlModifier)
    ? GP::srmNone
    : GP::srmSelect;
  setSelectionRectMode(mode);
  GraphicsPlotExtended::mousePressEvent(e);
}

void McrOriginRestoredPlotTiped::mouseReleaseEvent(QMouseEvent* e)
{
  if (selectionRectMode() != GP::srmSelect)
    setSelectionRectMode(GP::srmSelect);
  GraphicsPlotExtended::mouseReleaseEvent(e);
}

void McrOriginRestoredPlotTiped::CancelExclude()
{
  const auto curves = selectedCurves();
  for (const auto& curve : curves)
  {
    if (auto hcurve = qobject_cast<GPHighlightCurve*>(curve))
    {
      hcurve->CancelExcludeSelectedRange();
    }
  }
  emit ExcludePrepeared();
}

void McrOriginRestoredPlotTiped::BuildForNewModel()
{
  const auto curves = selectedCurves();
  for (const auto& curve : curves)
  {
    if (auto hcurve = qobject_cast<GPHighlightCurve*>(curve))
    {
      const auto excludedRanges = hcurve->GetExcludedRanges();
      if (excludedRanges.isEmpty())
      {
        const auto unselected = hcurve->GetUnSelectedSegments();
        for (const auto& range : unselected)
          hcurve->AddRangeForExclude(range);
      }
      else
      {
        const auto selected = hcurve->GetSelectedSegments();
        for (const auto& range : selected)
          hcurve->CancelExcludeSelectedRange(range);
      }
    }
  }
  emit ExcludePrepeared();
}

void McrOriginRestoredPlotTiped::leaveEvent(QEvent* event)
{
  GraphicsPlotExtended::leaveEvent(event);
  for (const auto& [curve, tracer] : Tracers)
    tracer->setVisible(false);

  m_tipWidget->hide();
}

void McrOriginRestoredPlotTiped::createTracer(GPCurve* curvePtr)
{
  const auto tracer = new GPCurveTracerItem(this);
  tracer->setSize(TracerSizeByDefault);
  tracer->setPen(QPen(Qt::white, 2));
  tracer->setStyle(GPCurveTracerItem::TracerStyle::tsCircle);
  tracer->setCurve(curvePtr);
  tracer->setBrush(curvePtr->pen().color());
  tracer->setVisible(true);
  tracer->setLayer(TickerLayer.data());
  m_legendItems[curvePtr].first = new CurveLabelWidget(curvePtr->name(), curvePtr->pen().color());
  m_legendItems[curvePtr].second = new QLabel();
  m_tipCurvesForm->addRow(m_legendItems[curvePtr].first, m_legendItems[curvePtr].second);
  Tracers.emplace(curvePtr, tracer);
}

void McrOriginRestoredPlotTiped::onCurveRemoved(QObject* obj)
{
  auto curvePtr = static_cast<GPCurve*>(obj);
  if (curvePtr && !Tracers.empty())
  {
    Tracers.erase(curvePtr);
    m_legendItems.remove(curvePtr);
  }
}

void McrOriginRestoredPlotTiped::mouseDoubleClickEvent(QMouseEvent* event)
{
  ResizeAxisToDefault();
  GraphicsPlotExtended::mouseDoubleClickEvent(event);
}

void McrOriginRestoredPlotTiped::ExcludePeaks()
{
  const auto curves = selectedCurves();
  for (const auto& curve : curves)
  {
    if (auto hcurve = qobject_cast<GPHighlightCurve*>(curve))
    {
      hcurve->ExcludeSelectedRange();
    }
  }
  emit ExcludePrepeared();
}

void McrOriginRestoredPlotTiped::Clear()
{
  for (const auto& [curvePtr, tracer] : Tracers)
    disconnect(curvePtr, &GPCurve::destroyed, this, &McrOriginRestoredPlotTiped::onCurveRemoved);
  Tracers.clear();
  m_legendItems.clear();
}
