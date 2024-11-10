#include "gpshape_with_label_item.h"


GPShapeWithLabelItem::GPShapeWithLabelItem(GraphicsPlot *parentPlot, QPointF pos, GPShape::ShapeType shape, QSizeF size, QString labelText, QFont font)
  : GPShapeItem(parentPlot, pos, shape, size)
  , LabelFont(font)
  , isLabelHidden(false)
  , IsInactive(false)
  , Text(new QStaticText(labelText))
{
  setMargins(0,0,0,0);
  setLabelSpacing(5);
  Text->prepare();
}

GPShapeWithLabelItem::GPShapeWithLabelItem(GraphicsPlot *parentPlot, QPointF pos, const QPainterPath &shape, int pixelSize, QString labelText, QFont font)
  : GPShapeItem(parentPlot, pos, shape, pixelSize),
  LabelFont(font),
  isLabelHidden(false),
  IsInactive(false),
  Text(new QStaticText(labelText))
{
  setMargins(0,0,0,0);
  setLabelSpacing(5);
  Text->prepare();
}


void GPShapeWithLabelItem::SetSize(QSizeF size)
{
  GPShapeItem::SetSize(size);
}

void GPShapeWithLabelItem::draw(GPPainter *painter)
{

  auto tmpRect = boundingRect();
  if (tmpRect.topLeft().toPoint() == tmpRect.bottomRight().toPoint())
    return;
  QRectF rect = tmpRect.normalized();
  double clipPad = mainPen().widthF();
  QRectF boundingRect = rect.adjusted(-clipPad, -clipPad, clipPad, clipPad);
  if (boundingRect.intersects(clipRect())) // only draw if bounding rect of rect item is visible in cliprect
  {
    painter->save();
    painter->setPen(plateMainPen());
    painter->setBrush(plateMainBrush());
    painter->drawRoundedRect(rect, 4, 4);
    painter->restore();

    GPShapeItem::draw(painter);

    if(!isLabelHidden)
    {
      painter->setFont(LabelFont);
      if(IsInactive)
        painter->setPen(QColor::fromRgbF(0,0,0,0.2));
      painter->drawStaticText(right->pixelPosition() + QPointF(LabelSpacing, -Text->size().height()/2), *Text);
    }
  }

}

const QPen &GPShapeWithLabelItem::plateMainPen()
{
  if(selected())
    return PlateSelectedPen;
  return PlatePen;
}

const QBrush &GPShapeWithLabelItem::plateMainBrush()
{
  if(selected())
    return PlateSelectedBrush;
  return PlateBrush;
}

const QFont &GPShapeWithLabelItem::getLabelFont() const
{
  return LabelFont;
}

void GPShapeWithLabelItem::setLabelFont(const QFont &newLabelFont)
{
  LabelFont = newLabelFont;
  parentPlot()->replot(GraphicsPlot::rpQueuedReplot);
}

void GPShapeWithLabelItem::setMargins(int left, int right, int top, int bottom)
{
  LeftMargin = left;
  RightMargin = right;
  TopMargin = top;
  BottomMargin = bottom;
  parentPlot()->replot(GraphicsPlot::rpQueuedReplot);
}

void GPShapeWithLabelItem::setLabelSpacing(int spacing)
{
  LabelSpacing = spacing;
  parentPlot()->replot(GraphicsPlot::rpQueuedReplot);
}

QRectF GPShapeWithLabelItem::boundingRect() const
{
  double width = 0;
  double height = 0;
  {
    width = right->pixelPosition().x() - left->pixelPosition().x();
    if(!isLabelHidden)
    {
      width += LabelSpacing + Text->size().width();
    }

  }
  {
    double shapeHeight = bottom->pixelPosition().y() - top->pixelPosition().y();
    height = shapeHeight;
    if(!isLabelHidden)
    {
      double textHeight = Text->size().height();
      if(textHeight > shapeHeight)
        height = textHeight;
    }
  }
  return QRectF(left->pixelPosition() + QPointF(-LeftMargin, -TopMargin - height/2),
                QSizeF(width + LeftMargin + RightMargin, height + TopMargin + BottomMargin));
}

