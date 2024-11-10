#include "gpshape_with_label_item_custom_hotteling.h"


GPShapeWithLabelItemCustomHotteling::GPShapeWithLabelItemCustomHotteling(GraphicsPlot *parentPlot,
                                                                         int componentPos,
                                                                         QPointF pos,
                                                                         GPShape::ShapeType shape,
                                                                         QSizeF size,
                                                                         QString labelText,
                                                                         QFont font)
  : GPShapeWithLabelItem (parentPlot, pos, shape, size, labelText, font)
{
  //    setColor(ToColor(GPShapeItem::Gray));
  if (componentPos < 0){
    Q_ASSERT(true);
    return;
  }
  ComponentPos = componentPos;

}

void GPShapeWithLabelItemCustomHotteling::setColor(QColor newColor)
{
  ItemColor = newColor;
  GPShapeItem::setBrush(QBrush(ItemColor));

}

void GPShapeWithLabelItemCustomHotteling::setInactive(bool isInactive)
{
  if(IsInactive == isInactive)
    return;
  IsInactive = isInactive;
  if(IsInactive)
  {
    auto inactiveColor = ItemColor;
    inactiveColor.setAlphaF(0.2);
    setShapeBrush(inactiveColor);

  }
  else
  {
    setShapeBrush(ItemColor);
  }

}

int GPShapeWithLabelItemCustomHotteling::getComponentPos() const
{
  return ComponentPos;
}
