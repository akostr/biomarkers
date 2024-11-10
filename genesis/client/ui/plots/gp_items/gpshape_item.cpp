#include "gpshape_item.h"
QColor fromName(QString name)
{
  int alpha = 255;
  if(name.size() > 7)
  {
    alpha = name.right(name.size() - 7).toInt(nullptr, 16);
    name.chop(name.size() - 7);
  }
  int blue = name.right(2).toInt(nullptr, 16);
  name.chop(2);
  int green = name.right(2).toInt(nullptr, 16);
  name.chop(2);
  int red = name.right(2).toInt(nullptr, 16);
  return QColor(red, green, blue, alpha);
};

QHash<GPShapeItem::DefaultColor, QColor> GPShapeItem::mColorMap
    {
     {DarkViolet,      fromName("#4936BB")},
     {Violet,          fromName("#7762EB")},
     {LightViolet,     fromName("#A79CEE")},
     {DarkBlue,        fromName("#0A549F")},
     {Blue,            fromName("#0C88CF")},
     {LightBlue,       fromName("#65BEFF")},
     {DarkGreen,       fromName("#0E7A57")},
     {Green,           fromName("#27B285")},
     {UsualLightGreen, fromName("#7AE3B1")},
     {DarkOrange,      fromName("#C17400")},
     {Orange,          fromName("#FFA217")},
     {Pink,            fromName("#F34C86")},
     {LightPink,       fromName("#FF93B9")},
     {DarkYellow,      fromName("#C8A907")},
     {Yellow,          fromName("#FFD90F")},
     {DarkRed,         fromName("#B51B11")},
     {Red,             fromName("#F34338")},
     {DarkGray,        fromName("#7D8683")},
     {Gray,            fromName("#ABB4B1")},
     {PaleGray,        fromName("#00416633")},
     {DarkLightGreen,  fromName("#7F9B0D")},
     {LightGreen,      fromName("#BCCD40")},
     };
QHash<GPShapeItem::DefaultColor, const char*> GPShapeItem::mColorNamesMap
    {
     {DarkViolet,      QT_TRANSLATE_NOOP("Color", "Dark violet")},
     {Violet,          QT_TRANSLATE_NOOP("Color", "Violet")},
     {LightViolet,     QT_TRANSLATE_NOOP("Color", "Light violet")},
     {DarkBlue,        QT_TRANSLATE_NOOP("Color", "Dark blue")},
     {Blue,            QT_TRANSLATE_NOOP("Color", "Blue")},
     {LightBlue,       QT_TRANSLATE_NOOP("Color", "Light blue")},
     {DarkGreen,       QT_TRANSLATE_NOOP("Color", "Dark green")},
     {Green,           QT_TRANSLATE_NOOP("Color", "Green")},
     {UsualLightGreen, QT_TRANSLATE_NOOP("Color", "Light - green")},
     {DarkOrange,      QT_TRANSLATE_NOOP("Color", "Dark orange")},
     {Orange,          QT_TRANSLATE_NOOP("Color", "Orange")},
     {Pink,            QT_TRANSLATE_NOOP("Color", "Pink")},
     {LightPink,       QT_TRANSLATE_NOOP("Color", "Light pink")},
     {DarkYellow,      QT_TRANSLATE_NOOP("Color", "Dark yellow")},
     {Yellow,          QT_TRANSLATE_NOOP("Color", "Yellow")},
     {DarkRed,         QT_TRANSLATE_NOOP("Color", "Dark red")},
     {Red,             QT_TRANSLATE_NOOP("Color", "Red")},
     {DarkGray,        QT_TRANSLATE_NOOP("Color", "Dark gray")},
     {Gray,            QT_TRANSLATE_NOOP("Color", "Gray")},
     {PaleGray,        QT_TRANSLATE_NOOP("Color", "Pale gray")},
     {DarkLightGreen,  QT_TRANSLATE_NOOP("Color", "Dark lightgreen")},//темно салатовый
     {LightGreen,      QT_TRANSLATE_NOOP("Color", "Lightgreen")}//салатовый
     };

QHash<uint, GPShapeItem::DefaultColor> GPShapeItem::mReColorMap
    {
     {fromName("#4936BB").rgba(), DarkViolet},
     {fromName("#7762EB").rgba(), Violet},
     {fromName("#A79CEE").rgba(), LightViolet},
     {fromName("#0A549F").rgba(), DarkBlue},
     {fromName("#0C88CF").rgba(), Blue},
     {fromName("#65BEFF").rgba(), LightBlue},
     {fromName("#0E7A57").rgba(), DarkGreen},
     {fromName("#27B285").rgba(), Green},
     {fromName("#7AE3B1").rgba(), UsualLightGreen},
     {fromName("#C17400").rgba(), DarkOrange},
     {fromName("#FFA217").rgba(), Orange},
     {fromName("#F34C86").rgba(), Pink},
     {fromName("#FF93B9").rgba(), LightPink},
     {fromName("#C8A907").rgba(), DarkYellow},
     {fromName("#FFD90F").rgba(), Yellow},
     {fromName("#B51B11").rgba(), DarkRed},
     {fromName("#F34338").rgba(), Red},
     {fromName("#7D8683").rgba(), DarkGray},
     {fromName("#ABB4B1").rgba(), Gray},
     {fromName("#00416633").rgba(), PaleGray},
     {fromName("#7F9B0D").rgba(), DarkLightGreen},
     {fromName("#BCCD40").rgba(), LightGreen},
     };
