#include "plot_template_model.h"
#include <QJsonArray>
#include <QJsonDocument>
#include <QFile>
#include <QMetaProperty>
#include <QBuffer>
#include "ui/plots/gp_items/gpshape_item.h"
#include "plot_template_items/text_item.h"
#include "plot_template_items/line_item.h"

namespace {
const int defaultHeight = 1080;
const int defaultWidth = 1920;
}

RootItem::RootItem()
  : m_size(defaultWidth, defaultHeight)
{

}

QList<PlotTemplateItem *> &RootItem::items()
{
  return m_items;
}

QJsonObject RootItem::saveToJson()
{
  QJsonObject jsonObj;
  QJsonArray jsonItems;
  for (const auto &item: m_items)
  {
    jsonItems.append(item->saveToJson());
  }
  jsonObj.insert("items", jsonItems);

  jsonObj.insert("axis_x_title", m_xAxis.title);
  jsonObj.insert("axis_x_from", m_xAxis.from);
  jsonObj.insert("axis_x_to", m_xAxis.to);
  jsonObj.insert("axis_x_is_linear", m_xAxis.isLinear);
  jsonObj.insert("template_type_id", static_cast<int>(m_plotType));

  jsonObj.insert("axis_y_title", m_yAxis.title);
  jsonObj.insert("axis_y_from", m_yAxis.from);
  jsonObj.insert("axis_y_to", m_yAxis.to);
  jsonObj.insert("axis_y_is_linear", m_yAxis.isLinear);
  jsonObj.insert("width", m_size.width());
  jsonObj.insert("height", m_size.height());
  return jsonObj;
}

void RootItem::loadFromJson(const QJsonObject& jsonObj)
{
  if (!jsonObj.isEmpty())
  {
    QJsonArray jsonItems = jsonObj["items"].toArray();
    for (int i = jsonItems.count() - 1; i >= 0; i--)
    {
      const auto &jsonValue = jsonItems[i];
      const QJsonObject object = jsonValue.toObject();
      int type = object["type"].toInt();
      auto item = addItem(type);
      item->loadFromJson(object);
    }
    m_xAxis.title = jsonObj["axis_x_title"].toString();
    m_xAxis.from = jsonObj["axis_x_from"].toDouble();
    m_xAxis.to = jsonObj["axis_x_to"].toDouble();
    m_xAxis.isLinear = jsonObj["axis_x_is_linear"].toBool();
    m_plotType = static_cast<TemplatePlot::PlotType>(jsonObj["template_type_id"].toInt());
    m_yAxis.title = jsonObj["axis_y_title"].toString();
    m_yAxis.from = jsonObj["axis_y_from"].toDouble();
    m_yAxis.to = jsonObj["axis_y_to"].toDouble();
    m_yAxis.isLinear = jsonObj["axis_y_is_linear"].toBool();
    m_size.setHeight(jsonObj["height"].toInt(defaultHeight));
    m_size.setWidth(jsonObj["width"].toInt(defaultWidth));
    emit loadedFromJson(true);
  }
  else
  {
    m_xAxis.title = tr("x");
    m_xAxis.from = 0;
    m_xAxis.to = 10;
    m_xAxis.isLinear = true;

    m_yAxis.title = tr("y");
    m_yAxis.from = 0;
    m_yAxis.to = 10;
    m_yAxis.isLinear = true;
    m_size.setHeight(defaultHeight);
    m_size.setWidth(defaultWidth);
    emit loadedFromJson(false);
  }
  emit axisXChanged(m_xAxis);
  emit axisYChanged(m_yAxis);
  emit sizeChanged(m_size);
}

