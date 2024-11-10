#include "switch_widget.h"

#include <QMouseEvent>
#include <QPainter>
#include <QPropertyAnimation>
#include <QApplication>

Switch::Switch(QWidget *parent) 
  : QCheckBox(parent)
  , Opacity(0.000)
  , X(0)
  , Y(0)
  , Height(12)
  , Margin(2)
  , Thumb("#d5d5d5")
  , Brush("#dae2e7")
  , BrushOff("#dae2e7")
  , BrushBg(Qt::black)
  , BrushBgOff(Qt::black)
  , Animation(new QPropertyAnimation(this, "offset", this))
{
  setOffset(Height / 2);
  Y = Height / 2;
}

void Switch::paintEvent(QPaintEvent* /*e*/) 
{
  Thumb = isChecked() ? Brush : BrushOff;
  QPainter p(this);
  p.setPen(Qt::NoPen);
  p.setClipping(false);
  if (isEnabled()) 
  {
    p.setBrush(isChecked() ? BrushBg : BrushBgOff);
    p.setOpacity(isChecked() ? 0.5 : 0.32);
    p.setRenderHint(QPainter::Antialiasing, true);
    p.drawRoundedRect(QRect(Margin, Margin, width() - 2 * Margin, height() - 2 * Margin), 8.0, 8.0);
    p.setBrush(Thumb);
    p.setOpacity(1.0);
    p.drawEllipse(QRectF(offset() - (Height / 2), Y - (Height / 2), height(), height()));
  }
  else 
  {
    p.setBrush(isChecked() ? BrushBg : BrushBgOff);
    p.setOpacity(0.12);
    p.drawRoundedRect(QRect(Margin, Margin, width() - 2 * Margin, height() - 2 * Margin), 8.0, 8.0);
    p.setOpacity(0.50);
    p.setBrush(Thumb);
    p.drawEllipse(QRectF(offset() - (Height / 2), Y - (Height / 2), height(), height()));
  }
}

void Switch::mousePressEvent(QMouseEvent*)
{
}

void Switch::mouseReleaseEvent(QMouseEvent* e) 
{
  if (e->button() & Qt::LeftButton) 
    setChecked(!isChecked());
  QAbstractButton::mouseReleaseEvent(e);
}

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
void Switch::enterEvent(QEvent* e)
#else
void Switch::enterEvent(QEnterEvent* e)
#endif
{
  setCursor(Qt::PointingHandCursor);
  QAbstractButton::enterEvent(e);
}

void Switch::resizeEvent(QResizeEvent *event)
{
  QAbstractButton::resizeEvent(event);
  if (isChecked())
  {
    if (offset() != width() - Height)
    {
      setOffset(width() - Height);
    }
  }
  else
  {
    if (offset() != height() / 2)
    {
      setOffset(height() / 2);
    }
  }
}

void Switch::setChecked(bool checked)
{
  if (isChecked() != checked)
  {
    if (isVisible() && isVisibleTo(QApplication::activeWindow()))
    {
      if (checked)
      {
        Animation->setStartValue(Height / 2);
        Animation->setEndValue(width() - Height);
        Animation->setDuration(120);
        Animation->start();
      }
      else
      {
        Animation->setStartValue(width() - Height);
        Animation->setEndValue(Height / 2);
        Animation->setDuration(120);
        Animation->start();
      }
    }
    else
    {
      if (checked)
        X = width() - Height;
      else
        X = Height / 2;
    }
    QCheckBox::setChecked(checked);
  }
}

QSize Switch::sizeHint() const 
{
  return QSize(2 * (Height + Margin), Height + 2 * Margin);
}

QBrush Switch::brush() const
{
  return Brush;
}

void Switch::setBrush(const QBrush& b)
{
  Brush = b;
  update();
}

void Switch::setBrushOff(const QBrush& b)
{
  BrushOff = b;
  update();
}

void Switch::setBrushBg(const QBrush& b)
{
  BrushBg = b;
  update();
}

void Switch::setBrushBgOff(const QBrush& b)
{
  BrushBgOff = b;
  update();
}

int Switch::offset() const
{
  return X;
}

void Switch::setOffset(int o)
{
  X = o;
  update();
}