void GPShapeWithLabelItem::setShapePen(const QPen &pen)
{
  setPen(pen);
}

void GPShapeWithLabelItem::setShapeSelectedPen(const QPen &pen)
{
  setSelectedPen(pen);
}

void GPShapeWithLabelItem::setPlatePen(const QPen &pen)
{
  PlatePen = pen;
}

void GPShapeWithLabelItem::setPlateSelectedPen(const QPen &pen)
{
  PlateSelectedPen = pen;
}

void GPShapeWithLabelItem::setShapeBrush(const QBrush &brush)
{
  setBrush(brush);
}

void GPShapeWithLabelItem::setShapeSelectedBrush(const QBrush &brush)
{
  setSelectedBrush(brush);
}

void GPShapeWithLabelItem::setPlateBrush(const QBrush &brush)
{
  PlateBrush = brush;
}

void GPShapeWithLabelItem::setPlateSelectedBrush(const QBrush &brush)
{
  PlateSelectedBrush = brush;
}

void GPShapeWithLabelItem::setColor(unsigned int color)
{
  Color = (GPShapeItem::DefaultColor)color;
  activeShapeColor = GPShapeItem::ToColor(Color);
  inactiveShapeColor = activeShapeColor;
  inactiveShapeColor.setAlphaF(0.2);

  if(IsInactive)
    setShapeBrush(inactiveShapeColor);
  else
    setShapeBrush(activeShapeColor);
}

void GPShapeWithLabelItem::setColor(QColor color)
{
  Color = FromColor(color);
  activeShapeColor = color;
  inactiveShapeColor = activeShapeColor;
  inactiveShapeColor.setAlphaF(0.2);

  if(IsInactive)
    setShapeBrush(inactiveShapeColor);
  else
    setShapeBrush(activeShapeColor);
}

void GPShapeWithLabelItem::setInactive(bool isInactive)
{
  if(IsInactive == isInactive)
    return;
  IsInactive = isInactive;
  if(IsInactive)
  {
    setShapeBrush(inactiveShapeColor);
  }
  else
  {
    setShapeBrush(activeShapeColor);
  }
}

bool GPShapeWithLabelItem::isInactive()
{
  return IsInactive;
}

unsigned int GPShapeWithLabelItem::getColor()
{
  return Color;
}

void GPShapeWithLabelItem::setLabelHidden(bool hidden)
{
  isLabelHidden = hidden;
  parentPlot()->replot(GraphicsPlot::rpQueuedReplot);
}

QString GPShapeWithLabelItem::getLabelText() const
{
  return Text->text();
}

void GPShapeWithLabelItem::setLabelText(const QString &newLabelText)
{
  Text->setText(newLabelText);
  Text->prepare();
  parentPlot()->replot(GraphicsPlot::rpQueuedReplot);
}

double GPShapeWithLabelItem::selectTest(const QPointF& pos, bool onlySelectable, QVariant* details) const
{
  if(onlySelectable && !selectable())
    return -1;

  auto rect = boundingRect();

  if(rect.contains(pos))
  {
    auto radius = rect.center() - rect.topLeft();
    auto delta = rect.center() - pos;
    double r = sqrt(pow(radius.x(), 2) + pow(radius.y(), 2));
    double d = sqrt(pow(delta.x(), 2) + pow(delta.y(), 2));
    return r/d;
  }
  else return -1;
}

void GPShapeWithLabelItem::selectEvent(QMouseEvent *event, bool additive, const QVariant &details, bool *selectionStateChanged)
{
  if(selectionStateChanged) *selectionStateChanged = false;

  if((additive || !selected()) && selectionStateChanged)
    *selectionStateChanged = true;
  if(additive)
    setSelected(!selected());
  else
    setSelected(true);
}

void GPShapeWithLabelItem::deselectEvent(bool *selectionStateChanged)
{
  if(selectionStateChanged) *selectionStateChanged = false;

  if(selected())
  {
    if(selectionStateChanged) *selectionStateChanged = true;
    setSelected(false);
  }
}