PlotTemplateItem *RootItem::addItem(int type)
{
  int number = 1;
  number += std::count_if (m_items.begin(), m_items.end(), [type](PlotTemplateItem* item) {
    return item->type() == type;
  });

  PlotTemplateItem *item = nullptr;
  switch(type)
  {
  case PlotTemplateItem::PixmapType:
  {
    item = new PixmapItem(this);
    item->setName(QString("Image %1").arg(number));
    break;
  }
  case PlotTemplateItem::PolylineType:
  {
    item = new PolylineItem(this);
    item->setName(QString("Polyline %1").arg(number));
    break;
  }
  case PlotTemplateItem::ShapeType:
  {
    ShapeItem *shape = new ShapeItem(this);
    shape->setName(QString("Shape %1").arg(number));
    shape->setColor(GPShapeItem::GetCycledColor(number));
    shape->setShape(GPShapeItem::GetCycledShape(number));
    item = shape;
    break;
  }
  case PlotTemplateItem::TextType:
  {
    TextItem *text = new TextItem(this);
    text->setName(QString("Text %1").arg(number));
    text->setText(text->name());
    text->setTextColor(GPShapeItem::GetCycledColor(0));
    text->setFont(QFont());
    item = text;
    break;
  }
  case PlotTemplateItem::LineType:
  {
    LineItem *line = new LineItem(this);
    line->setName(QString("Line %1").arg(number));
    item = line;
    break;
  }
  }

  if (item)
    m_items.push_front(item);
  return item;
}

void RootItem::setAxisX(Axis axis)
{
  if (m_yAxis != axis)
  {
    m_xAxis = axis;
    emit axisXChanged(m_xAxis);
  }
}

void RootItem::setAxisY(Axis axis)
{
  if (m_yAxis != axis)
  {
    m_yAxis = axis;
    emit axisYChanged(m_yAxis);
  }
}

void RootItem::setSize(QSize size)
{
  if (m_size != size)
  {
    m_size = size;
    emit sizeChanged(m_size);
  }
}

void RootItem::setPlotType(TemplatePlot::PlotType type)
{
  if (m_plotType != type)
  {
    m_plotType = type;
    emit typeChanged(type);
  }
}

PlotTemplateItem::PlotTemplateItem(RootItem *parent)
  : QObject(parent)
  , m_root(parent)
  , m_selected(false)
  , m_isEnabled(true)
  , m_isVisible(true)
  , m_rotate(0)
  , m_showOnlyAxisRect(true)
  , m_uuid(QUuid::createUuid())
{
}

QJsonObject PlotTemplateItem::saveToJson()
{
  QJsonObject json;
  json["type"] = type();
  json["name"] = name();
  json["enabled"] = isEnabled();
  json["visible"] = isVisible();
  if(property(PROP::opacity).isValid())
    json["opacity"] = property(PROP::opacity).toDouble();
  json["rotate"] = rotate();
  json["uuid"] = uuid().toString();
  json["show_only_axis_rect"] = m_showOnlyAxisRect;
  return json;
}

void PlotTemplateItem::loadFromJson(const QJsonObject &json)
{
  setName(json["name"].toString());
  setEnabled(json["enabled"].toBool());
  setVisible(json["visible"].toBool());
  if(property(PROP::opacity).isValid())
    setProperty(PROP::opacity, json["opacity"].toDouble());
  // setOpacity(json["opacity"].toDouble());
  setRotate(json["rotate"].toDouble());
  setShowOnlyAxisRect(json["show_only_axis_rect"].toBool());
  if (json.contains("uuid"))
    m_uuid = QUuid::fromString(json["uuid"].toString());
}

void PlotTemplateItem::setName(QString name)
{
  m_name = name;
}

void PlotTemplateItem::setSelected(bool selected)
{
  if (m_selected != selected )
  {
    m_selected = selected;
    selectedChanged(m_selected);
  }
}

void PlotTemplateItem::setEnabled(bool isEnabled)
{
  if (m_isEnabled != isEnabled)
  {
    m_isEnabled = isEnabled;
    enabledChanged(m_isEnabled);
  }
}

void PlotTemplateItem::setVisible(bool isVisible)
{
  if (m_isVisible != isVisible)
  {
    m_isVisible = isVisible;
    visibleChanged(m_isVisible);
  }
}

void PlotTemplateItem::setRotate(double rotate)
{
  if (std::fabs(m_rotate - rotate) > std::numeric_limits<double>::epsilon())
  {
    m_rotate = rotate;
    rotateChanged(m_rotate);
  }
}

