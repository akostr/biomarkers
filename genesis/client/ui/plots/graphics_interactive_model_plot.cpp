#include "graphics_interactive_model_plot.h"
#include "gp_items/gpshape_with_label_item.h"

#include "logic/service/service_locator.h"
#include "logic/service/igraphics_plot_axis_modificator.h"

#include <graphicsplot/graphics_items/graphics_item_interactive_pixmap.h>
#include <graphicsplot/graphics_items/graphics_item_interactive_text.h>
#include <graphicsplot/graphics_items/graphics_item_interactive_line.h>
#include "logic/models/plot_template_items/text_item.h"
#include "logic/models/plot_template_items/line_item.h"
#include <graphicsplot/graphics_items/graphics_item_interactive_polyline.h>

GraphicsInteractiveModelPlot::GraphicsInteractiveModelPlot(QWidget* parent, unsigned flags)
  :GraphicsPlotExtended(parent, flags)
  , mModel(nullptr)
{
  if (auto modifier = Service::ServiceLocator::Instance().Resolve<GraphicsInteractiveModelPlot, Service::IGraphicsPlotAxisModificator>())
    modifier->ApplyModificator(this);

  addLayer("shapeLayer");
  setFlag(GP::AAMagnifier, false);
  if (!mSelectionRect)
  {
    setSelectionRect(new GPSelectionRect(this));
  }

  auto rect = selectionRect();
  rect->setPen(QPen(QColor(0, 120, 210), 2));
  rect->setBrush(QBrush(QColor(0, 32, 51, 13)));
  setSelectionRectMode(GP::srmSelect);
  GPRange range(0,10);
  xAxis->setRange(range);
  yAxis->setRange(range);

  QColor paleInterfaceColor = QColor(0, 32, 51, 153);
  xAxis->setTickLabelColor(paleInterfaceColor);
  xAxis->setBasePen(paleInterfaceColor);
  xAxis->setTickPen(paleInterfaceColor);
  xAxis->setSubTickPen(paleInterfaceColor);
  xAxis->setLabelColor(paleInterfaceColor);
  yAxis->setTickLabelColor(paleInterfaceColor);
  yAxis->setBasePen(paleInterfaceColor);
  yAxis->setTickPen(paleInterfaceColor);
  yAxis->setSubTickPen(paleInterfaceColor);
  yAxis->setLabelColor(paleInterfaceColor);

  {
    mAxisRect = new GPItemRect(this);
    mAxisRect->topLeft->setCoords(axisRect()->topLeft());
    mAxisRect->bottomRight->setCoords(axisRect()->bottomRight());
    QPen pen = QPen(QColor(0x00,0x91,0xFF), 2, Qt::DotLine);
    mAxisRect->setPen(pen);
    mAxisRect->setBrush(Qt::NoBrush);
    mAxisRect->setSelectable(false);
  }
}

void GraphicsInteractiveModelPlot::setModel(PlotTemplateModel* model)
{
  if (!model)
    return;

  if (mModel)
    disconnect (mModel, 0, this, 0);

  for (PlotTemplateItem* modelItem: mItems.keys())
  {
    removeModelItem(modelItem);
  }
  mItems.clear();

  mModel = model;
  connect (mModel, &PlotTemplateModel::itemAdded, this, &GraphicsInteractiveModelPlot::addModelItem);
  connect (mModel, &PlotTemplateModel::itemRemoved, this, &GraphicsInteractiveModelPlot::removeModelItem);

  connect (mModel, &PlotTemplateModel::rowsMoved, this,
          [this](const QModelIndex &sourceParent, int sourceStart, int sourceEnd, const QModelIndex &destinationParent, int destinationRow)
          {
    this->moveItemLayers(sourceStart, sourceEnd, sourceStart < destinationRow ? destinationRow - 1 : destinationRow);
  });
  connect (mModel->getRootItem(), &RootItem::axisXChanged, this, [&] (Axis axis) {
    mInitialAxisX = axis;
    setToDefaultAxisX();
  });
  connect (mModel->getRootItem(), &RootItem::axisYChanged, this, [&] (Axis axis) {
    mInitialAxisY = axis;
    setToDefaultAxisY();
  });
  replot(GraphicsPlot::RefreshPriority::rpQueuedRefresh);
}

