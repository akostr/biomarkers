#pragma once
#ifndef SLOPE_OFFSET_PLOT_H
#define SLOPE_OFFSET_PLOT_H

#include "pca_plots.h"

#include "ui/contexts/slope_offset_context.h"

class GPShapeWithLabelItem;

class SlopeOffsetPlot : public AnalysisPlot
{
  Q_OBJECT

public:
  explicit SlopeOffsetPlot(QWidget* parentWidget = nullptr);
  ~SlopeOffsetPlot() override = default;

  void excludeItems(const QList<GPAbstractItem*>& items);
  void resetExcludedItems(const QList<GPAbstractItem*>& items);
  std::vector<int> GetExcludedItemsIds() const;
  void ClearExcluded();

signals:
  void excludedStateChanged(bool hasExcluded);
  void itemsExcluded(std::vector<int> items);
  void excludedItemsReset(std::vector<int> items);

protected:
  void contextMenuEvent(QContextMenuEvent* event) override;

private:
  QList<GPShapeWithLabelItem*> ExcludedItems;
  bool HasExcludedItems = false;
  Widgets::SlopeOffsetContext* PlotMenu = nullptr;

  void updateExcludedState();
  void ShowPlotMenu(QContextMenuEvent* event);
  void ConnectSignals();
  std::vector<int> GetSelectedItemIds() const;

public slots:
  void resetExcludedItems();
};
#endif