void PlotTemplateItem::setShowOnlyAxisRect(bool showOnlyAxisRect)
{
  if (m_showOnlyAxisRect != showOnlyAxisRect)
  {
    m_showOnlyAxisRect = showOnlyAxisRect;
    emit showOnlyAxisRectChanged(m_showOnlyAxisRect);
  }
}

void PlotTemplateItem::setUuid(QUuid uuid)
{
  m_uuid = uuid;
}

PixmapItem::PixmapItem(RootItem *rootItem)
  : PlotTemplateItem (rootItem)
  , m_opacity(1.0)
{
  auto axisX = rootItem->axisX();
  auto axisY = rootItem->axisY();
  double halfX = 0;
  double halfY = 0;

  if (axisX.isLinear)
  {
    halfX = (axisX.from + axisX.to) / 2.;
  }
  else
  {
    halfX = sqrt(axisX.from * axisX.to);
  }
  if (axisY.isLinear)
  {
    halfY = (axisY.from + axisY.to) / 2.;
  }
  else
  {
    halfY = sqrt(axisY.from * axisY.to);
  }

  m_bottomRightCoordinate = QPointF(halfX, axisY.from);
  m_topLeftCoordinate = QPointF(axisX.from, halfY);
  m_topLeftEditableCoordinate = m_topLeftCoordinate;
  m_bottomRightEditableCoordinate = m_bottomRightCoordinate;

}

QJsonObject PixmapItem::saveToJson()
{
  QJsonObject json;
  json = PlotTemplateItem::saveToJson();
  QPointF topLeft = topLeftCoordinate();
  json["top_left_x"] = topLeft.x();
  json["top_left_y"] = topLeft.y();
  QPointF bottomRight = bottomRightCoordinate();
  json["bottom_right_x"] = bottomRight.x();
  json["bottom_right_y"] = bottomRight.y();
  QPointF topLeftEditable = topLeftEditableCoordinate();
  json["top_left_x_editable"] = topLeftEditable.x();
  json["top_left_y_editable"] = topLeftEditable.y();
  QPointF bottomRightEditable = bottomRightEditableCoordinate();
  json["bottom_right_x_editable"] = bottomRightEditable.x();
  json["bottom_right_y_editable"] = bottomRightEditable.y();
  QPixmap p = pixmap();
  QByteArray ba;
  QBuffer buffer(&ba);
  buffer.open(QIODevice::WriteOnly);
  p.save(&buffer, "jpg");
  json["pixmap"] = QString(ba.toHex());
  return json;
}

void PixmapItem::loadFromJson(const QJsonObject&json)
{
  QPointF topLeft;
  topLeft.setX(json["top_left_x"].toDouble());
  topLeft.setY(json["top_left_y"].toDouble());
  setTopLeftCoordinate(topLeft);
  QPointF bottomRight;
  bottomRight.setX(json["bottom_right_x"].toDouble());
  bottomRight.setY(json["bottom_right_y"].toDouble());
  setBottomRightCoordinate(bottomRight);

  if (json.contains("top_left_x_editable"))
  {
    QPointF topLeftEdit;
    topLeftEdit.setX(json["top_left_x_editable"].toDouble());
    topLeftEdit.setY(json["top_left_y_editable"].toDouble());
    setTopLeftEditableCoordinate(topLeftEdit);
  }
  else
  {
    setTopLeftEditableCoordinate(topLeft);
  }

  if (json.contains("bottom_right_x_editable"))
  {
    QPointF bottomRightEdit;
    bottomRightEdit.setX(json["bottom_right_x_editable"].toDouble());
    bottomRightEdit.setY(json["bottom_right_y_editable"].toDouble());
    setBottomRightEditableCoordinate(bottomRightEdit);
  }
  else
  {
    setBottomRightEditableCoordinate(bottomRight);
  }

  QPixmap p = pixmap();
  QByteArray ba = json["pixmap"].toString().toUtf8();
  p.loadFromData(QByteArray::fromHex(ba), "jpg");
  setPixmap(p);
  PlotTemplateItem::loadFromJson(json);
}

void PixmapItem::setTopLeftCoordinate(QPointF topLeft)
{
  if (m_topLeftCoordinate != topLeft)
  {
    m_topLeftCoordinate = topLeft;
    topLeftCoordinateChanged(m_topLeftCoordinate);
  }
}