PlotTemplateModel* GraphicsInteractiveModelPlot::model()
{
  return mModel;
}

GPAbstractItem* GraphicsInteractiveModelPlot::getItemForModelItem(PlotTemplateItem* modelItem)
{
  if (mItems.contains(modelItem))
    return mItems[modelItem];
}

void GraphicsInteractiveModelPlot::setEditMode(bool isEnable)
{
  mIsEditMode = isEnable;
  setInteraction(GP::iSelectItems, isEnable);
  mAxisRect->setVisible(isEnable);
}

void GraphicsInteractiveModelPlot::setClickFunc(ClickFunc func)
{
  mClickFunc = func;
}

void GraphicsInteractiveModelPlot::resetClickFunc()
{
  mCurrentEditingItem = nullptr;
  mClickFunc = nullptr;
}

bool GraphicsInteractiveModelPlot::hasClickFunc()
{
  return mClickFunc != nullptr;
}

void GraphicsInteractiveModelPlot::addModelItem(PlotTemplateItem* modelItem)
{
  GPAbstractItem* plotItem = nullptr;
  switch (modelItem->type())
  {
  case PlotTemplateItem::PixmapType:
  {
    PixmapItem *pixmapModelItem = qobject_cast<PixmapItem *>(modelItem);
    if (!pixmapModelItem)
      return;
    auto pixmapItem = new GPItemInteractivePixmap(this);
    pixmapItem->setVisible(pixmapModelItem->isVisible());
    pixmapItem->setSelectable(pixmapModelItem->isEnabled());
    pixmapItem->setClipToAxisRect(mIsEditMode ? pixmapModelItem->showOnlyAxisRect() : true);
    pixmapItem->setOpacity(pixmapModelItem->opacity());
    pixmapItem->setRotate(pixmapModelItem->rotate());
    pixmapItem->setTopLeftCoordinate(pixmapModelItem->topLeftCoordinate());
    pixmapItem->setBottomRightCoordinate(pixmapModelItem->bottomRightCoordinate());
    pixmapItem->setTopLeftEditableCoordinate(pixmapModelItem->topLeftEditableCoordinate());
    pixmapItem->setBottomRightEditableCoordinate(pixmapModelItem->bottomRightEditableCoordinate());
    pixmapItem->setPixmap(pixmapModelItem->pixmap());
    pixmapItem->setSelected(pixmapModelItem->selected());

    connectPixmapItem(pixmapModelItem, pixmapItem);
    plotItem = pixmapItem;
    break;
  }
  case PlotTemplateItem::PolylineType:
  {
    auto polyline = new GraphicsItemInteractivePolyline(this);
    PolylineItem *polylineItem = qobject_cast<PolylineItem *>(modelItem);

    polyline->setClipToAxisRect(mIsEditMode ? polylineItem->showOnlyAxisRect() : true);
    polyline->setPoints(polylineItem->points());
    QPen pen(polylineItem->penColor(), polylineItem->penWidth(), polylineItem->penStyle());
    polyline->setPen(pen);
    QBrush brush(Qt::NoBrush);
    if(polylineItem->isLineClosed())
    {
      polyline->closeLine();
      QColor brushColor = polylineItem->fillColor().value<QColor>();
      brushColor.setAlphaF(1.0 - polylineItem->transparency().value<qreal>());
      brush.setColor(brushColor);
      brush.setStyle(Qt::SolidPattern);
      GPLineEnding le((GPLineEnding::EndingStyle)polylineItem->lineEndingType().value<int>());
      polyline->setLineEnding(le);
    }
    polyline->setBrush(brush);

    connectPolylineItem(polylineItem, polyline);
    plotItem = polyline;
    if (polyline->points()->size() == 0 && mIsEditMode)
    {
      polyline->setSelected(true);
    }
    else
    {
      polyline->setSelected(false);
      polyline->setVisible(polylineItem->isVisible());
      polyline->setSelectable(polylineItem->isEnabled());
    }
    // else
    // {
    //   if(auto polylineModelItem = qobject_cast<PolylineItem*>(modelItem))
    //   {
    //     auto polyline = createPolyline(polylineModelItem);
    //     polyline->setSelected(false);
    //     polyline->setVisible(polylineModelItem->isVisible());
    //     polyline->setSelectable(polylineModelItem->isEnabled());
    //     plotItem = polyline;
    //   }
    //   else
    //   {
    //     plotItem = nullptr;
    //   }

    //   // auto pen = polyline->pen();
    //   // //pen.setStyle(polylineItem->penStyle());
    //   // pen.setColor(polylineItem->penColor());
    //   // //pen.setWidthF(polylineItem->penWidth());
    //   // polyline->setPen(pen);

    //   // auto brush = polyline->brush();
    //   // if(!polylineItem->fillColor().isValid())
    //   // {
    //   //   //polyline->resetBrush();
    //   // }
    //   // else
    //   // {
    //   //   auto brush = polyline->brush();
    //   //   auto color = polylineItem->fillColor().value<QColor>();
    //   //   brush.setColor(color);
    //   //   polyline->setBrush(brush);
    //   // }

    //   // if(!polylineItem->transparency().isValid())
    //   // {
    //   //   //polyline->resetBrush();
    //   // }
    //   // else
    //   // {
    //   //   auto brush = polyline->brush();
    //   //   auto color = brush.color();
    //   //   color.setAlphaF(1.0 - polylineItem->transparency().toFloat());
    //   //   brush.setColor(color);
    //   //   polyline->setBrush(brush);
    //   // }

    //   // if(!polylineItem->lineEndingType().isValid())
    //   // {
    //   //   //polyline->resetLineEnding();
    //   // }
    //   // else
    //   // {
    //   //   GPLineEnding newLineEnding(GPLineEnding::EndingStyle(polylineItem->lineEndingType().toInt()), polyline->pen().width());
    //   //   polyline->setLineEnding(newLineEnding);
    //   // }

    // }
    break;
  }
  case PlotTemplateItem::ShapeType:
  {
    ShapeItem *shapeModelItem = qobject_cast<ShapeItem *>(modelItem);
    if (!shapeModelItem)
      return;
    auto shapeItem = new GPShapeWithLabelItem(this, shapeModelItem->coordinate(), shapeModelItem->shapeType(), shapeModelItem->size());
    const auto axisIndex = mModel->getRootItem()->plotType() == TemplatePlot::PlotType::Triplot ? 1 : 0;
    for (const auto& pos : shapeItem->positions())
    {
      const auto leftAxis = axisRect()->axis(GPAxis::atLeft, axisIndex);
      const auto bottomAxis = axisRect()->axis(GPAxis::atBottom, axisIndex);
      pos->setAxes(bottomAxis, leftAxis);
    }
    shapeItem->setLayer("shapeLayer");
    shapeItem->setSelectable(false);
    shapeItem->setMargins(5, 5, 5, 5);
    {
      shapeItem->setColor(shapeModelItem->color());
      auto brush = QBrush(shapeModelItem->color());
      shapeItem->setShapeSelectedBrush(brush);
      shapeItem->setShapePen(Qt::NoPen);
      shapeItem->setShapeSelectedPen(Qt::NoPen);
      shapeItem->setPlatePen(Qt::NoPen);
      shapeItem->setPlateSelectedPen(Qt::NoPen);
      shapeItem->setPlateBrush(Qt::NoBrush);
      shapeItem->setPlateSelectedBrush(QColor(86, 185, 242, 51)); //color from figma
    }
    shapeItem->setColor(shapeModelItem->color());
    shapeItem->SetShape(shapeModelItem->shapeType());
    shapeItem->SetPos(shapeModelItem->coordinate());
    shapeItem->SetSize(shapeModelItem->size());
    shapeItem->setClipToAxisRect(mIsEditMode ? shapeModelItem->showOnlyAxisRect() : true);
    shapeItem->setLabelHidden(true);
    connectShapeItem(shapeModelItem, shapeItem);
    plotItem = shapeItem;
    break;
  }
  case PlotTemplateItem::TextType:
  {
    TextItem *textModelItem = qobject_cast<TextItem *>(modelItem);
    if (!textModelItem)
      return;
    auto textItem = new GPItemInteractiveText(this);
    textItem->setVisible(textModelItem->isVisible());
    textItem->setSelectable(textModelItem->isEnabled());
    textItem->setClipToAxisRect(mIsEditMode ? textModelItem->showOnlyAxisRect() : true);

    textItem->setPositionAsCoordinate(textModelItem->position());
    textItem->setRotate(textModelItem->rotate());
    textItem->setText(textModelItem->text());
    textItem->setFont(textModelItem->font());
    textItem->setColor(textModelItem->textColor());

    connectTextItem(textModelItem, textItem);
    plotItem = textItem;
    break;
  }
  case PlotTemplateItem::LineType:
  {
    LineItem *lineModelItem = qobject_cast<LineItem *>(modelItem);
    if (!lineModelItem)
      return;
    auto lineItem = new GPItemInteractiveLine(this);
    lineItem->setVisible(lineModelItem->isVisible());
    lineItem->setSelectable(lineModelItem->isEnabled());
    lineItem->setClipToAxisRect(mIsEditMode ? lineModelItem->showOnlyAxisRect() : true);

    lineItem->setStartCoord(lineModelItem->start());
    lineItem->setEndCoord(lineModelItem->end());
    QPen pen = lineItem->pen();
    pen.setStyle(lineModelItem->penStyle());
    pen.setColor(lineModelItem->penColor());
    pen.setWidth(lineModelItem->penWidth());
    lineItem->setPen(pen);
    lineItem->setSelectedPen(pen);
    lineItem->setHead(GPLineEnding::EndingStyle(lineModelItem->lineEndingType()));
    plotItem = lineItem;
    connectLineItem(lineModelItem, lineItem);
    break;
  }
  default:
    break;
  }
  if (plotItem)
  {
    mItems.insert(modelItem, plotItem);
    if (modelItem->type() != PlotTemplateItem::ShapeType) // shape must upper then other items
    {
      QString layerName = QString("Layer%1").arg(modelItem->name());
      addLayer(layerName, layer(QLatin1String("grid")), limBelow);
      plotItem->setLayer(layerName);
    }
  }
  replot(GraphicsPlot::RefreshPriority::rpQueuedReplot);
}

