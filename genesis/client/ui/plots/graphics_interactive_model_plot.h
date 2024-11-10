#pragma once

#include <QObject>
#include <graphicsplot/graphicsplot_extended.h>

#include "logic/models/plot_template_model.h"

class GPItemInteractivePixmap;
class GraphicsItemInteractivePolyline;
class GPShapeWithLabelItem;
class TextItem;
class GPItemInteractiveText;
class GPItemInteractiveLine;

using ClickFunc = std::function<void(QMouseEvent* event)>;

class GraphicsInteractiveModelPlot : public GraphicsPlotExtended
{
  Q_OBJECT
public:
  GraphicsInteractiveModelPlot(QWidget* parent = nullptr, unsigned flags = GP::AADefault);
  void setModel(PlotTemplateModel*);
  PlotTemplateModel* model();
  GPAbstractItem* getItemForModelItem(PlotTemplateItem*);
  void setEditMode(bool isEnable);
  void setClickFunc(ClickFunc func);
  void resetClickFunc();
  bool hasClickFunc();

  void setToDefaultAxisX();
  void setToDefaultAxisY();

protected slots:
  void addModelItem(PlotTemplateItem*);
  void removeModelItem(PlotTemplateItem*);
  void moveItemLayers(int sourceStart, int sourceEnd, int destinationRow);

protected:
  virtual void FillContextMenu(QContextMenuEvent* event, QMenu& menu, std::map<QAction*, std::function<void(void)>>& actions) override;

  virtual void mousePressEvent(QMouseEvent* event) override;
  virtual void mouseReleaseEvent(QMouseEvent* event) override;
  void keyPressEvent(QKeyEvent *event) override;

private:
  void connectPixmapItem(PixmapItem* modelItem, GPItemInteractivePixmap* plotItem);
  void connectPolylineItem(PolylineItem* modelItem, GraphicsItemInteractivePolyline* plotItem);
  void connectShapeItem(ShapeItem* modelItem, GPShapeWithLabelItem *plotItem);
  void connectTextItem(TextItem* modelItem, GPItemInteractiveText *plotItem);
  void connectLineItem(LineItem* modelItem, GPItemInteractiveLine *plotItem);

protected:
  PlotTemplateModel* mModel;
  QMap<PlotTemplateItem*, GPAbstractItem*> mItems;
  GPItemRect *mAxisRect;
  Axis mInitialAxisX;
  Axis mInitialAxisY;
  bool mIsEditMode;
  ClickFunc mClickFunc = nullptr;

private:
  GPAbstractItem* mCurrentEditingItem = nullptr;
};

Q_DECLARE_METATYPE(QPointF)
Q_DECLARE_METATYPE(QPixmap)