void PixmapItem::setBottomRightCoordinate(QPointF bottomRight)
{
  if (m_bottomRightCoordinate != bottomRight)
  {
    m_bottomRightCoordinate = bottomRight;
    bottomRightCoordinateChanged(m_bottomRightCoordinate);
  }
}

void PixmapItem::setTopLeftEditableCoordinate(QPointF topLeft)
{
  if (m_topLeftEditableCoordinate != topLeft)
  {
    m_topLeftEditableCoordinate = topLeft;
    topLeftEditableCoordinateChanged(m_topLeftEditableCoordinate);
  }
}

void PixmapItem::setBottomRightEditableCoordinate(QPointF bottomRight)
{
  if (m_bottomRightEditableCoordinate != bottomRight)
  {
    m_bottomRightEditableCoordinate = bottomRight;
    bottomRightEditableCoordinateChanged(m_bottomRightEditableCoordinate);
  }
}

void PixmapItem::setPixmap(const QPixmap& pixmap)
{
  if (m_pixmap.toImage() != pixmap.toImage())
  {
    m_pixmap = pixmap;
    pixmapChanged(m_pixmap);
  }
}

double PixmapItem::opacity() const
{
  return m_opacity;
}

void PixmapItem::setOpacity(double new_opacity)
{
  new_opacity = std::round(new_opacity * 100) / 100;
  if (qFuzzyCompare(m_opacity, new_opacity))
    return;
  m_opacity = new_opacity;
  emit opacityChanged(m_opacity);
}

ShapeItem::ShapeItem(RootItem *rootItem)
  : PlotTemplateItem (rootItem)
{
  auto axisX = rootItem->axisX();
  auto axisY = rootItem->axisY();
  double halfX = 0;
  double halfY = 0;
  if (axisX.isLinear)
  {
    halfX = (axisX.from + axisX.to) / 2.;
  }
  else
  {
    halfX = sqrt(axisX.from * axisX.to);
  }
  if (axisY.isLinear)
  {
    halfY = (axisY.from + axisY.to) / 2.;
  }
  else
  {
    halfY = sqrt(axisY.from * axisY.to);
  }
  setCoordinate(QPointF(halfX, halfY));
  setColor(GPShapeItem::GetCycledColor(0));
  setSize(QSize(8, 8));
  setShape(GPShape::Circle);
}

QJsonObject ShapeItem::saveToJson()
{
  QJsonObject json;
  json = PlotTemplateItem::saveToJson();
  json["coordinate_x"] = m_coordinate.x();
  json["coordinate_y"] = m_coordinate.y();
  json["width"] = m_size.width();
  json["height"] = m_size.height();
  json["shape_type"] = m_shapeType;
  json["color"] = m_color.name();
  return json;
}

void ShapeItem::loadFromJson(const QJsonObject& json)
{
  double x = json["coordinate_x"].toDouble();
  double y = json["coordinate_y"].toDouble();
  m_coordinate = QPointF(x,y);
  int width = json["width"].toInt();
  int height = json["height"].toInt();
  m_size = QSize(width, height);
  m_shapeType = (GPShape::ShapeType)json["shape_type"].toInt();
  m_color.setNamedColor(json["color"].toString());
  PlotTemplateItem::loadFromJson(json);
}

void ShapeItem::setCoordinate(QPointF coordinate)
{
  if (m_coordinate != coordinate)
  {
    m_coordinate = coordinate;
    emit coordinateChanged(coordinate);
  }
}

void ShapeItem::setSize(int size)
{
  setSize(QSize(size, size));
}

void ShapeItem::setSize(QSizeF size)
{
  if (m_size != size)
  {
    m_size = size;
    emit sizeChanged(size);
  }
}

void ShapeItem::setColor(QColor color)
{
  if (m_color != color)
  {
    m_color = color;
    emit colorChanged(color);
  }
}

void ShapeItem::setShape(QPainterPath path)
{
  setShape(GPShape::ToShape(path));
}

void ShapeItem::setShape(GPShape::ShapeType type)
{
  if (m_shapeType != type)
  {
    m_shapeType = type;
    emit shapeTypeChanged(m_shapeType);
  }
}

