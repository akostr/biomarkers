#include "line_item.h"
#include <qapplication.h>
#include "ui/plots/gp_items/gpshape_item.h"


LineItem::LineItem(RootItem *parent)
  : PlotTemplateItem{ parent }
  , mStart(0, 0)
  , mEnd(1, 1)
  , mPenColor(GPShapeItem::GetCycledColor(0))
  , mPenWidth(1)
  , mPenStyle(Qt::SolidLine)
  , mLineEndingType(esNone)
{
}

QJsonObject LineItem::saveToJson()
{
  QJsonObject json;
  json = PlotTemplateItem::saveToJson();
  json["end_x"] = mEnd.x();
  json["end_y"] = mEnd.y();
  json["start_x"] = mStart.x();
  json["start_y"] = mStart.y();
  json["pen_style"] = penStyle();
  json["pen_color"] = mPenColor.name();
  json["pen_width"] = mPenWidth;
  json["pen_line_ending_type"] = mLineEndingType;
  return json;
}

void LineItem::loadFromJson(const QJsonObject &json)
{
  PlotTemplateItem::loadFromJson(json);
  QPointF start;
  setStart({json["start_x"].toDouble(), json["start_y"].toDouble()});
  setEnd({json["end_x"].toDouble(), json["end_y"].toDouble()});
  setPenStyle(static_cast<Qt::PenStyle>(json["pen_style"].toInt()));
  setPenColor(QColor::fromString(json["pen_color"].toString()));
  setPenWidth(json["pen_width"].toInt());
  setLineEndingType(json["pen_line_ending_type"].toInt());
}

QPointF LineItem::start()
{
  return mStart;
}

QPointF LineItem::end()
{
  return mEnd;
}

Qt::PenStyle LineItem::penStyle()
{
  return mPenStyle;
}

QColor LineItem::penColor()
{
  return mPenColor;
}

int LineItem::penWidth()
{
  return mPenWidth;
}

int LineItem::lineEndingType()
{
  return mLineEndingType;
}

void LineItem::setStart(QPointF start)
{
  if (mStart != start)
  {
    mStart = start;
    startChanged(start);
  }
}

void LineItem::setEnd(QPointF end)
{
  if (mEnd != end)
  {
    mEnd = end;
    endChanged(end);
  }

}

void LineItem::setPenStyle(Qt::PenStyle style)
{
  if (mPenStyle != style)
  {
    mPenStyle = style;
    penStyleChanged(style);
  }
}

void LineItem::setPenColor(QColor color)
{
  if (mPenColor != color)
  {
    mPenColor = color;
    penColorChanged(color);
  }
}

void LineItem::setPenWidth(int width)
{
  if (mPenWidth != width)
  {
    mPenWidth = width;
    penWidthChanged(width);
  }
}

void LineItem::setLineEndingType(int lineType)
{
  if (mLineEndingType != lineType)
  {
    mLineEndingType = lineType;
    lineEndingTypeChanged(lineType);
  }
}
