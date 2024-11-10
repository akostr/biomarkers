#ifndef GPSHAPE_WITH_LABEL_ITEM_CUSTOM_HOTTELING_H
#define GPSHAPE_WITH_LABEL_ITEM_CUSTOM_HOTTELING_H

#include "gpshape_with_label_item.h"

class GPShapeWithLabelItemCustomHotteling: public GPShapeWithLabelItem
{
  Q_OBJECT
public:
  GPShapeWithLabelItemCustomHotteling(GraphicsPlot *parentPlot,
                                      int componentPos,
                                      QPointF pos = QPointF(0,0),
                                      GPShape::ShapeType shape = GPShape::Circle,
                                      QSizeF size = QSizeF(5,5),
                                      QString labelText = "",
                                      QFont font = QFont());

  //    virtual ~GPShapeWithLabelItemCustomHotteling() = default;
  //    using GPShapeWithLabelItem::setColor;
  void setColor(QColor newColor);
  void setInactive(bool isInactive) override;

  int getComponentPos() const;

protected:
  QColor ItemColor;
  int ComponentPos;
};

#endif // GPSHAPE_WITH_LABEL_ITEM_CUSTOM_HOTTELING_H