PlotTemplateModel::PlotTemplateModel(QObject *parent)
    : QAbstractItemModel(parent)
    , m_hasChanges(false)
{
  m_rootItem = new RootItem();
}

PlotTemplateModel::~PlotTemplateModel()
{

}

QModelIndex PlotTemplateModel::index(int row,
                                     int column,
                                     const QModelIndex &parent) const
{
  if (row > rowCount(parent) || row < 0 ||
      column > columnCount() || column < 0)
    return QModelIndex();

  if (!parent.isValid()) //parent is invisible root item, i.e. it is top lvl item
  {
    return createIndex(row, column, m_rootItem->items()[row]);
  }
  return QModelIndex();
}

QModelIndex PlotTemplateModel::parent(const QModelIndex &child) const
{
  return QModelIndex();
}

int PlotTemplateModel::rowCount(const QModelIndex &parent) const
{
  if (!parent.isValid())
  {
    return m_rootItem->items().count();
  }
  else
  {
    return 0;
  }
}

int PlotTemplateModel::columnCount(const QModelIndex &parent) const
{
  return 3;
}

QVariant PlotTemplateModel::data(const QModelIndex &index, int role) const
{
  if (!index.isValid())
  {
    return QVariant();
  }
  if (role == Qt::DisplayRole || role == Qt::EditRole )
  {
    if (m_rootItem->items().contains(index.internalPointer()))
    {
      QObject *obj = static_cast <QObject *>(index.internalPointer());
      PlotTemplateItem *item = qobject_cast<PlotTemplateItem*>(obj);
      switch (index.column())
      {
      case 0:
        return item->name();
      case 1:
        return item->isEnabled();
      case 2:
        return item->isVisible();
      }
    }
  }
  return QVariant();
}

bool PlotTemplateModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
  if (!index.isValid())
    return false;
  QObject *obj = static_cast <QObject *>(index.internalPointer());
  if (m_rootItem->items().contains(index.internalPointer()))
  {
    PixmapItem *item = qobject_cast<PixmapItem *>(obj);
    if (role == Qt::DisplayRole || role == Qt::EditRole )
    {
      if (index.column() == 1)
      {
        item->setEnabled(value.toBool());
        return true;
      }
      else if (index.column() == 2)
      {
        item->setVisible(value.toBool());
        return true;
      }
    }
  }
  return false;
}

Qt::ItemFlags PlotTemplateModel::flags(const QModelIndex& index) const
{
  auto flags = QAbstractItemModel::flags(index);
  if (index.column() == 1 || (!index.parent().isValid() && index.column() == 2))
    flags |= Qt::ItemIsEditable;
  return flags;
}

void PlotTemplateModel::FillContextMenu(PlotTemplateItem*item, QMenu& menu, std::map<QAction*, std::function<void(void)>>& actions)
{
  if (!item)
    return;
  const auto& items = m_rootItem->items();
  int lastItemPos = items.size() - 1;
  int itemPos = items.indexOf(item);
  //Layer actions
  QAction* moveForeground = menu.addAction(tr("Move to the foreground"));
  moveForeground->setEnabled(itemPos > 0);
  actions[moveForeground] = [this, itemPos](){ this->moveItemLayer(itemPos, 0); };

  QAction* moveAbove = menu.addAction(tr("Move above"));
  moveAbove->setEnabled(itemPos > 0);
  actions[moveAbove] = [this, itemPos](){ this->moveItemLayer(itemPos, qMax(itemPos - 1, 0)); };

  QAction* moveBelow = menu.addAction(tr("Move below"));
  moveBelow->setEnabled(itemPos < lastItemPos);
  actions[moveBelow] = [this, itemPos, lastItemPos]() { this->moveItemLayer(itemPos, qMin(itemPos + 1, lastItemPos)); };

  QAction* moveBackground = menu.addAction(tr("Move to the background"));
  moveBackground->setEnabled(itemPos < lastItemPos);
  actions[moveBackground] = [this, itemPos, lastItemPos](){ this->moveItemLayer(itemPos, lastItemPos); };

  QAction* remove = menu.addAction(tr("Remove"));
  actions[remove] = [this, item](){ this->removeItem(item); };
}