void GraphicsInteractiveModelPlot::removeModelItem(PlotTemplateItem *modelItem)
{
  if (!modelItem || !mItems.contains(modelItem))
    return;
  resetClickFunc();
  auto layer = mItems[modelItem]->layer();
  removeItem(mItems[modelItem]);
  removeLayer(layer);
  mItems.remove(modelItem);
  replot(GraphicsPlot::RefreshPriority::rpQueuedReplot);
}

void GraphicsInteractiveModelPlot::moveItemLayers(int sourceStart, int sourceEnd, int destinationRow)
{
  if (destinationRow > mItems.count() || sourceStart >= mItems.count() || sourceEnd >= mItems.count()  )
    return;

  QMap<int, GPAbstractItem*> MapByLayoutIndex;
  for (GPAbstractItem* item: mItems.values())
  {
    MapByLayoutIndex[item->layer()->index()] = item;
  }
  QList <GPAbstractItem*> sortedItems;
  for (GPAbstractItem* item: MapByLayoutIndex.values())
  {
    sortedItems.push_front(item);
  }
  GPLayer *otherLayer = nullptr;
  GraphicsPlot::LayerInsertMode insertMode;
  GPAbstractItem *itemPixmap;
  if (destinationRow == sortedItems.count())
  {
    insertMode = limAbove;
    otherLayer = sortedItems.last()->layer();
    itemPixmap = sortedItems.last();
  }
  else
  {
    insertMode = destinationRow > sourceStart ? limBelow : limAbove;
    otherLayer = sortedItems[destinationRow]->layer();
    itemPixmap = sortedItems[destinationRow];
  }

  for (auto it = sortedItems.begin() + sourceStart; it <= sortedItems.begin() + sourceEnd; it++)
  {
    moveLayer((*it)->layer(), otherLayer, insertMode);
    insertMode = limAbove;
    otherLayer = (*it)->layer();
  }
  replot(GraphicsPlot::RefreshPriority::rpQueuedRefresh);
}

