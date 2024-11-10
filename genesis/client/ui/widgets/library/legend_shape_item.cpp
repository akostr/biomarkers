#include "legend_shape_item.h"
#include "ui_legend_shape_item.h"
#include <QToolTip>
#include <QUuid>

LegendShapeItem::LegendShapeItem(QWidget* parent)
  : QWidget(parent)
{
  setupUi();
  connectSignals();
}

LegendShapeItem::~LegendShapeItem()
{
  delete mUi;
}

void LegendShapeItem::setShape(const QPainterPath& shape)
{
  mUi->shapeWidget->setShape(shape);
}

void LegendShapeItem::setShapePen(const QPen& pen)
{
  mUi->shapeWidget->setShapePen(pen);
}

void LegendShapeItem::setShapeBrush(const QBrush& brush)
{
  mUi->shapeWidget->setShapeBrush(brush);
}

void LegendShapeItem::setText(const QString& text)
{
  mUi->shapeLabel->setText(text);
  // qDebug() << "setTitle" << property("uid").toUuid() << text;
}

void LegendShapeItem::setupUi()
{
  mUi = new Ui::LegendShapeItem();
  mUi->setupUi(this);
  connect(mUi->shapeLabel, &QLabel::linkHovered, this, [this]()
          {
            QToolTip::showText(QCursor::pos(), toolTip(), this);
          });
}

void LegendShapeItem::connectSignals()
{
  connect(mUi->editButton, &QPushButton::clicked, this, &LegendShapeItem::editClicked);
}

bool LegendShapeItem::event(QEvent *event)
{
  if(event->type() == QEvent::ToolTip)
  {
    event->ignore();
    return true;
  }
  return QWidget::event(event);
}
