#ifndef GPSHAPE_ITEM_H
#define GPSHAPE_ITEM_H
#include <graphicsplot/graphicsplot.h>
#include "gpshape.h"

class GPShapeItem : public GPItemRect
{
  Q_OBJECT//for properly color name translating
public:
  enum DefaultColor{
    Violet,
    Green,
    Pink,
    Orange,
    Blue,
    Yellow,
    Red,
    Gray,
    PaleGray,
    DarkViolet,
    LightBlue,
    DarkBlue,
    LightViolet,
    DarkGreen,
    LightPink,
    DarkOrange,
    UsualLightGreen,
    DarkYellow,
    LightGreen,
    DarkRed,
    DarkGray,
    DarkLightGreen,
    LastColor,
    CustomColor,
  };
  enum DefaultSize
  {
    Small,
    Medium,
    Big,
    VeryBig
  };

  GPShapeItem(GraphicsPlot *parentPlot,
              QPointF pos = QPointF(0,0),
              GPShape::ShapeType shape = GPShape::Circle,
              QSizeF size = QSizeF(5,5));
  GPShapeItem(GraphicsPlot *parentPlot,
              QPointF pos = QPointF(0,0),
              const QPainterPath& shape = QPainterPath(),
              int pixelSize = 5);
  void SetSize(QSizeF size);
  void SetPos(QPointF plotPos);
  QPointF GetPos();
  void SetShape(GPShape::ShapeType shape);
  void SetShape(const QPainterPath& shape);
  QSizeF GetSize();
  GPShape::ShapeType GetShape() const;
  const QPainterPath& GetPainterPath() const;

  static QColor GetCycledColor(unsigned int ind);
  static DefaultColor GetCycledColorIndex(unsigned int ind);
  static GPShape::ShapeType GetCycledShape(unsigned int ind);
  static QColor ToColor(DefaultColor color);
  static DefaultColor FromColor(const QColor color);
  static QSize ToDefaultSize(DefaultSize size);
  static QString ToString(DefaultColor color);


protected:
  void draw(GPPainter *painter) override;
  GPShape Shape;
  QPainterPath ShapePath;

  static QHash<DefaultColor, QColor> mColorMap;
  static QHash<uint, DefaultColor> mReColorMap;
  static QHash<DefaultColor, const char*> mColorNamesMap;

};
#endif // GPSHAPE_ITEM_H
