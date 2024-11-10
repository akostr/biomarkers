#ifndef LOAD_PLOT_H
#define LOAD_PLOT_H

#include "pca_plots.h"

#include "logic/structures/pca_data_structures.h"

class GPShapeWithLabelItem;

class LoadPlot : public AnalysisPlot
{
  Q_OBJECT

public:
  explicit LoadPlot(QWidget* parentWidget = nullptr);
  virtual ~LoadPlot() override {};

  void setPoints(const QStringList& titles, const QMap<QString, int> &P_Ids,
                 const Component& firstComp,
                 const Component& secondComp,
                 const QList<int>& excluded);
  void clearData() override;
  const QList<GPShapeWithLabelItem *>& GetExcludedItems();
  int  GetPointsCount() override;
  void ExcludePeaks(const QStringList& peaks);
  void CancelExclude();
  bool getHasExcludedItems() const;

signals:
  void excludedStateChanged(bool hasExcluded);
  void AllItemsExcluded();
  void ResetExcludedPeaks(QStringList names);

public slots:
  void showNames(bool isShown);
  void resetExcludedItems(bool update= true);
  void resetExcludedItems(QList<GPAbstractItem*> items);
  void resetExcludedItem(GPAbstractItem *item);

protected:
  void FillContextMenu(QContextMenuEvent* event, QMenu& menu, std::map<QAction*, std::function<void(void)>>& actions);
  void excludeItem(GPShapeWithLabelItem *item);
  void excludeItems(QList<GPAbstractItem*> items);
  void updateExcludedState();

private:
  QList<GPShapeWithLabelItem*> P_Points;
  QList<GPShapeWithLabelItem*> ExcludedItems;
  bool HasExcludedItems;
};

#endif