void GraphicsInteractiveModelPlot::FillContextMenu(QContextMenuEvent* event, QMenu& menu, std::map<QAction*, std::function<void(void)>>& actions)
{
  auto items = selectedItems();
  if (!items.isEmpty() &&
      items.count() == 1 &&
      layerableListAt(event->pos(), false).contains(items.first()))
  {
    GPAbstractItem *item = qobject_cast <GPAbstractItem*>(items.first());
    PlotTemplateItem *selectedModelItem = mItems.key(item);
    mModel->FillContextMenu(selectedModelItem, menu, actions);
    GPItemInteractivePixmap *pixmapItem = qobject_cast <GPItemInteractivePixmap*>(item);
    if (pixmapItem)
    {
      menu.addSeparator();
      pixmapItem->FillContextMenu(event, menu, actions);
    }
  }
  else
  {
    GraphicsPlotExtended::FillContextMenu(event, menu, actions);
  }
}

void GraphicsInteractiveModelPlot::connectPixmapItem(PixmapItem* modelItem, GPItemInteractivePixmap* plotItem)
{
  if (modelItem && plotItem)
  {
    connect(modelItem, &PixmapItem::selectedChanged, plotItem, &GPItemInteractivePixmap::setSelected);
    connect(modelItem, &PixmapItem::opacityChanged, plotItem, &GPItemInteractivePixmap::setOpacity);
    connect(modelItem, &PixmapItem::rotateChanged, plotItem, &GPItemInteractivePixmap::setRotate);
    connect(modelItem, &PixmapItem::pixmapChanged, plotItem, &GPItemInteractivePixmap::setPixmap);
    connect(modelItem, &PixmapItem::topLeftCoordinateChanged, plotItem, &GPItemInteractivePixmap::setTopLeftCoordinate);
    connect(modelItem, &PixmapItem::bottomRightCoordinateChanged, plotItem, &GPItemInteractivePixmap::setBottomRightCoordinate);
    connect(modelItem, &PixmapItem::topLeftEditableCoordinateChanged, plotItem, &GPItemInteractivePixmap::setTopLeftEditableCoordinate);
    connect(modelItem, &PixmapItem::bottomRightEditableCoordinateChanged, plotItem, &GPItemInteractivePixmap::setBottomRightEditableCoordinate);
    connect(modelItem, &PixmapItem::visibleChanged, plotItem, &GPItemInteractivePixmap::setVisible);
    connect(modelItem, &PixmapItem::enabledChanged, plotItem, &GPItemInteractivePixmap::setSelectable);
    connect(modelItem, &PixmapItem::showOnlyAxisRectChanged, plotItem, [plotItem](bool clip){
      plotItem->setClipToAxisRect(clip);
      plotItem->parentPlot()->replot(GraphicsPlot::RefreshPriority::rpQueuedRefresh);
    });

    connect(plotItem, &GPItemInteractivePixmap::selectionChanged, modelItem, &PixmapItem::setSelected);
    connect(plotItem, &GPItemInteractivePixmap::opacityChanged, modelItem, &PixmapItem::setOpacity);
    connect(plotItem, &GPItemInteractivePixmap::rotateChanged, modelItem, &PixmapItem::setRotate);
    connect(plotItem, &GPItemInteractivePixmap::topLeftChanged, modelItem, &PixmapItem::setTopLeftCoordinate);
    connect(plotItem, &GPItemInteractivePixmap::bottomRightChanged, modelItem, &PixmapItem::setBottomRightCoordinate);
    connect(plotItem, &GPItemInteractivePixmap::topLeftEditableChanged, modelItem, &PixmapItem::setTopLeftEditableCoordinate);
    connect(plotItem, &GPItemInteractivePixmap::bottomRightEditableChanged, modelItem, &PixmapItem::setBottomRightEditableCoordinate);
  }
}