QJsonObject PlotTemplateModel::saveToJson()
{
  QJsonObject obj = m_rootItem->saveToJson();
  for (auto key: m_customValues.keys())
  {
    obj [key] = m_customValues[key].toString();
  }
  return obj;
}

void PlotTemplateModel::loadFromJson(const QJsonObject &jsonObj)
{
  if (jsonObj != saveToJson())
  {
    m_jsonObject = jsonObj;
    removeAllItems();
    if (!jsonObj.isEmpty())
    {
      QJsonArray jsonItems = jsonObj["items"].toArray();
      beginInsertRows(QModelIndex(), 0, jsonItems.size());
      m_rootItem->loadFromJson(jsonObj);
      for (int i = m_rootItem->items().count() - 1; i >= 0; i--)
      {
          emit itemAdded(m_rootItem->items()[i]);
      }
      endInsertRows();
    }
  }
}

void PlotTemplateModel::cancelChanges()
{
  loadFromJson(m_jsonObject);
}

void PlotTemplateModel::setCustomValues(QString key, QVariant value)
{
  m_customValues[key] = value;
}

void PlotTemplateModel::removeAllItems()
{
  if (m_rootItem->items().isEmpty())
    return;
  beginRemoveRows(QModelIndex(),0, rowCount() - 1);
  for (auto item: m_rootItem->items())
  {
    delete item;
    emit itemRemoved(item);
  }
  m_rootItem->items().clear();
  endRemoveRows();
}

void PlotTemplateModel::removeItem(PlotTemplateItem *item)
{
  int row = m_rootItem->items().indexOf(item);
  beginRemoveRows(QModelIndex(),row, row);
  m_rootItem->items().removeAll(item);
  delete item;
  endRemoveRows();
  emit itemRemoved(item);
}

PixmapItem* PlotTemplateModel::addPixmap()
{
  beginInsertRows(QModelIndex(), 0, 0);
  PixmapItem* item = qobject_cast<PixmapItem*>(m_rootItem->addItem(PlotTemplateItem::PixmapType));
  endInsertRows();
  emit itemAdded(item);
  return item;
}

PolylineItem *PlotTemplateModel::addPolyline()
{
  beginInsertRows(QModelIndex(), 0, 0);
  PolylineItem* item = qobject_cast<PolylineItem*>(m_rootItem->addItem(PlotTemplateItem::PolylineType));
  endInsertRows();
  emit itemAdded(item);
  return item;
}

ShapeItem* PlotTemplateModel::addShape()
{
  beginInsertRows(QModelIndex(), 0, 0);
  ShapeItem* item = qobject_cast<ShapeItem*>(m_rootItem->addItem(PlotTemplateItem::ShapeType));
  endInsertRows();
  emit itemAdded(item);
  return item;
}

TextItem* PlotTemplateModel::addText()
{
  beginInsertRows(QModelIndex(), 0, 0);
  TextItem* item = qobject_cast<TextItem*>(m_rootItem->addItem(PlotTemplateItem::TextType));
  endInsertRows();
  emit itemAdded(item);
  return item;
}

LineItem* PlotTemplateModel::addLine()
{
  beginInsertRows(QModelIndex(), 0, 0);
  LineItem* item = qobject_cast<LineItem*>(m_rootItem->addItem(PlotTemplateItem::LineType));
  endInsertRows();
  emit itemAdded(item);
  return item;
}

void PlotTemplateModel::moveItemLayer(int fromRow, int toRow)
{
  beginMoveRows(QModelIndex(), fromRow, fromRow, QModelIndex(), fromRow < toRow ? toRow + 1 : toRow);
  m_rootItem->items().move(fromRow, toRow);
  endMoveRows();
}

PolylineItem::PolylineItem(RootItem *rootItem)
  : PlotTemplateItem (rootItem)
{
  resetPenStyle();
  resetPenColor();
  resetPenWidth();
  resetLineEndingType();
}

