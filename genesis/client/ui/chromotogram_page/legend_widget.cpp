#include "legend_widget.h"

// #include <graphicsplot/graphicsplot_extended.h>
#include <ui/plots/gp_items/gpshape.h>
#include <QGuiApplication>
#include <QPainter>
#include <genesis_style/style.h>

LegendWidget::LegendWidget(QWidget* parent)
  : QWidget(parent),
  mShape(GPShape::GetPath(GPShape::Square)),
  mFont(qApp->font()),
  mPen(Qt::NoPen),
  mFontMetrics(QFontMetricsF(mFont)),
  mSpacing(5)
{
  QFont font(Style::GetSASSValue("fontFaceNormal"),
             -1,
             Style::GetSASSValue("fontWeight").toInt());
  font.setPixelSize(Style::GetSASSValue("fontSizeRegularTextScalableFont").toInt());

  setFont(font);
  updateShapeSize();
}

void LegendWidget::setShapeBrush(const QBrush& brush)
{
  mBrush = brush;
}

void LegendWidget::setShapePen(const QPen& pen)
{
  mPen = pen;
}

void LegendWidget::setShape(int shape)
{
  mShape = GPShape::GetPath((GPShape::ShapeType)shape);
}

void LegendWidget::setShape(const QPainterPath &shape)
{
  mShape = shape;
}

void LegendWidget::setText(const QString& text)
{
  mText = text;
}

void LegendWidget::setFont(const QFont& font)
{
  mFont = font;
  mFontMetrics = QFontMetricsF(mFont);
  updateShapeSize();
}

void LegendWidget::setSpacing(int spacing)
{
  mSpacing = spacing;
}

int LegendWidget::spacing()
{
  return mSpacing;
}

void LegendWidget::paintEvent(QPaintEvent* e)
{
  QPainter p(this);
  p.setRenderHint(QPainter::Antialiasing);
  p.setFont(mFont);
  p.save();
  p.setPen(mPen);
  p.setBrush(mBrush);
  p.translate(0, (mFontMetrics.height() - mFontMetrics.ascent()) / 2.0);
  p.scale(mSize.width(), mSize.width());
  p.drawPath(mShape);
  p.restore();
  p.setPen(Qt::black);
  p.drawText(QPointF(mSpacing + mSize.width(), mFontMetrics.height() - mFontMetrics.descent()), mText);
}

QSize LegendWidget::sizeHint() const
{
  QSizeF size(0, mFontMetrics.height());
  size.setWidth(mSize.width() + mSpacing + mFontMetrics.boundingRect(mText).width() + 2);//2 for borders
  return size.toSize();
}

void LegendWidget::updateShapeSize()
{
  auto height = mFontMetrics.ascent();
  mSize = {height, height};
}