void GraphicsInteractiveModelPlot::connectPolylineItem(PolylineItem *modelItem, GraphicsItemInteractivePolyline *plotItem)
{
  if(!modelItem || !plotItem)
    return;

  connect(modelItem, &PolylineItem::enabledChanged, plotItem, &GraphicsItemInteractivePolyline::setSelectable);
  connect(modelItem, &PolylineItem::visibleChanged, plotItem, &GraphicsItemInteractivePolyline::setVisible);
  connect(modelItem, &PolylineItem::showOnlyAxisRectChanged, plotItem, [plotItem](bool clip){
    plotItem->setClipToAxisRect(clip);
    plotItem->parentPlot()->replot(GraphicsPlot::RefreshPriority::rpQueuedRefresh);
  });

  connect(modelItem, &PolylineItem::selectedChanged, plotItem, &GraphicsItemInteractivePolyline::setSelected);
  connect(modelItem, &PolylineItem::penStyleChanged, plotItem, [modelItem, plotItem]()
          {
            auto pen = plotItem->pen();
            pen.setStyle(modelItem->penStyle());
            plotItem->setPen(pen);
          });
  connect(modelItem, &PolylineItem::penColorChanged, plotItem, [modelItem, plotItem]()
          {
            auto pen = plotItem->pen();
            pen.setColor(modelItem->penColor());
            plotItem->setPen(pen);
          });
  connect(modelItem, &PolylineItem::penWidthChanged, plotItem, [modelItem, plotItem]()
          {
            auto pen = plotItem->pen();
            pen.setWidthF(modelItem->penWidth());
            plotItem->setPen(pen);
          });
  connect(modelItem, &PolylineItem::fillColorChanged, plotItem, [modelItem, plotItem]()
          {
            if(!modelItem->fillColor().isValid())
            {
              plotItem->resetBrush();
              return;
            }
            auto brush = plotItem->brush();
            brush.setColor(modelItem->fillColor().value<QColor>());
            plotItem->setBrush(brush);
          });
  connect(modelItem, &PolylineItem::transparencyChanged, plotItem, [modelItem, plotItem]()
          {
            if(!modelItem->transparency().isValid())
            {
              plotItem->resetBrush();
              return;
            }
            auto brush = plotItem->brush();
            auto color = brush.color();
            color.setAlphaF(1.0 - modelItem->transparency().toFloat());
            brush.setColor(color);
            plotItem->setBrush(brush);
          });
  connect(modelItem, &PolylineItem::lineEndingChanged, plotItem, [modelItem, plotItem]()
          {
            if(!modelItem->lineEndingType().isValid())
            {
              plotItem->resetLineEnding();
              return;
            }
            GPLineEnding newLineEnding(GPLineEnding::EndingStyle(modelItem->lineEndingType().toInt()));
            plotItem->setLineEnding(newLineEnding);
          });
  connect(modelItem, &PolylineItem::pointsChanged, plotItem, [modelItem, plotItem]()
          {
            plotItem->setPoints(modelItem->points());
          });
  connect(modelItem, &PolylineItem::lineClosingStateChanged, [modelItem, plotItem]()
          {
            plotItem->closeLine(modelItem->isLineClosed());
          });


  connect(plotItem, &GraphicsItemInteractivePolyline::penChanged, modelItem, [modelItem, plotItem]()
          {
            auto pen = plotItem->pen();
            //this setters do nothing if value is equal to previous one
            modelItem->setPenStyle(pen.style());
            modelItem->setPenColor(pen.color());
            modelItem->setPenWidth(pen.widthF());
          });

  connect(plotItem, &GraphicsItemInteractivePolyline::brushChanged, modelItem, [modelItem, plotItem]()
          {
            auto brush = plotItem->brush();
            //this setters do nothing if value is equal to previous one
            modelItem->setFillColor(brush.color());//alpha channel will be cutted here
            modelItem->setTransparency(1.0 - brush.color().alphaF());
          });
  connect(plotItem, &GraphicsItemInteractivePolyline::lineEndingChanged, modelItem, [modelItem, plotItem]()
          {
            auto lineEnding = plotItem->lineEnding();

            //FIXME:
            //when i want reset modelItem line ending property to QVariant() for
            //removing it's editor from the side bar, at this place it rewrites
            //empty QVariant to esNone, wich prevents editor removing.
            //find the better way please, to avoid this problem.
            if(!modelItem->lineEndingType().isValid() && lineEnding.style() == GPLineEnding::esNone)
              return;


            modelItem->setLineEndingType((PolylineItem::LineEndingType)(int)(lineEnding.style()));//one to one
          });
  connect(plotItem, &GraphicsItemInteractivePolyline::pointsChanged, modelItem, [modelItem, plotItem]()
          {
            modelItem->setPoints(*plotItem->points());
          });

  connect(plotItem, &GraphicsItemInteractivePolyline::lineClosedStateChanged, modelItem, [modelItem, plotItem]()
          {
            modelItem->setLineClosed(plotItem->isClosed());
          });
}

