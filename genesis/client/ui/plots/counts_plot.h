#ifndef COUNTS_PLOT_H
#define COUNTS_PLOT_H

#include "pca_plots.h"
#include "logic/structures/pca_data_structures.h"

class GPShapeWithLabelItem;

using ShapeLegendType = QMap<QString, GPShape::ShapeType>;
using ShapeStyleType = QMap<ParameterType, ShapeLegendType>;
using ColorLegendType = QMap<QString, GPShapeItem::DefaultColor>;
using ColorStyleType = QMap<ParameterType, ColorLegendType>;

using CustomItemStyleMap = QMap<int, CustomItemStyle>;

class SampleInfoPlate;

class CountsPlot : public AnalysisPlot
{
  Q_OBJECT

public:
  explicit CountsPlot(QWidget* parentWidget = nullptr);
  virtual ~CountsPlot() override {};

  virtual void setPoints(const QList<T_Title>& titles,
    const Component& firstComp, const Component& secondComp);
  void clearData() override;
  void clearStyles();
  void handleHighlighting(QPointF viewportPos) override;
  void UpdateItemsColorsBy(ParameterType param, bool updateLegend = true);
  void UpdateItemsShapesBy(ParameterType param, bool updateLegend = true);
  void UpdateItemsSizes();
  void UpdateStyle();
  int  GetPointsCount() override;

  ColorLegendType getColorLegend() const;
  ShapeLegendType getShapeLegend() const;
  ColorStyleType getColorStyle() const;
  ShapeStyleType getShapeStyle() const;
  void setColorStyle(const ColorStyleType& style);
  void setShapeStyle(const ShapeStyleType& style);

  ParameterType GetColorSortParam() const;
  ParameterType GetShapeSortParam() const;

  void SetupStyle(const QList<T_Title>& titles);

  void SetCustomItemStyle(const CustomItemStyleMap& newCustomItemStyle);
  void ResetItemStyleToDefault(const QList<GPShapeWithLabelItem*>& items, bool update = false);
  const CustomItemStyleMap& getCustomItemStyle() const;

  GPShapeWithLabelItem* findItemByFileId(int id);
  const std::map<GPShapeWithLabelItem*, SampleInfoPlate*>& GetExcludedItems() const;

  bool getHasExcludedItems() const;

public slots:
  void ShowNames(bool isShow);
  void ShowPassport(bool isShow);
  void SetupDefaultStyle();
  void customizeSampleRequest(GPShapeWithLabelItem* item);
  void customizeSelectedSamplesRequest(QList<GPAbstractItem*> items);
  void resetCustomStyles();
  void excludeItem(GPShapeWithLabelItem *item);
  void excludeItems(QList<GPShapeWithLabelItem*> items);
  void resetExcludedItems();
  void resetExcludedItems(QList<GPShapeWithLabelItem*> items);
  void updateExcludedState();

signals:
  void legendChanged();
  void showSampleChromatogramm(GPShapeWithLabelItem* item);
  void showSelectedSamplesChromatogramms(QList<GPAbstractItem*> items);
  void showSelectedSamplesInfo(QList<GPAbstractItem*> items);
  void setSelectedSamplesForExcludingFromModel(std::vector<int> items);
  void excludedItemsReset(std::vector<int> items);
  void excludedStateChanged(bool hasExcluded);

protected:
  void FillContextMenu(QContextMenuEvent* event, QMenu& menu, std::map<QAction*, std::function<void(void)>>& actions);
  void mousePressEvent(QMouseEvent* e);
  void mouseReleaseEvent(QMouseEvent* e);

protected:
  std::map<GPShapeWithLabelItem*, SampleInfoPlate*> T_Points;

  ParameterType ColorSortParam;
  ParameterType ShapeSortParam;

  ColorStyleType ColorStyle;
  ShapeStyleType ShapeStyle;
  CustomItemStyleMap CustomItemStyle;

  QPoint leftMouseButtonPressPosition;
  bool labelsHidden, parametersHidden, hasExcludedItems;
  std::map<GPShapeWithLabelItem*, SampleInfoPlate*> ExcludedItems;
};

#endif
