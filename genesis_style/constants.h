#pragma once

#include <QColor>
#include <QPalette>

namespace Style
{
  namespace Constants
  {
    //// Contants
    enum Contant
    {
      ConstantPagePadding = 8,
    };

    //// Get color code
    QString     GetColorName(QPalette::ColorGroup group, QPalette::ColorRole role); 

    //// Get color object
    QColor      GetColor(QPalette::ColorGroup group, QPalette::ColorRole role);

    //// Get default application palette
    QPalette    GetPalette();
  }
}
