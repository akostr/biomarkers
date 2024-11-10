#include "notification_widget.h"
#include <QEvent>
#include <QVBoxLayout>
#include <QTimer>
#include <QGraphicsOpacityEffect>
#include <genesis_style/style.h>

NotificationWidget::NotificationWidget(Notification* notification, QWidget *parent)
  : QWidget{parent}
  , mNotification(notification)
  , mOpacityEffect(new QGraphicsOpacityEffect(this))
{
  setAttribute(Qt::WA_DeleteOnClose);
  if(mNotification)
  {
    connect(mNotification, &Notification::Changed, this, &NotificationWidget::onNotificationUpdate);
    connect(mNotification, &Notification::Close, this, &NotificationWidget::onNotificationClosed);
    mDecayInterval = mNotification->DestroyDelay();
  }
  setFixedWidth(Style::Scale(500));
  setStyleSheet("NotificationWidget {background:transparent; border: none;}");
  setContentsMargins(0,0,0,0);
  mContent = new QWidget();
  mContent->setContentsMargins(0,0,0,0);
  auto l = new QVBoxLayout();
  l->setContentsMargins(0,0,0,0);
  this->setLayout(new QVBoxLayout());
  this->layout()->setContentsMargins(0,0,0,0);
  layout()->addWidget(mContent);
  mOpacityEffect->setOpacity(1);
  mContent->setGraphicsEffect(mOpacityEffect);
}

void NotificationWidget::onNotificationUpdate()
{

}

void NotificationWidget::onNotificationClosed()
{
  if(mNotification)
  {
    mNotification->SetNotification(Notification::StatusDelete, "", "", QNetworkReply::NoError);
    mNotification = nullptr;
  }
  mIsFinished = true;
  close();
}


bool NotificationWidget::event(QEvent *event)
{
  bool result = QWidget::event(event);
  switch (event->type())
  {
  case QEvent::ContentsRectChange:
  case QEvent::Expose:
  case QEvent::Hide:
  case QEvent::HideToParent:
  case QEvent::LayoutRequest:
  case QEvent::Move:
  case QEvent::Resize:
  case QEvent::Show:
  case QEvent::ShowToParent:
  {
    emit updateRequired();
  }
    break;
  default:
    break;
  }
  return result;
}

void NotificationWidget::processOpacity(int delayMs)
{
  if(mIsFinished || !mDecayingOn || !mNotification)
    return;
  double interval = mNotification->GetDecoyDelay();
  double delta = delayMs;
  double op = mOpacityEffect->opacity();
  double d = 1.0 / (interval / delta);
  mOpacityEffect->setOpacity(op - d);
}

int NotificationWidget::getDecayInterval()
{
  return mDecayInterval;
}

void NotificationWidget::showEvent(QShowEvent *event)
{
  QWidget::showEvent(event);
  auto interval = getDecayInterval();
  if(interval != -1 && mNotification)
  {
    QTimer::singleShot(interval, this, [this]()
    {
      if(!mDecayingOn)
      {
        mNotification->StartDestroyTimer();
        mDecayingOn = true;
      }
    });
  }
}

void NotificationWidget::hideEvent(QHideEvent *event)
{
  QWidget::hideEvent(event);
}

void NotificationWidget::mouseMoveEvent(QMouseEvent *event)
{
  QWidget::mouseMoveEvent(event);
}

void NotificationWidget::enterEvent(QEnterEvent *event)
{
  QWidget::enterEvent(event);
  if(!mNotification)
    return;
  if(mDecayingOn)
  {
    mNotification->StopDestroyTimer();
    mDecayingOn = false;
    mOpacityEffect->setOpacity(1);
  }
}

void NotificationWidget::leaveEvent(QEvent *event)
{
  QWidget::leaveEvent(event);
  if(!mNotification || mDecayInterval == -1)
    return;
  mDecayingOn = true;
  mNotification->StartDestroyTimer();
}
