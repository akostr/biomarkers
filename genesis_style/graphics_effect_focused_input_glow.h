#pragma once

#include <QGraphicsDropShadowEffect>
#include <QWidget>

class GraphicsEffectFocusedInputGlow : public QGraphicsDropShadowEffect
{
public:
  explicit GraphicsEffectFocusedInputGlow(QWidget* inputWidget);
  ~GraphicsEffectFocusedInputGlow();

private:
  virtual bool eventFilter(QObject* watched, QEvent* event) override;

  virtual QRectF boundingRectFor(const QRectF & rect) const override;
  virtual void draw(QPainter* painter) override;
};
