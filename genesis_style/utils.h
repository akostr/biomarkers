#pragma once

#include <QStyle>

class QMenu;

namespace Style
{
  namespace Utils
  {
    int DefaultDPIX();
    int DefaultDPIY();
    int DefaultDPI();
    qreal DPIScaled(qreal value);    
  }

  namespace GuiUtils
  {
    void AddMenuSection(QMenu* menu, const QString& icon, const QString& text);
  }
}