void GraphicsInteractiveModelPlot::connectShapeItem(ShapeItem* modelItem, GPShapeWithLabelItem* plotItem)
{
  if (modelItem && plotItem)
  {
    connect(modelItem, &ShapeItem::coordinateChanged, plotItem, &GPShapeWithLabelItem::SetPos);
    connect(modelItem, &ShapeItem::sizeChanged, plotItem, &GPShapeWithLabelItem::SetSize);
    connect(modelItem, &ShapeItem::colorChanged, plotItem, [plotItem](QColor color) {
      plotItem->setColor(color);
      auto brush = QBrush(color);
      plotItem->setShapeSelectedBrush(brush);
    });
    connect(modelItem, &ShapeItem::shapeTypeChanged, plotItem, qOverload<GPShape::ShapeType>(&GPShapeWithLabelItem::SetShape));
    connect(modelItem, &ShapeItem::showOnlyAxisRectChanged, plotItem, [plotItem](bool clip){
      plotItem->setClipToAxisRect(clip);
      plotItem->parentPlot()->replot(GraphicsPlot::RefreshPriority::rpQueuedRefresh);
    });
  }
}

void GraphicsInteractiveModelPlot::connectTextItem(TextItem* modelItem, GPItemInteractiveText *plotItem)
{
  connect(modelItem, &TextItem::selectedChanged, plotItem, &GPItemInteractiveText::setSelected);
  connect(modelItem, &TextItem::enabledChanged, plotItem, &GPItemInteractiveText::setSelectable);
  connect(modelItem, &TextItem::visibleChanged, plotItem, &GPItemInteractiveText::setVisible);
  connect(modelItem, &TextItem::showOnlyAxisRectChanged, plotItem, [plotItem](bool clip){
    plotItem->setClipToAxisRect(clip);
    plotItem->parentPlot()->replot(GraphicsPlot::RefreshPriority::rpQueuedRefresh);
  });
  connect(modelItem, &TextItem::positionChanged, plotItem, &GPItemInteractiveText::setPositionAsCoordinate, Qt::DirectConnection);
  connect(modelItem, &TextItem::textChanged, plotItem, [plotItem](QString text){
    plotItem->setText(text);
    plotItem->parentPlot()->replot(GraphicsPlot::RefreshPriority::rpQueuedRefresh);
  });
  connect(modelItem, &TextItem::fontChanged, plotItem, [plotItem](QFont font){
    plotItem->setFont(font);
    plotItem->setSelectedFont(font);
    plotItem->parentPlot()->replot(GraphicsPlot::RefreshPriority::rpQueuedRefresh);
  });
  connect(modelItem, &TextItem::textColorChanged, plotItem,  [plotItem](QColor color){
    plotItem->setColor(color);
    plotItem->parentPlot()->replot(GraphicsPlot::RefreshPriority::rpQueuedRefresh);
  });

  connect(plotItem, &GPItemInteractiveText::positionChanged, modelItem, &TextItem::setPosition, Qt::DirectConnection);
  connect(plotItem, &GPItemInteractiveText::rotateChanged, modelItem, &TextItem::setRotate);
}

