#include "pls_coefficient_plot.h"

#include "ui/contexts/slope_offset_context.h"
#include "ui/plots/gp_items/gp_highlight_curve.h"

using namespace Widgets;

PlsCoefficientPlot::PlsCoefficientPlot(QWidget* parent)
  : GraphicsPlotExtended(parent)
{
  SetupUi();
  ConnectSignals();
}

PlsCoefficientPlot::~PlsCoefficientPlot()
{
  delete ContextMenu;
}

void PlsCoefficientPlot::SetDefaultAxisRanges(const GPRange& xRange, const GPRange& yRange)
{
  xRangeDefault = xRange;
  yRangeDefault = yRange;
  ResizeAxisToDefault();
}

void PlsCoefficientPlot::ResizeAxisToDefault()
{
  xAxis->setRange(xRangeDefault);
  yAxis->setRange(yRangeDefault);
  queuedReplot();
}

GPCurve* PlsCoefficientPlot::addCurve(GPAxis* keyAxis, GPAxis* valueAxis)
{
  auto curvePtr = new GPHighlightCurve(keyAxis, valueAxis);
  curvePtr->SetSelectionColor(QColor(86, 185, 242, 51));
  curvePtr->setSelectable(GP::SelectionType::stMultipleDataRanges);
  return curvePtr;
}

void PlsCoefficientPlot::AddExcludedPeaks(const std::vector<int>& peaks)
{
  std::vector<int> sortedPeaks = peaks;
  std::vector<int> differences;
  differences.reserve(peaks.size());
  std::sort(sortedPeaks.begin(), sortedPeaks.end());
  std::adjacent_difference(sortedPeaks.begin(), sortedPeaks.end(),
    std::back_inserter(differences));
  const auto count = std::count_if(differences.begin(), differences.end(), [](int diff) { return diff > 1; });
  std::vector<GPDataRange> ranges;
  ranges.reserve(count + 1);
  for (int i = 0; i < differences.size(); i++)
  {
    int begin = sortedPeaks[i] - 1;
    while (++i < differences.size() && differences[i] == 1);
    ranges.emplace_back(begin, sortedPeaks[--i]);
  }

  for (const auto& curve : mCurves)
  {
    if (const auto highlightCurve = qobject_cast<GPHighlightCurve*>(curve))
    {
      for (const auto range : ranges)
      {
        highlightCurve->AddRangeForExclude(range);
      }
    }
  }
}

void PlsCoefficientPlot::CancelExcludePeaks(const std::vector<int>& peaks)
{
  std::vector<int> sortedPeaks = peaks;
  std::vector<int> differences;
  differences.reserve(peaks.size());
  std::sort(sortedPeaks.begin(), sortedPeaks.end());
  std::adjacent_difference(sortedPeaks.begin(), sortedPeaks.end(),
    std::back_inserter(differences));
  const auto count = std::count_if(differences.begin(), differences.end(), [](int diff) { return diff > 1; });
  std::vector<GPDataRange> ranges;
  ranges.reserve(count + 1);
  for (int i = 0; i < differences.size(); i++)
  {
    int begin = sortedPeaks[i] - 1;
    while (++i < differences.size() && differences[i] == 1);
    ranges.emplace_back(begin, sortedPeaks[--i]);
  }

  for (const auto& curve : mCurves)
  {
    if (const auto highlightCurve = qobject_cast<GPHighlightCurve*>(curve))
    {
      for (const auto range : ranges)
      {
        highlightCurve->CancelExcludeSelectedRange(range);
      }
    }
  }
}

bool PlsCoefficientPlot::ExcludedExists()
{
  if (mCurves.isEmpty())
    return false;
  const auto curve = qobject_cast<GPHighlightCurve*>(mCurves.front());
  const auto ranges = curve->GetExcludedRanges();
  return !ranges.isEmpty();
}

QVector<int> PlsCoefficientPlot::GetSelectedExcludedPeaks()
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
    std::iota(peakIds.begin(), peakIds.end(), currentRange.begin() - 1);
    excludedPeaks.append(peakIds);
  }

  return excludedPeaks;
}

