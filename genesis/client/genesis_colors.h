#ifndef GENESIS_COLORS_H
#define GENESIS_COLORS_H
#include <QColor>
namespace Colors
{
  namespace Markup
  {
  QColor stepperFilledBrush{0, 120, 210};
  QColor stepperCompletedBrush{34, 195, 142};
  QColor stepperEmptyBrush{164, 174, 179};
  QColor stepperHighlightedFilledBrush{stepperFilledBrush.lighter(200)};
  QColor stepperHighlightedEmptyBrush{"lightblue"};
  QColor stepperEmptyTextPen{Qt::white};
  QColor stepperFilledTextPen{Qt::white};
  QColor stepperLabelFilledTextPen{0, 32, 51, 255};
  QColor stepperLabelEmptyTextPen{0, 32, 51, 153};
  }
}

#endif // GENESIS_COLORS_H