void GraphicsInteractiveModelPlot::connectLineItem(LineItem* modelItem, GPItemInteractiveLine *plotItem)
{
  connect(modelItem, &LineItem::selectedChanged, plotItem, &GPItemInteractiveLine::setSelected);
  connect(modelItem, &LineItem::enabledChanged, plotItem, &GPItemInteractiveLine::setSelectable);
  connect(modelItem, &LineItem::visibleChanged, plotItem, &GPItemInteractiveLine::setVisible);
  connect(modelItem, &LineItem::showOnlyAxisRectChanged, plotItem, [plotItem](bool clip){
    plotItem->setClipToAxisRect(clip);
    plotItem->parentPlot()->replot(GraphicsPlot::RefreshPriority::rpQueuedRefresh);
  });
  connect(modelItem, &LineItem::startChanged, plotItem, &GPItemInteractiveLine::setStartCoord);
  connect(modelItem, &LineItem::endChanged, plotItem, &GPItemInteractiveLine::setEndCoord);
  connect(modelItem, &LineItem::penStyleChanged, plotItem, [plotItem](Qt::PenStyle style){
    QPen pen = plotItem->pen();
    pen.setStyle(style);
    plotItem->setPen(pen);
    plotItem->setSelectedPen(pen);
    plotItem->parentPlot()->replot(GraphicsPlot::RefreshPriority::rpQueuedRefresh);
  });
  connect(modelItem, &LineItem::penColorChanged, plotItem, [plotItem](QColor color){
    QPen pen = plotItem->pen();
    pen.setColor(color);
    plotItem->setPen(pen);
    plotItem->setSelectedPen(pen);
    plotItem->parentPlot()->replot(GraphicsPlot::RefreshPriority::rpQueuedRefresh);
  });
  connect(modelItem, &LineItem::penWidthChanged, plotItem, [plotItem](int width){
    QPen pen = plotItem->pen();
    pen.setWidth(width);
    plotItem->setPen(pen);
    plotItem->setSelectedPen(pen);
    plotItem->parentPlot()->replot(GraphicsPlot::RefreshPriority::rpQueuedRefresh);
  });
  connect(modelItem, &LineItem::lineEndingTypeChanged, plotItem, [plotItem](int endingType){
    plotItem->setHead(GPLineEnding::EndingStyle(endingType));
    plotItem->parentPlot()->replot(GraphicsPlot::RefreshPriority::rpQueuedRefresh);
  });

  connect(plotItem, &GPItemInteractiveLine::startCoordinateChanged, modelItem, &LineItem::setStart);
  connect(plotItem, &GPItemInteractiveLine::endCoordinateChanged, modelItem, &LineItem::setEnd);
}

