#include "identification_legend_plate.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QPainter>

IdentificationLegendPlate::IdentificationLegendPlate(const QVariant& color, const QString& text, QWidget *parent)
  : QWidget{parent}
{
  auto l = new QHBoxLayout(this);
  setLayout(l);
  mColorLabel = new QLabel(this);
  l->addWidget(mColorLabel);
  mTextLabel = new QLabel(text, this);
  l->addWidget(mTextLabel);

  setColor(color);
}

void IdentificationLegendPlate::setColor(const QVariant &color)
{
  QPixmap colorPixmap(12, 12);
  QPainter p(&colorPixmap);
  // p.setPen(QPen(QBrush(Qt::black), 1));
  p.setPen(Qt::NoPen);
  p.setBrush(color.value<QColor>());
  p.drawRect(colorPixmap.rect());
  mColorLabel->setPixmap(colorPixmap);
}

void IdentificationLegendPlate::setText(const QString &text)
{
  mTextLabel->setText(text);
}
