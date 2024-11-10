#include "graphics_effect_focused_input_glow.h"

#include <QCheckBox>
#include <QRadioButton>

#include <QEvent>
#include <QColor>
#include <QPainter>

namespace
{
  const double BlurRadius = 3.0;
}

GraphicsEffectFocusedInputGlow::GraphicsEffectFocusedInputGlow(QWidget* inputWidget)
  : QGraphicsDropShadowEffect(inputWidget)
{
  inputWidget->setGraphicsEffect(this);
  inputWidget->installEventFilter(this);

  setOffset(0.0);
  setColor(QColor(qRgba(97, 127, 255, 191)));
  setBlurRadius(BlurRadius);
  setEnabled(inputWidget->hasFocus());
}

GraphicsEffectFocusedInputGlow::~GraphicsEffectFocusedInputGlow()
{
}

bool GraphicsEffectFocusedInputGlow::eventFilter(QObject* watched, QEvent* event)
{
  switch (event->type())
  {
  case QEvent::FocusIn:
    setEnabled(true);
    break;
  case QEvent::FocusOut:
    setEnabled(false);
    break;
  default:
    break;
  }
  return QGraphicsDropShadowEffect::eventFilter(watched, event);
}

QRectF GraphicsEffectFocusedInputGlow::boundingRectFor(const QRectF& rectArg) const
{
  return QGraphicsDropShadowEffect::boundingRectFor(rectArg);
}

void GraphicsEffectFocusedInputGlow::draw(QPainter* painter)
{
  bool clip = false;
  bool draw = true;
  QWidget* widget = qobject_cast<QWidget*>(parent());
  if (auto p = parent())
  {
    QString className = p->metaObject()->className();
    if (className == "QCheckBox")
    {
      clip = true;
    }
    else if (className == "QRadioButton")
    {
      clip = true;
      if (widget && widget->height() > 32)
      {
        draw = false;
      }
    }
  }

  if (draw)
  {
    if (clip)
    {
      QRectF widgetRect = widget->rect();

      // Clip
      QRectF clipRect;
      if (widget->layoutDirection() == Qt::RightToLeft)
        clipRect = QRectF(widgetRect.width() - widgetRect.height() - BlurRadius, -BlurRadius, widgetRect.height() + BlurRadius * 2, widgetRect.height() + BlurRadius * 2);
      else
        clipRect = QRectF(-BlurRadius, -BlurRadius, widgetRect.height() + BlurRadius * 2, widgetRect.height() + BlurRadius * 2);

      painter->setClipRect(clipRect);
      QGraphicsDropShadowEffect::draw(painter);

      // Clip
      painter->setClipRect(widgetRect);

      drawSource(painter);
    }
    else
    {
      QGraphicsDropShadowEffect::draw(painter);
    }
  }
  else
  {
    drawSource(painter);
  }
}