void GraphicsInteractiveModelPlot::mousePressEvent(QMouseEvent* e)
{
  if(e->button() == Qt::LeftButton)
  {
    if(mClickFunc)
    {
      mClickFunc(e);
      e->accept();
      return;
    }
  }
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

void GraphicsInteractiveModelPlot::mouseReleaseEvent(QMouseEvent* e)
{
  if (selectionRectMode() != GP::srmSelect)
    setSelectionRectMode(GP::srmSelect);
  GraphicsPlotExtended::mouseReleaseEvent(e);
}

void GraphicsInteractiveModelPlot::keyPressEvent(QKeyEvent *event)
{
  if(mClickFunc)
  {
    if(event->key() == Qt::Key_Escape
        || event->key() == Qt::Key_Enter)
    {
      resetClickFunc();
    }
  }

  GraphicsPlotExtended::keyPressEvent(event);
}

void GraphicsInteractiveModelPlot::setToDefaultAxisX()
{
  if (mModel)
  {
    xAxis->setRange(mInitialAxisX.from, mInitialAxisX.to);
    xAxis->setLabel(mInitialAxisX.title);
    xAxis->setScaleType(mInitialAxisX.isLinear ? GPAxis::stLinear : GPAxis::stLogarithmic);
    QPointF topLeft(mInitialAxisX.to, mInitialAxisY.from);
    QPointF bottomRight(mInitialAxisX.from, mInitialAxisY.to);
    mAxisRect->topLeft->setCoords(topLeft);
    mAxisRect->bottomRight->setCoords(bottomRight);
    replot(GraphicsPlot::RefreshPriority::rpQueuedRefresh);
  }
}

void GraphicsInteractiveModelPlot::setToDefaultAxisY()
{
  if (mModel)
  {
    yAxis->setRange(mInitialAxisY.from, mInitialAxisY.to);
    yAxis->setLabel(mInitialAxisY.title);
    yAxis->setScaleType(mInitialAxisY.isLinear ? GPAxis::stLinear : GPAxis::stLogarithmic);
    QPointF topLeft(mInitialAxisX.to, mInitialAxisY.from);
    QPointF bottomRight(mInitialAxisX.from, mInitialAxisY.to);
    mAxisRect->topLeft->setCoords(topLeft);
    mAxisRect->bottomRight->setCoords(bottomRight);
    replot(GraphicsPlot::RefreshPriority::rpQueuedRefresh);
  }
}