GPShapeItem::GPShapeItem(GraphicsPlot *parentPlot,
                         QPointF pos,
                         GPShape::ShapeType shape,
                         QSizeF size)
  : GPItemRect(parentPlot)
  , Shape(shape)
  , ShapePath(Shape.GetPath())
{
  createPosition("center");
  auto p = position("center");
  p->setType(GPItemPosition::ptPlotCoords);
  topLeft->setType(GPItemPosition::ptAbsolute);
  topLeft->setParentAnchor(p);
  bottomRight->setType(GPItemPosition::ptAbsolute);
  bottomRight->setParentAnchor(p);
  SetSize(size);
  SetPos(pos);
}

GPShapeItem::GPShapeItem(GraphicsPlot *parentPlot, QPointF pos, const QPainterPath &shape, int pixelSize)
  : GPItemRect(parentPlot)
  , Shape(GPShape::PathShape)
  , ShapePath(shape)
{
  createPosition("center");
  auto p = position("center");
  p->setType(GPItemPosition::ptPlotCoords);
  topLeft->setType(GPItemPosition::ptAbsolute);
  topLeft->setParentAnchor(p);
  bottomRight->setType(GPItemPosition::ptAbsolute);
  bottomRight->setParentAnchor(p);
  SetSize(QSizeF(pixelSize, pixelSize));
  SetPos(pos);
}

void GPShapeItem::SetSize(QSizeF size)
{
  topLeft->setCoords(-size.width()/2, -size.height()/2);
  bottomRight->setCoords(size.width()/2, size.height()/2);
}

void GPShapeItem::SetPos(QPointF plotPos)
{
  //place item center in given coordinates.
  auto p = position("center");
  p->setCoords(plotPos);
}

QPointF GPShapeItem::GetPos()
{
  return position("center")->coords();
}

void GPShapeItem::SetShape(GPShape::ShapeType shape)
{
  Shape.SetShape(shape);
  ShapePath = Shape.GetPath();
}

void GPShapeItem::SetShape(const QPainterPath &shape)
{
  Shape.SetShape(GPShape::PathShape);
  ShapePath = shape;
}

QSizeF GPShapeItem::GetSize()
{
  auto delta = bottomRight->coords() - topLeft->coords();
  return QSizeF(delta.x(), delta.y());
}

GPShape::ShapeType GPShapeItem::GetShape() const
{
  return Shape.GetShape();
}

const QPainterPath &GPShapeItem::GetPainterPath() const
{
  return ShapePath;
}

QColor GPShapeItem::GetCycledColor(unsigned int ind)
{
  DefaultColor color = (DefaultColor)(ind % LastColor);
  return ToColor(color);
}

GPShapeItem::DefaultColor GPShapeItem::GetCycledColorIndex(unsigned int ind)
{
  return (DefaultColor)(ind % LastColor);
}

GPShape::ShapeType GPShapeItem::GetCycledShape(unsigned int ind)
{
  return (GPShape::ShapeType)(ind % GPShape::LastShape);
}

QColor GPShapeItem::ToColor(DefaultColor color)
{
  return mColorMap.value(color, QColor());
}

GPShapeItem::DefaultColor GPShapeItem::FromColor(const QColor color)
{
  return mReColorMap.value(color.rgba(), CustomColor);
}

QSize GPShapeItem::ToDefaultSize(DefaultSize size)
{
  switch (size) {
  case Small:
    return QSize(4, 4);
  case Medium:
    return QSize(8, 8);
  case Big:
    return QSize(10, 10);
  case VeryBig:
    return QSize(12, 12);
  default:
    return QSize();
  };
}

QString GPShapeItem::ToString(DefaultColor color)
{
  return QCoreApplication::translate("Color", mColorNamesMap.value(color, ""));
}

void GPShapeItem::draw(GPPainter *painter)
{
  QPointF size = bottomRight->pixelPosition() - topLeft->pixelPosition();
  painter->save();
  painter->translate(topLeft->pixelPosition());
  painter->scale(size.x(), size.y());
  painter->setBrush(brush());
  auto pn = pen();
  pn.setWidthF(pn.widthF() / size.x());
  painter->setPen(pn);
  painter->drawPath(ShapePath);
  painter->restore();
}
