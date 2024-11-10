#include "slope_offset_plot.h"
#include "ui/plots/gp_items/gpshape_with_label_item.h"

using namespace Widgets;

namespace
{
  const std::string_view SampleId = "sample_id";
}

SlopeOffsetPlot::SlopeOffsetPlot(QWidget* parentWidget)
  : AnalysisPlot(parentWidget)
{
  PlotMenu = new SlopeOffsetContext(this);
  ConnectSignals();
}

void SlopeOffsetPlot::contextMenuEvent(QContextMenuEvent* event)
{
  if (InDrawingMeasuringPolygon)
    return;

  if (!(Flags & GP::AAMenu))
    return;

  if (ZoomingAxisMoved)
    return GraphicsPlot::contextMenuEvent(event);

  if (PlotMenu)
  {
    ShowPlotMenu(event);
  }
  else
    GraphicsPlot::contextMenuEvent(event);
}

void SlopeOffsetPlot::excludeItems(const QList<GPAbstractItem*>& items)
{
  for (auto& item : items)
  {
    if (auto shape = qobject_cast<GPShapeWithLabelItem*>(item))
    {
      if (!ExcludedItems.contains(shape))
      {
        ExcludedItems.append(shape);
        shape->setInactive(true);
        updateExcludedState();
      }
    }
  }
}

void SlopeOffsetPlot::updateExcludedState()
{
  bool hasExcludedCheck = !ExcludedItems.empty();
  if (hasExcludedCheck != HasExcludedItems)
  {
    HasExcludedItems = hasExcludedCheck;
    emit excludedStateChanged(HasExcludedItems);
  }
}

void SlopeOffsetPlot::ShowPlotMenu(QContextMenuEvent* event)
{
  auto items = selectedItems();

  if (const auto picked = itemAt(event->pos(), true))
  {
    picked->setSelected(true);
    if(items.isEmpty())
      items.append(picked);
  }

  PlotMenu->TitleAction->setText(tr("PICKED %n SAMPLES", "", items.count()));
  const auto it = std::find_if(items.begin(), items.end(), [&](const GPAbstractItem* item)
    {
      return ExcludedItems.contains(item);
    });
  const auto itemIsNotExcluded = it == items.end();
  PlotMenu->ExcludePointsAction->setVisible(itemIsNotExcluded);
  PlotMenu->CancelExcludePointsAction->setVisible(!itemIsNotExcluded);
  PlotMenu->PickForNewModel->setVisible(ExcludedItems.isEmpty() && itemIsNotExcluded);
  PlotMenu->popup(event->globalPos());
}

void SlopeOffsetPlot::ConnectSignals()
{
  connect(PlotMenu->ExcludePointsAction, &QAction::triggered, this,
    [&](){ emit itemsExcluded(GetSelectedItemIds()); });

  connect(PlotMenu->CancelExcludePointsAction, &QAction::triggered, this,
    [&]() { emit excludedItemsReset(GetSelectedItemIds()); });

  connect(PlotMenu->PickForNewModel, &QAction::triggered, this,
    [&]()
    {
      const auto selectedIds = GetSelectedItemIds();
      std::vector<int> forChange;
      forChange.reserve(selectedIds.size());
      if (ExcludedItems.isEmpty())
      {
        for (const auto& item : mItems)
        {
          if (const auto labelItem = qobject_cast<GPShapeWithLabelItem*>(item))
          {
            const auto currentId = labelItem->property(SampleId.data()).toInt();
            const auto findIt = std::find(selectedIds.begin(), selectedIds.end(), currentId);
            if(findIt == selectedIds.end())
                forChange.push_back(currentId);
          }
        }
        emit itemsExcluded(forChange);
      }
      else
      {
        for (const auto& item : ExcludedItems)
        {
          if (const auto labelItem = qobject_cast<GPShapeWithLabelItem*>(item))
          {
            const auto currentId = labelItem->property(SampleId.data()).toInt();
            const auto findIt = std::find(selectedIds.begin(), selectedIds.end(), currentId);
            if (findIt != selectedIds.end())
              forChange.push_back(currentId);
          }
        }
        emit excludedItemsReset(forChange);
      }
    });
}

std::vector<int> SlopeOffsetPlot::GetSelectedItemIds() const
{
  const auto items = selectedItems();
  std::vector<int> ids;
  ids.reserve(items.size());
  std::set<int> unique;
  std::transform(items.begin(), items.end(), std::inserter(unique, unique.end()),
    [](GPAbstractItem* item) { return item->property(SampleId.data()).toInt(); });
  std::copy(unique.begin(), unique.end(), std::back_inserter(ids));
  return ids;
}

void SlopeOffsetPlot::resetExcludedItems()
{
  for (auto& item : ExcludedItems)
    if (hasItem(item))
      item->setInactive(false);
  ExcludedItems.clear();
  updateExcludedState();
}

void SlopeOffsetPlot::resetExcludedItems(const QList<GPAbstractItem*>& items)
{
  for (auto& item : ExcludedItems)
    if (items.contains(item))
      item->setInactive(false);
  ExcludedItems.erase(std::remove_if(ExcludedItems.begin(), ExcludedItems.end(),
    [items](GPShapeWithLabelItem*& v) { return items.contains(v); }),
    ExcludedItems.end()
  );
  updateExcludedState();
}

std::vector<int> SlopeOffsetPlot::GetExcludedItemsIds() const
{
  std::vector<int> ids;
  ids.reserve(ExcludedItems.size());
  std::transform(ExcludedItems.begin(), ExcludedItems.end(), std::back_inserter(ids),
    [&](GPShapeWithLabelItem* item) { return item->property(SampleId.data()).toInt(); });
  return ids;
}

void SlopeOffsetPlot::ClearExcluded()
{
  resetExcludedItems();
}
