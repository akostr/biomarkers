#include "gpshape.h"

QPainterPath createPath(GPShape::ShapeType shape)
{
  QPainterPath path;
  switch(shape)
  {
  case GPShape::Diamond:
    path.moveTo(0  , 0.5);
    path.lineTo(0.5, 0  );
    path.lineTo(1  , 0.5);
    path.lineTo(0.5, 1  );
    path.lineTo(0  , 0.5);
    break;
  case GPShape::Square:
    path.lineTo(1, 0);
    path.lineTo(1, 1);
    path.lineTo(0, 1);
    path.lineTo(0, 0);
    break;
  case GPShape::Triangle:
    path.moveTo(0.5, 0);
    path.lineTo(1  , 1);
    path.lineTo(0  , 1);
    path.lineTo(0.5, 0);
    break;
  case GPShape::InvertedTriangle:
    path.moveTo(0.5, 1);
    path.lineTo(0  , 0);
    path.lineTo(1  , 0);
    path.lineTo(0.5, 1);
    break;
  case GPShape::Ellipse:
    path.addEllipse(0, 0.25, 1, 0.5);
    break;
  case GPShape::GPShape::RightTriangle:
    path.lineTo(0,1);
    path.lineTo(1,1);
    path.lineTo(0,0);
    break;
  case GPShape::HalfCircle:
    path.moveTo(0.5, 0.5);
    path.arcTo(QRectF(QPointF(0,0), QPointF(1,1)), 90, 180);
    path.closeSubpath();
    break;
  case GPShape::CircleSector:
    path.moveTo(0.5, 1);
    path.arcTo(QRectF(QPointF(-0.5,0), QPointF(1.5,2)), 90 - 26.565, 26.565 * 2);
    path.closeSubpath();
    break;
  case GPShape::Parallelogramm:
    path.moveTo(0.2, 0.2);
    path.lineTo(1,0.2);
    path.lineTo(0.8,0.8);
    path.lineTo(0,0.8);
    path.lineTo(0.2,0.2);
    break;
  case GPShape::Rectangle:
    path.moveTo(0, 0.25);
    path.lineTo(1, 0.25);
    path.lineTo(1, 0.75);
    path.lineTo(0, 0.75);
    path.lineTo(0, 0.25);
    break;
  case GPShape::Pentagon:
    path.moveTo(0.5, 0.05);
    path.lineTo(0.955, 0.4);
    path.lineTo(0.8, 0.95);
    path.lineTo(0.2, 0.95);
    path.lineTo(0.055, 0.4);
    path.lineTo(0.5, 0.05);
    break;
  case GPShape::Heptagon:
    path.moveTo(0.5,      0.05);
    path.lineTo(0.890916, 0.238255);
    path.lineTo(0.987464, 0.66126);
    path.lineTo(0.716942, 1.0);
    path.lineTo(0.283058, 1.0);
    path.lineTo(0.012536, 0.66126);
    path.lineTo(0.109084, 0.238255);
    path.lineTo(0.5,      0.05);
    break;
  case GPShape::HeptaStar:
    path.moveTo(0.5,      0.05);
    path.lineTo(0.597624, 0.347282);
    path.lineTo(0.890916, 0.238255);
    path.lineTo(0.719359, 0.499933);
    path.lineTo(0.987464, 0.66126);
    path.lineTo(0.675912, 0.690285);
    path.lineTo(0.716942, 1.000);
    path.lineTo(0.5,      0.775);
    path.lineTo(0.283058, 1.000);
    path.lineTo(0.324088, 0.690285);
    path.lineTo(0.012536, 0.66126);
    path.lineTo(0.280641, 0.499933);
    path.lineTo(0.109084, 0.238255);
    path.lineTo(0.402376, 0.347282);
    path.lineTo(0.5,      0.05);
    break;
  case GPShape::DecaStar:
    path.moveTo(0.958341, 0.45);
    path.lineTo(0.856675, 0.334166);
    path.lineTo(0.870841, 0.180833);
    path.lineTo(0.720425, 0.146666);
    path.lineTo(0.641675, 0.0141663);
    path.lineTo(0.500008, 0.0749996);
    path.lineTo(0.358341, 0.0141663);
    path.lineTo(0.279591, 0.146666);
    path.lineTo(0.129175, 0.180416);
    path.lineTo(0.143341, 0.33375);
    path.lineTo(0.0416748, 0.45);
    path.lineTo(0.143341, 0.565833);
    path.lineTo(0.129175, 0.719583);
    path.lineTo(0.279591, 0.75375);
    path.lineTo(0.358341, 0.88625);
    path.lineTo(0.500008, 0.825);
    path.lineTo(0.641675, 0.885833);
    path.lineTo(0.720425, 0.753333);
    path.lineTo(0.870841, 0.719166);
    path.lineTo(0.856675, 0.565833);
    path.lineTo(0.958341, 0.45);
    break;
  case GPShape::PentaStar:
    path.moveTo(0.5, 0.05);
    path.lineTo(0.625, 0.375);
    path.lineTo(0.975, 0.4);
    path.lineTo(0.7125, 0.625);
    path.lineTo(0.8, 0.95);
    path.lineTo(0.5, 0.775);
    path.lineTo(0.2, 0.95);
    path.lineTo(0.2875, 0.625);
    path.lineTo(0.025, 0.4);
    path.lineTo(0.375, 0.375);
    path.lineTo(0.5, 0.05);
    break;
  case GPShape::Trapezoid:
    path.moveTo(0.3,0.2);
    path.lineTo(0.7,0.2);
    path.lineTo(1,0.8);
    path.lineTo(0,0.8);
    path.lineTo(0.3,0.2);
    break;
  case GPShape::InvertedTrapezoid:
    path.moveTo(0, 0.2);
    path.lineTo(1, 0.2);
    path.lineTo(0.7, 0.8);
    path.lineTo(0.3, 0.8);
    path.lineTo(0, 0.2);
    break;
  case GPShape::CrossInSquare:
    path.moveTo(0.1, 0);
    path.lineTo(0.5, 0.4);
    path.lineTo(0.9, 0);
    path.lineTo(0.1, 0);

    path.moveTo(1, 0.1);
    path.lineTo(1, 0.9);
    path.lineTo(0.6, 0.5);
    path.lineTo(1, 0.1);

    path.moveTo(0.1, 1);
    path.lineTo(0.5, 0.6);
    path.lineTo(0.9, 1);
    path.lineTo(0.1, 1);

    path.moveTo(0, 0.1);
    path.lineTo(0, 0.9);
    path.lineTo(0.4, 0.5);
    path.lineTo(0, 0.1);
    break;
  case GPShape::BrokenSquare:
    path.addRect(QRectF(QPointF(0,0), QPointF(0.4, 0.4)));
    path.addRect(QRectF(QPointF(0.6,0), QPointF(1, 0.4)));
    path.addRect(QRectF(QPointF(0,0.6), QPointF(0.4, 1)));
    path.addRect(QRectF(QPointF(0.6,0.6), QPointF(1, 1)));
    break;
  case GPShape::Bookmark:
    path.moveTo(0.2, 0.1);
    path.lineTo(0.8, 0.1);
    path.lineTo(0.8, 0.9);
    path.lineTo(0.5, 0.75);
    path.lineTo(0.2, 0.9);
    path.lineTo(0.2, 0.1);
    break;
  case GPShape::Flag:
    path.moveTo(0.2, 0.1);
    path.lineTo(0.8, 0.1);
    path.lineTo(0.65, 0.35);
    path.lineTo(0.8, 0.6);
    path.lineTo(0.2, 0.6);
    path.lineTo(0.2, 0.1);
    path.addRect(QRectF(QPointF(0.2, 0.6), QPointF(0.3, 0.9)));
    break;
  case GPShape::BrokenDiamond:
    path.moveTo(0.0,        0.499956);
    path.lineTo(0.222205,   0.277731);
    path.lineTo(0.444497,   0.499956);
    path.lineTo(0.222205,   0.722269);
    path.lineTo(0.0,        0.499956);
    path.moveTo(0.277802,   0.77778);
    path.lineTo(0.500007,   0.555554);
    path.lineTo(0.722207,   0.77778);
    path.lineTo(0.500007,   1);
    path.lineTo(0.277802,   0.77778);
    path.moveTo(0.277802,   0.22222);
    path.lineTo(0.500007,   0.0);
    path.lineTo(0.722207,   0.22222);
    path.lineTo(0.500007,   0.444446);
    path.lineTo(0.277802,   0.22222);
    path.moveTo(0.555591,   0.499956);
    path.lineTo(0.777796,   0.277731);
    path.lineTo(0.999996,   0.499956);
    path.lineTo(0.777796,   0.722269);
    path.lineTo(0.555591,   0.499956);
    break;
  case GPShape::WindMill:
    path.moveTo(0.5, 0.25);
    path.arcTo(QRectF(QPointF(0.25,0), QPointF(0.75,0.5)), 90, 180);
    path.closeSubpath();
    path.moveTo(0.75, 0.5);
    path.arcTo(QRectF(QPointF(0.5,0.25), QPointF(1,0.75)), 0, 180);
    path.closeSubpath();
    path.moveTo(0.5, 0.75);
    path.arcTo(QRectF(QPointF(0.25,0.5), QPointF(0.75,1)), -90, 180);
    path.closeSubpath();
    path.moveTo(0.25, 0.5);
    path.arcTo(QRectF(QPointF(0,0.25), QPointF(0.5,0.75)), 180, 180);
    path.closeSubpath();
    break;
  case GPShape::Plus:
  {
    path.moveTo(0.1, 0.4);
    path.lineTo(0.4, 0.4);
    path.lineTo(0.4, 0.1);
    path.lineTo(0.6, 0.1);
    path.lineTo(0.6, 0.4);
    path.lineTo(0.9, 0.4);
    path.lineTo(0.9, 0.6);
    path.lineTo(0.6, 0.6);
    path.lineTo(0.6, 0.9);
    path.lineTo(0.4, 0.9);
    path.lineTo(0.4, 0.6);
    path.lineTo(0.1, 0.6);
    path.lineTo(0.1, 0.4);
    break;
  }
  case GPShape::Minus:
  {
    path.addRect(QRectF(QPointF(0.1, 0.4), QPointF(0.9, 0.6)));
    break;
  }
  case GPShape::Hourglass:
    path.moveTo(0.25, 0.0333008);
    path.lineTo(0.25, 0.283301);
    path.lineTo(0.416699, 0.45);
    path.lineTo(0.25, 0.616699);
    path.lineTo(0.25, 0.866699);
    path.lineTo(0.75, 0.866699);
    path.lineTo(0.75, 0.616699);
    path.lineTo(0.583301, 0.45);
    path.lineTo(0.75, 0.283301);
    path.lineTo(0.75, 0.0333008);
    path.lineTo(0.25, 0.0333008);
    break;
  case GPShape::CrossedCircles:
    path.addEllipse(QRectF(QPointF(0,0.15), QPointF(0.7, 0.85)));
    path.addEllipse(QRectF(QPointF(0.3, 0.15), QPointF(1, 0.85)));
    break;
  case GPShape::Cloud:
    path.moveTo(0.806006, 0.267902);
    path.cubicTo(QPointF(0.792006,     0.197002 ),
                 QPointF(0.753998,     0.1333   ),
                 QPointF(0.697998,     0.0875003));
    path.cubicTo(QPointF(0.641998,     0.0416003),
                 QPointF(0.572   ,     0.0165995),
                 QPointF(0.5     ,     0.0166995));
    path.cubicTo(QPointF(0.38    ,     0.0166995),
                 QPointF(0.274998,     0.0850011),
                 QPointF(0.222998,     0.184601 ));
    path.cubicTo(QPointF(0.161998,     0.191301 ),
                 QPointF(0.105001,     0.220403 ),
                 QPointF(0.0640015,    0.266303 ));
    path.cubicTo(QPointF(0.0230015,    0.312203 ),
                 QPointF(0.0,          0.371701 ),
                 QPointF(0.0,          0.433301 ));
    path.cubicTo(QPointF(0.0,          0.499601 ),
                 QPointF(0.025998,     0.563202 ),
                 QPointF(0.072998,     0.610102 ));
    path.cubicTo(QPointF(0.119998,     0.657002 ),
                 QPointF(0.184,        0.683301 ),
                 QPointF(0.25,         0.683301 ));

    path.lineTo(0.792004, 0.683301);

    path.cubicTo(QPointF(0.819004, 0.683301),
                 QPointF(0.845996, 0.677899),
                 QPointF(0.870996, 0.667499));

    path.cubicTo(QPointF(0.896996, 0.656999),
                 QPointF(0.920001, 0.641603),
                 QPointF(0.939001, 0.622303));

    path.cubicTo(QPointF(0.958001, 0.603003),
                 QPointF(0.973997, 0.58    ),
                 QPointF(0.983997, 0.5547  ));

    path.cubicTo(QPointF(0.994997, 0.5294  ),
                 QPointF(1,        0.5023  ),
                 QPointF(1,        0.475   ));

    path.cubicTo(QPointF(1,        0.365   ),
                 QPointF(0.915006, 0.275802),
                 QPointF(0.806006, 0.267902));
    break;

  case GPShape::Tree:
    path.addEllipse(QRectF(QPointF(0.15, 0), QPointF(0.85, 0.7)));
    path.addRect(QRectF(QPointF(0.15, 0.9), QPointF(0.85, 1)));
    path.addRect(QRectF(QPointF(0.45, 0.7), QPointF(0.55, 0.9)));
    break;
  case GPShape::Cube:
  {
    path.moveTo(0.5, 0);
    path.lineTo(0.95, 0.25);
    path.lineTo(0.95, 0.75);
    path.lineTo(0.5, 1);
    path.lineTo(0.05, 0.75);
    path.lineTo(0.05, 0.25);
    path.lineTo(0.5, 0);
    QPolygonF sub;
    sub << QPointF(0.2, 0.275)
        << QPointF(0.5, 0.1)
        << QPointF(0.8, 0.275)
        << QPointF(0.5, 0.45)
        << QPointF(0.2, 0.275);
    path.addPolygon(sub);
    break;
  }
  case GPShape::Lightning:
    path.moveTo(0.05, 0.05);
    path.lineTo(0.05, 0.6 );
    path.lineTo(0.2 , 0.6 );
    path.lineTo(0.2 , 1   );
    path.lineTo(0.55, 0.45);
    path.lineTo(0.35, 0.45);
    path.lineTo(0.55, 0.05);
    path.lineTo(0.05, 0.05);
    break;
  case GPShape::Circle:
    path.addEllipse(0, 0, 1, 1);
    break;
  case GPShape::StarInCircle:
    path.addEllipse(QRectF(QPointF(0, 0), QPointF(1,1)));
    path.moveTo(0.711495, 0.799757);
    path.lineTo(0.499994, 0.672653);
    path.lineTo(0.288493, 0.799757);
    path.lineTo(0.344497, 0.559951);
    path.lineTo(0.157993, 0.398086);
    path.lineTo(0.403994, 0.377697);
    path.lineTo(0.499994, 0.149879);
    path.lineTo(0.595994, 0.376497);
    path.lineTo(0.841995, 0.398086);
    path.lineTo(0.655503, 0.558751);
    path.lineTo(0.711495, 0.799757);
    break;
  default: break;
  }

  path.setFillRule(Qt::OddEvenFill);
  return path;
};
QByteArray stringifyPath(const QPainterPath& path)
{
  QByteArray path_bin;
  QDataStream stream(&path_bin, QIODeviceBase::WriteOnly);
  stream << path;
  return path_bin;
}
QHash<GPShape::ShapeType, QPainterPath> GPShape::mShapeMap
    {
        {Circle,              createPath(Circle)},
        {Square,              createPath(Square)},
        {Diamond,             createPath(Diamond)},
        {Triangle,            createPath(Triangle)},
        {Pentagon,            createPath(Pentagon)},
        {InvertedTriangle,    createPath(InvertedTriangle)},
        {RightTriangle,       createPath(RightTriangle)},
        {Ellipse,             createPath(Ellipse)},
        {PentaStar,           createPath(PentaStar)},
        {Rectangle,           createPath(Rectangle)},
        {Trapezoid,           createPath(Trapezoid)},
        {InvertedTrapezoid,   createPath(InvertedTrapezoid)},
        {Parallelogramm,      createPath(Parallelogramm)},
        {Heptagon,            createPath(Heptagon)},
        {HeptaStar,           createPath(HeptaStar)},
        {HalfCircle,          createPath(HalfCircle)},
        {DecaStar,            createPath(DecaStar)},
        {CircleSector,        createPath(CircleSector)},
        {Cube,                createPath(Cube)},
        {Hourglass,           createPath(Hourglass)},
        {Cloud,               createPath(Cloud)},
        {CrossInSquare,       createPath(CrossInSquare)},
        {Lightning,           createPath(Lightning)},
        {Plus,                createPath(Plus)},
        {Minus,               createPath(Minus)},
        {BrokenSquare,        createPath(BrokenSquare)},
        {BrokenDiamond,       createPath(BrokenDiamond)},
        {Bookmark,            createPath(Bookmark)},
        {StarInCircle,        createPath(StarInCircle)},
        {WindMill,            createPath(WindMill)},
        {Tree,                createPath(Tree)},
        {CrossedCircles,      createPath(CrossedCircles)},
        {Flag,                createPath(Flag)}
    };
