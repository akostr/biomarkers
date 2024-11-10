#include "utils.h"

#include <QGuiApplication>
#include <QScreen>
#include <QtMath>

#include <QHBoxLayout>
#include <QLabel>
#include <QMenu>
#include <QWidgetAction>

namespace Style
{
  namespace Utils
  {
    int DefaultDPIX()
    {
      if (QCoreApplication::instance()->testAttribute(Qt::AA_Use96Dpi))
        return 96;

      if (const QScreen *screen = QGuiApplication::primaryScreen())
        return qRound(screen->logicalDotsPerInchX());

      return 100;
    }

    int DefaultDPIY()
    {
      if (QCoreApplication::instance()->testAttribute(Qt::AA_Use96Dpi))
        return 96;

      if (const QScreen *screen = QGuiApplication::primaryScreen())
        return qRound(screen->logicalDotsPerInchY());

      return 100;
    }

    int DefaultDPI()
    {
      return DefaultDPIY();
    }

    qreal DPIScaled(qreal value)
    {
#ifdef Q_OS_MAC
      // On mac the DPI is always 72 so we should not scale it
      return value;
#else
      static const qreal scale = qreal(DefaultDPIX()) / 96.0;
      return value * scale;
#endif
    }
  }

  namespace GuiUtils
  {
    void AddMenuSection(QMenu *menu, const QString &icon, const QString& text)
    {
      auto action = new QWidgetAction(menu);

      auto widget = new QWidget;
      auto layout = new QHBoxLayout(widget);
      layout->setContentsMargins(6,4,4,4);
      layout->setSpacing(6);

      QIcon ico(icon);
      auto pixmap = new QLabel(widget);
      pixmap->setPixmap(ico.pixmap(16, 16));
      layout->addWidget(pixmap);

      auto label = new QLabel(text, widget);
      auto f = label->font();
      f.setBold(true);
      label->setFont(f);
      layout->addWidget(label);

      layout->addStretch(1);

      action->setDefaultWidget(widget);
      menu->addAction(action);
    }
  }
}
