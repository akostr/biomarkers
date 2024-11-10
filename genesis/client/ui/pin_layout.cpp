#include "pin_layout.h"
#include "qlabel.h"

#include <genesis_style/style.h>

#include <QEvent>
#include <QTimer>
#include <QRegion>
#include <QScrollBar>

PinWidget::PinWidget(QWidget* parent)
  : QWidget(parent)
{
//  Container       = new QWidget(this);
  setObjectName("Container");
  setStyleSheet(Style::ApplySASS("QWidget#Container { background-color: @frameColor; border: 3px solid red;} "));
  ContainerLayout = new QVBoxLayout(this);
  ContainerLayout->setContentsMargins(10, 10, 10, 10);
  Update();
}

void PinWidget::Pin(QWidget* widget)
{
  if (Pinned == widget)
    return;
  if(Pinned)
    Unpin();
  if (widget)
  {
    Pinned = widget;
    PrevParent = Pinned->parentWidget();
    Pinned->setParent(this);
    ContainerLayout->addWidget(Pinned);
  }
  Update();
}

QPointer<QWidget> PinWidget::Unpin()
{
  auto ret = Pinned;
  if (Pinned)
  {
    int index = ContainerLayout->indexOf(Pinned);
    if (index != -1)
    {
      QLayoutItem* item = ContainerLayout->itemAt(index);
      ContainerLayout->removeItem(item);
    }
    Pinned->setParent(PrevParent);
    Pinned = nullptr;
  }
  
  Update();
  return ret;
}

QWidget *PinWidget::GetPinned()
{
  return Pinned;
}

void PinWidget::Update()
{
  updateGeometry();
}

QSize PinWidget::sizeHint() const
{
  if(!Pinned)
    return QSize(0,0);
  return QWidget::sizeHint();
}


QSize PinWidget::minimumSizeHint() const
{
  if(!Pinned)
    return QSize(0,0);
  return QWidget::minimumSizeHint();
}