QHash<GPShape::ShapeType, const char*> GPShape::mShapeNamesMap
    {
        {Circle,              QT_TRANSLATE_NOOP("Shape", "Circle")},
        {Square,              QT_TRANSLATE_NOOP("Shape", "Square")},
        {Diamond,             QT_TRANSLATE_NOOP("Shape", "Diamond")},
        {Triangle,            QT_TRANSLATE_NOOP("Shape", "Triangle")},
        {Pentagon,            QT_TRANSLATE_NOOP("Shape", "Pentagon")},
        {InvertedTriangle,    QT_TRANSLATE_NOOP("Shape", "Inverted triangle")},
        {RightTriangle,       QT_TRANSLATE_NOOP("Shape", "Right triangle")},
        {Ellipse,             QT_TRANSLATE_NOOP("Shape", "Ellipse")},
        {PentaStar,           QT_TRANSLATE_NOOP("Shape", "Five pointed star")},
        {Rectangle,           QT_TRANSLATE_NOOP("Shape", "Rectangle")},
        {Trapezoid,           QT_TRANSLATE_NOOP("Shape", "Trapezoid")},
        {InvertedTrapezoid,   QT_TRANSLATE_NOOP("Shape", "Inverted trapezoid")},
        {Parallelogramm,      QT_TRANSLATE_NOOP("Shape", "Parallelogramm")},
        {Heptagon,            QT_TRANSLATE_NOOP("Shape", "Heptagon")},
        {HeptaStar,           QT_TRANSLATE_NOOP("Shape", "Seven pointed star")},
        {HalfCircle,          QT_TRANSLATE_NOOP("Shape", "Half circle")},
        {DecaStar,            QT_TRANSLATE_NOOP("Shape", "Ten pointed star")},
        {CircleSector,        QT_TRANSLATE_NOOP("Shape", "Circle sector")},
        {Cube,                QT_TRANSLATE_NOOP("Shape", "Cube")},
        {Hourglass,           QT_TRANSLATE_NOOP("Shape", "Hourglass")},
        {Cloud,               QT_TRANSLATE_NOOP("Shape", "Cloud")},
        {CrossInSquare,       QT_TRANSLATE_NOOP("Shape", "Cross in square")},
        {Lightning,           QT_TRANSLATE_NOOP("Shape", "Lightning")},
        {Plus,                QT_TRANSLATE_NOOP("Shape", "Plus")},
        {Minus,               QT_TRANSLATE_NOOP("Shape", "Minus")},
        {BrokenSquare,        QT_TRANSLATE_NOOP("Shape", "Broken square")},
        {BrokenDiamond,       QT_TRANSLATE_NOOP("Shape", "Broken diamond")},
        {Bookmark,            QT_TRANSLATE_NOOP("Shape", "Bookmark")},
        {StarInCircle,        QT_TRANSLATE_NOOP("Shape", "Star in circle")},
        {WindMill,            QT_TRANSLATE_NOOP("Shape", "Wind mill")},
        {Tree,                QT_TRANSLATE_NOOP("Shape", "Tree")},
        {CrossedCircles,      QT_TRANSLATE_NOOP("Shape", "Crossed circles")},
        {Flag,                QT_TRANSLATE_NOOP("Shape", "Flag")}
    };