QJsonObject PolylineItem::saveToJson()
{
  auto pointsListToJsonArr = [](const PointsList& list)->QJsonArray
  {
    QJsonArray jlist;
    for(auto& pt : list)
    {
      QJsonArray jpt;
      jpt << pt.x() << pt.y();
      jlist << jpt;
    }
    return jlist;
  };
  QJsonObject json;
  json = PlotTemplateItem::saveToJson();
  json["pen_style"] = mPenStyle;
  // json["pen_color"] = colorToJsonArr(mPenColor);
  json["pen_color"] = mPenColor.name();
  json["pen_width"] = mPenWidth;
  json["points"] = pointsListToJsonArr(mPoints);
  json["fill_color"] = mFillColor.isValid() ? QJsonValue(mFillColor.value<QColor>().name()) :  QJsonValue();
  json["transparency"] = mTransparency.isValid() ? mTransparency.toFloat() : QJsonValue();
  json["line_ending_type"] = mLineEndingType.isValid() ? mLineEndingType.toInt() : QJsonValue();
  json["line_is_closed"] = m_isLineClosed;
  return json;
}

void PolylineItem::loadFromJson(const QJsonObject &json)
{
  auto jsonValueToPointsList = [](QJsonValue v)->PointsList
  {
    PointsList points;
    if(v.isNull() || !v.isArray())
      return points;
    auto arr = v.toArray();
    if(arr.isEmpty())
      return points;
    for(int i = 0; i < arr.size(); i++)
    {
      auto v = arr[i];
      if(v.isNull() || !v.isArray())
        continue;
      auto jpt = v.toArray();
      if(jpt.size() < 2)
        continue;
      points << GPVector2D{jpt[0].toDouble(), jpt[1].toDouble()};
    }
    return points;
  };
  PlotTemplateItem::loadFromJson(json);
  //do not change order from here
  if(json.contains("points") && !json["points"].isNull())
    setPoints(jsonValueToPointsList(json["points"]));
  else
    resetPoints();

  setLineClosed(json["line_is_closed"].toBool());
  //to here and leave it always on topb

  if(json.contains("pen_style") && !json["pen_style"].isNull())
    setPenStyle((Qt::PenStyle)(json["pen_style"].toInt()));
  else
    resetPenStyle();

  if(json.contains("pen_color"))
  {
    auto jcolorNameValue = json["pen_color"];
    if(jcolorNameValue.isNull())
      resetPenColor();
    else
      setPenColor(QColor(jcolorNameValue.toString()));
  }

  if(json.contains("pen_width") && !json["pen_width"].isNull())
    setPenWidth(json["pen_width"].toDouble());
  else
    resetPenWidth();

  if(json.contains("fill_color"))
  {
    auto jcolorNameValue = json["fill_color"];
    if(jcolorNameValue.isNull())
      resetFillColor();
    else
      setFillColor(QColor(jcolorNameValue.toString()));
  }

  if(json.contains("transparency") && !json["transparency"].isNull())
    setTransparency(json["transparency"].toDouble());
  else
    resetTransparency();

  if(json.contains("line_ending_type") && !json["line_ending_type"].isNull())
    setLineEndingType((LineEndingType)(json["line_ending_type"].toInt()));
  else
    resetLineEndingType();
}

void PolylineItem::setPenStyle(Qt::PenStyle newPenStyle)
{
  if(mPenStyle == newPenStyle)
    return;
  mPenStyle = newPenStyle;
  emit penStyleChanged();
}

void PolylineItem::setPenColor(const QColor &newColor)
{
  if(mPenColor == newColor)
    return;
  mPenColor = newColor;
  emit penColorChanged();
}

void PolylineItem::setPenWidth(qreal width)
{
  if(mPenWidth == width)
    return;
  mPenWidth = width;
  emit penWidthChanged();
}

void PolylineItem::setPoints(const PointsList &points)
{
  if(points.size() != mPoints.size())
  {
    mPoints = points;
    emit pointsChanged();
    return;
  }
  bool different = false;
  for(int i = 0; i < mPoints.size(); i++)
  {
    if(!qFuzzyCompare(mPoints[i].x(), points[i].x()))
    {
      different = false;
      break;
    }
    if(!qFuzzyCompare(mPoints[i].y(), points[i].y()))
    {
      different = false;
      break;
    }
  }
  if(different)
  {
    mPoints = points;
    emit pointsChanged();
  }
}

