#ifndef GPSHAPE_H
#define GPSHAPE_H
#include <QCoreApplication>
#include <QPainterPath>

class GPShape
{
  Q_DECLARE_TR_FUNCTIONS(GPShape)

public:
  enum ShapeType
  {
    Circle,
    Square,
    Diamond,
    Triangle,
    Pentagon,
    InvertedTriangle,
    RightTriangle,
    Ellipse,
    PentaStar,
    Rectangle,
    Trapezoid,
    InvertedTrapezoid,
    Parallelogramm,
    Heptagon,
    HeptaStar,
    HalfCircle,
    DecaStar,
    CircleSector,
    Cube,
    Hourglass,
    Cloud,
    CrossInSquare,
    Lightning,
    Plus,
    Minus,
    BrokenSquare,
    BrokenDiamond,
    Bookmark,
    StarInCircle,
    WindMill,
    Tree,
    CrossedCircles,
    Flag,

    LastShape,
    PathShape,
  };

  static QString ToString(ShapeType type);

  QString ToString();
  GPShape(ShapeType type);

  ShapeType GetShape() const;
  void SetShape(ShapeType newShape);

  QPainterPath GetPath() const;
  static QPainterPath GetPath(ShapeType shape);
  static ShapeType ToShape(const QPainterPath& path);

  double GetSizeX() const;
  void SetSizeX(double newSizeX);

  double GetSizeY() const;
  void SetSizeY(double newSizeY);

private:
  ShapeType Shape;
  double SizeX;
  double SizeY;

  static QHash<ShapeType, QPainterPath> mShapeMap;
  static QHash<ShapeType, const char*> mShapeNamesMap;
  static QHash<QByteArray, ShapeType> mReShapeMap;

};

#endif // GPSHAPE_H