QHash<QByteArray, GPShape::ShapeType> GPShape::mReShapeMap
    {
     {stringifyPath(createPath(Circle)), Circle},
     {stringifyPath(createPath(Square)), Square},
     {stringifyPath(createPath(Diamond)), Diamond},
     {stringifyPath(createPath(Triangle)), Triangle},
     {stringifyPath(createPath(Pentagon)), Pentagon},
     {stringifyPath(createPath(InvertedTriangle)), InvertedTriangle},
     {stringifyPath(createPath(RightTriangle)), RightTriangle},
     {stringifyPath(createPath(Ellipse)), Ellipse},
     {stringifyPath(createPath(PentaStar)), PentaStar},
     {stringifyPath(createPath(Rectangle)), Rectangle},
     {stringifyPath(createPath(Trapezoid)), Trapezoid},
     {stringifyPath(createPath(InvertedTrapezoid)), InvertedTrapezoid},
     {stringifyPath(createPath(Parallelogramm)), Parallelogramm},
     {stringifyPath(createPath(Heptagon)), Heptagon},
     {stringifyPath(createPath(HeptaStar)), HeptaStar},
     {stringifyPath(createPath(HalfCircle)), HalfCircle},
     {stringifyPath(createPath(DecaStar)), DecaStar},
     {stringifyPath(createPath(CircleSector)), CircleSector},
     {stringifyPath(createPath(Cube)), Cube},
     {stringifyPath(createPath(Hourglass)), Hourglass},
     {stringifyPath(createPath(Cloud)), Cloud},
     {stringifyPath(createPath(CrossInSquare)), CrossInSquare},
     {stringifyPath(createPath(Lightning)), Lightning},
     {stringifyPath(createPath(Plus)), Plus},
     {stringifyPath(createPath(Minus)), Minus},
     {stringifyPath(createPath(BrokenSquare)), BrokenSquare},
     {stringifyPath(createPath(BrokenDiamond)), BrokenDiamond},
     {stringifyPath(createPath(Bookmark)), Bookmark},
     {stringifyPath(createPath(StarInCircle)), StarInCircle},
     {stringifyPath(createPath(WindMill)), WindMill},
     {stringifyPath(createPath(Tree)), Tree},
     {stringifyPath(createPath(CrossedCircles)), CrossedCircles},
     {stringifyPath(createPath(Flag)), Flag},
     };

QString GPShape::ToString(ShapeType type)
{
  return QCoreApplication::translate("Shape", mShapeNamesMap.value(type, ""));
}

QString GPShape::ToString()
{
  return ToString(Shape);
}

GPShape::GPShape(ShapeType type)
  : Shape(type)
  , SizeX(1)
  , SizeY(1)
{
}

GPShape::ShapeType GPShape::GetShape() const
{
  return Shape;
}

void GPShape::SetShape(ShapeType newShape)
{
  Shape = newShape;
}

QPainterPath GPShape::GetPath() const
{
  return GetPath(Shape);
}

QPainterPath GPShape::GetPath(ShapeType shape)
{
  return mShapeMap.value(shape, QPainterPath());
}

GPShape::ShapeType GPShape::ToShape(const QPainterPath &path)
{
  return mReShapeMap.value(stringifyPath(path), PathShape);
}

double GPShape::GetSizeX() const
{
  return SizeX;
}

void GPShape::SetSizeX(double newSizeX)
{
  SizeX = newSizeX;
}

double GPShape::GetSizeY() const
{
  return SizeY;
}

void GPShape::SetSizeY(double newSizeY)
{
  SizeY = newSizeY;
}