void PlsCoefficientPlot::ClearExcludedPeaks()
{
  for (const auto& curve : mCurves)
  {
    if (auto hcurve = qobject_cast<GPHighlightCurve*>(curve))
    {
      hcurve->ClearExcluded();
    }
  }
  ContextMenu->CancelExcludePointsAction->setVisible(false);
}

void PlsCoefficientPlot::CancelSelectedExcludePeaks()
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

void PlsCoefficientPlot::PickForNewModel()
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
        for(const auto& range : unselected)
          hcurve->AddRangeForExclude(range);
      }
    }
  }
  emit ExcludePrepeared();
}

void PlsCoefficientPlot::SetupUi()
{
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
  ContextMenu = new SlopeOffsetContext(this);
  ContextMenu->CancelExcludePointsAction->setVisible(false);

  yAxis->setLabel(tr("Coefficient"));
  xAxis->setLabel(tr("Variables"));
}

void PlsCoefficientPlot::ConnectSignals()
{
  connect(ContextMenu->ExcludePointsAction, &QAction::triggered, this, &PlsCoefficientPlot::ExcludeSelectedPoints);
  connect(ContextMenu->CancelExcludePointsAction, &QAction::triggered, this, &PlsCoefficientPlot::CancelSelectedExcludePeaks);
  connect(ContextMenu->PickForNewModel, &QAction::triggered, this, &PlsCoefficientPlot::PickForNewModel);
  connect(this, &PlsCoefficientPlot::ExcludePrepeared, this, &PlsCoefficientPlot::HideBuildForNewModel);
}

void PlsCoefficientPlot::contextMenuEvent(QContextMenuEvent* event)
{
  if (InDrawingMeasuringPolygon)
    return;

  if (!(Flags & GP::AAMenu))
    return;

  if (ZoomingAxisMoved)
    return GraphicsPlot::contextMenuEvent(event);

  if (ContextMenu)
  {
    const auto selected = selectedPlottables();
    int totalItems = 0;
    for (const auto& item : selected)
    {
      const auto selection = item->selection();
      const auto ranges = selection.dataRanges();
      for (const auto& range : ranges)
      {
        totalItems += range.size();
      }
    }
    ContextMenu->SetTitleActionText(tr("PICKED %n PEAKS", "", totalItems));
    if (!mCurves.isEmpty())
    {
      if (const auto hcurve = qobject_cast<GPHighlightCurve*>(mCurves.front()))
      {
        const auto excludedIsEmpty = hcurve->GetExcludedRanges().isEmpty();
        const auto selectedIsExcluded = hcurve->IsSelectedAreExcluded();
        ContextMenu->CancelExcludePointsAction->setVisible(!excludedIsEmpty && selectedIsExcluded);
        ContextMenu->ExcludePointsAction->setVisible(!selectedIsExcluded);
        ContextMenu->PickForNewModel->setVisible(excludedIsEmpty);
      }
    }
    ContextMenu->popup(event->globalPos());
  }
  else
    GraphicsPlot::contextMenuEvent(event);
}

void PlsCoefficientPlot::mousePressEvent(QMouseEvent* e)
{
  auto mode = e->modifiers().testFlag(Qt::ControlModifier)
    ? GP::srmNone
    : GP::srmSelect;
  setSelectionRectMode(mode);
  GraphicsPlotExtended::mousePressEvent(e);
}

void PlsCoefficientPlot::mouseReleaseEvent(QMouseEvent* e)
{
  if (selectionRectMode() != GP::srmSelect)
    setSelectionRectMode(GP::srmSelect);
  GraphicsPlotExtended::mouseReleaseEvent(e);
}

void PlsCoefficientPlot::ExcludeSelectedPoints()
{
  for (const auto& curve : mCurves)
  {
    if (const auto highlightCurve = qobject_cast<GPHighlightCurve*>(curve))
    {
      highlightCurve->ExcludeSelectedRange();
    }
  }
  emit ExcludePrepeared();
}

void PlsCoefficientPlot::HideBuildForNewModel()
{
  bool visible = true;
  for (const auto& curve : mCurves)
  {
    if (const auto highlightCurve = qobject_cast<GPHighlightCurve*>(curve))
    {
      const auto range = highlightCurve->GetExcludedRanges();
      visible &= range.isEmpty();
    }
  }
  ContextMenu->PickForNewModel->setVisible(visible);
}

