#include "editable_legend_widget.h"
#include <ui/chromotogram_page/legend_widget.h>

#include <QHBoxLayout>
#include <QPushButton>

EditableLegendWidget::EditableLegendWidget(QWidget *parent)
  : QWidget{parent},
  mLegendWidget(new LegendWidget(this))
{
  auto layout = new QHBoxLayout(this);
  setLayout(layout);
  layout->addWidget(mLegendWidget);
  layout->setSpacing(mLegendWidget->spacing());
  mEditButton = new QPushButton(this);
  mEditButton->setFlat(true);
  auto icon = QIcon(":/resource/icons/icon_pencil@2x.png");
  if(icon.isNull())
  {
    qDebug() << "can't load pencil icon";
    mEditButton->setText("E");
  }
  else
  {
    mEditButton->setIcon(icon);
  }
  layout->addWidget(mEditButton);
  //@TODO: check it works:
  connect(mEditButton, &QPushButton::clicked, this, &EditableLegendWidget::editingRequested);
}

void EditableLegendWidget::setShapeBrush(const QBrush &brush)
{
  mLegendWidget->setShapeBrush(brush);
}

void EditableLegendWidget::setShapePen(const QPen &pen)
{
  mLegendWidget->setShapePen(pen);
}

void EditableLegendWidget::setShape(int shape)
{
  mLegendWidget->setShape(shape);
}

void EditableLegendWidget::setShape(const QPainterPath &shape)
{
  mLegendWidget->setShape(shape);
}

void EditableLegendWidget::setText(const QString &text)
{
  mLegendWidget->setText(text);
}

void EditableLegendWidget::setFont(const QFont &font)
{
  mLegendWidget->setFont(font);
}

void EditableLegendWidget::setSpacing(int spacing)
{
  mLegendWidget->setSpacing(spacing);
}