void PolylineItem::setFillColor(QVariant newColorv)
{
  if(!mFillColor.isValid() && newColorv.isValid())//null - not-null case
  {
    mFillColor = newColorv.value<QColor>();
    emit fillColorChanged();
    return;
  }
  auto compareColorWithoutAlpha = [](const QColor& op1, const QColor& op2) -> bool
  {
    return op1.red() == op2.red()
           && op1.green() == op2.green()
           && op1.blue() == op2.blue();
  };
  if(newColorv.isValid())
  {
    if(mFillColor.isValid() && compareColorWithoutAlpha(mFillColor.value<QColor>(), newColorv.value<QColor>()))
      return;
  }
  else if(!mFillColor.isValid())
  {
    return;
  }
  if(!newColorv.isValid())
  {
    mFillColor = newColorv;
  }
  else
  {
    auto color = newColorv.value<QColor>();
    color.setAlphaF(1.0);//remove alpha from color;
    mFillColor = color;
  }
  emit fillColorChanged();
}

void PolylineItem::setTransparency(QVariant transparencyv)
{
  if(!mTransparency.isValid() && transparencyv.isValid())//null - not-null case
  {
    mTransparency = transparencyv.toReal();
    emit transparencyChanged();
    return;
  }
  if(!mTransparency.isValid() && !transparencyv.isValid())
    return;
  if(mTransparency.isValid() &&
      transparencyv.isValid() &&
      qFuzzyCompare(mTransparency.toReal(), transparencyv.toReal()))
    return;
  if(!transparencyv.isValid())
    mTransparency = transparencyv;
  else
    mTransparency = transparencyv.toReal();
  emit transparencyChanged();
}

void PolylineItem::setLineEndingType(QVariant lineEndingTypev)
{
  if(!mLineEndingType.isValid() && !lineEndingTypev.isValid())
    return;
  if(mLineEndingType.isValid() &&
      lineEndingTypev.isValid() &&
      mLineEndingType.toInt() == lineEndingTypev.toInt())
    return;

  if(!lineEndingTypev.isValid())
    mLineEndingType = lineEndingTypev;
  else
    mLineEndingType = (LineEndingType)lineEndingTypev.toInt();

  emit lineEndingChanged();
  return;
}

void PolylineItem::setLineClosed(bool closed)
{
  if(m_isLineClosed != closed)
  {
    m_isLineClosed = closed;
    emit lineClosingStateChanged();
    resetFillColor();
    resetTransparency();
    resetLineEndingType();
  }
}

Qt::PenStyle PolylineItem::penStyle()
{
  return mPenStyle;
}

QColor PolylineItem::penColor()
{
  return mPenColor;
}

qreal PolylineItem::penWidth()
{
  return mPenWidth;
}

PolylineItem::PointsList PolylineItem::points()
{
  return mPoints;
}

QVariant PolylineItem::fillColor()
{
  return mFillColor;
}

QVariant PolylineItem::transparency()
{
  return mTransparency;
}

QVariant PolylineItem::lineEndingType()
{
  return mLineEndingType;
}

bool PolylineItem::isLineClosed()
{
  return m_isLineClosed;
}

void PolylineItem::resetPenStyle()
{
  setPenStyle(Qt::SolidLine);
}

void PolylineItem::resetPenColor()
{
  setPenColor(GPShapeItem::GetCycledColor(0));
}

void PolylineItem::resetPenWidth()
{
  setPenWidth(1.0);
}

void PolylineItem::resetPoints()
{
  setPoints({});
}

void PolylineItem::resetFillColor()
{
  if(m_isLineClosed)
    setFillColor(GPShapeItem::GetCycledColor(0));
  else
    setFillColor(QVariant());
}

void PolylineItem::resetTransparency()
{
  if(m_isLineClosed)
    setTransparency(0);
  else
    setTransparency(QVariant());
}

void PolylineItem::resetLineEndingType()
{
  if(!m_isLineClosed)
    setLineEndingType(LineEndingType::esNone);
  else
    setLineEndingType(QVariant());
}
