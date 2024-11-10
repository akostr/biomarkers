#include "genesis_window_notifications.h"
#include "../genesis_style/style.h"
#include "qgraphicseffect.h"

#include <ui/widgets/notifications/error_notification.h>
#include <ui/widgets/notifications/headerless_success_notification.h>
#include <ui/widgets/notifications/success_notification.h>
#include <ui/widgets/notifications/info_notification.h>
#include <ui/widgets/notifications/warning_notification.h>
#include <ui/widgets/notifications/multi_notification.h>

#include <QHBoxLayout>
#include <QLabel>
#include <QStyle>
#include <QTimer>
#include <QEvent>
#include <QNetworkReply>

namespace Details
{
  static const int NotificationPresenter_MaximumViewsCount__ = 7;
}

////////////////////////////////////////////////////
//// Notifications presenter
NotificationPresenter::NotificationPresenter(QWidget* parent)
  : QWidget(parent)
  , mOpacityUpdateTimer(new QTimer(this))
  , mOpacityUpdateTimerInterval(10)
{
  //// Create
  SetupUi();
  mOpacityUpdateTimer->setInterval(mOpacityUpdateTimerInterval);
  connect(mOpacityUpdateTimer, &QTimer::timeout, this, &NotificationPresenter::processWidgetsOpacity);
//  mOpacityUpdateTimer->start();

  //// Connect to all notifications
  connect(NotificationManager::Get(), &NotificationManager::notificationAdded, this, &NotificationPresenter::onNotificationAdded);
}

NotificationPresenter::~NotificationPresenter()
{
  for(auto& w : mNotificationWidgets)
  {
    disconnect(w, nullptr, this, nullptr);
  }
  mNotificationWidgets.clear();
  mNotificationsQueue.clear();
}

void NotificationPresenter::SetupUi()
{
  setStyleSheet(Style::Genesis::GetUiStyle());

  //// Layout
  Layout = new QVBoxLayout(this);
  Layout->setContentsMargins(0, 0, 0, 0);

  LayoutOuterVertical = new QVBoxLayout;
  LayoutOuterVertical->setContentsMargins(0, 0, 0, 0);
  Layout->addStretch();
  Layout->addLayout(LayoutOuterVertical);

  LayoutOuterHorizontal = new QHBoxLayout;
  LayoutOuterHorizontal->setContentsMargins(0, 0, 0, 0);
  LayoutOuterVertical->addStretch();
  LayoutOuterVertical->addLayout(LayoutOuterHorizontal);

  LayoutViews = new QVBoxLayout;
  LayoutViews->setContentsMargins(Style::Scale(40), Style::Scale(40), Style::Scale(40), Style::Scale(40));
  LayoutViews->setSpacing(Style::Scale(16));
  LayoutViews->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Minimum,QSizePolicy::Expanding));
  LayoutViews->setStretch(0,1);
  LayoutOuterHorizontal->addStretch();
  LayoutOuterHorizontal->addLayout(LayoutViews);
}

void NotificationPresenter::onNotificationAdded(Notification *notification)
{
  if(!notification)
    return;
  addNotificationToQueue(notification);
}

void NotificationPresenter::onNotificationWidgetClosed()
{
  auto w = (NotificationWidget*)(sender());
  mNotificationWidgets.removeOne(w);
  processNotificationQueue();
}

void NotificationPresenter::requestRegionUpdate()
{
  if(mRegionUpdateRequested)
    return;
  mRegionUpdateRequested = true;
  QTimer::singleShot(0, this, &NotificationPresenter::UpdateRegion);
}

void NotificationPresenter::addNotificationToQueue(Notification *notification)
{
  if(!notification)
    return;
  mNotificationsQueue << notification;
  processNotificationQueue();
}

void NotificationPresenter::processNotificationQueue()
{
  if(!mNotificationsQueue.isEmpty())
  {
    int availablePositions =  Details::NotificationPresenter_MaximumViewsCount__ - mNotificationWidgets.count();
    if(availablePositions > 0)
    {
      for(int i = 0; i < availablePositions; i++)
      {
        NotificationWidget* w = nullptr;
        while(!w && !mNotificationsQueue.isEmpty())
        {
          w = makeNotification(mNotificationsQueue.takeFirst());
        }
        if(w)
        {
          LayoutViews->addWidget(w);
          connect(w, &NotificationWidget::destroyed, this, &NotificationPresenter::onNotificationWidgetClosed);
          connect(w, &NotificationWidget::updateRequired, this, &NotificationPresenter::requestRegionUpdate);
          mNotificationWidgets << w;
        }
        if(mNotificationsQueue.isEmpty())
          break;
      }
    }
  }
  bool anyWithDecay = false;
  for(auto& w : mNotificationWidgets)
  {
    if(w->getDecayInterval() != -1)
    {
      anyWithDecay = true;
      break;
    }
  }
  if(!anyWithDecay && mOpacityUpdateTimer->isActive())
    mOpacityUpdateTimer->stop();
  else if(anyWithDecay && !mOpacityUpdateTimer->isActive())
    mOpacityUpdateTimer->start();
  UpdateRegion();
}

NotificationWidget *NotificationPresenter::makeNotification(Notification *notification)
{
  if(!notification)
    return nullptr;
  NotificationWidget* newNotification = nullptr;
  auto echoNotification = [&notification](QString status)
  {
    QString message;
    if(notification->GetHeader().isEmpty())
       message = notification->GetText();
    else
       message =QString("%1: %2").arg(notification->GetHeader()).arg(notification->GetText());
    auto errData = notification->GetData("network_error");
    if(errData.isValid() && !errData.isNull() && errData.value<QNetworkReply::NetworkError>() != QNetworkReply::NoError)
      qDebug() << status << message << "; NETWORK ERROR: " << errData.value<QNetworkReply::NetworkError>();
    else
      qDebug() << status << message;
  };
  switch(notification->GetStatus())
  {
  case Notification::StatusSuccess:
    newNotification = new SuccessNotification(notification, this);
    echoNotification("LOG_SUCCESS");
    break;
  case Notification::StatusInformation:
    newNotification = new InfoNotification(notification, this);
    echoNotification("LOG_INFO");
    break;
  case Notification::StatusHeaderlessInformation:
    newNotification = new HeaderlessSuccessNotification(notification, this);
    echoNotification("LOG_HEADERLESS");
    break;
  case Notification::StatusError:
    newNotification = new ErrorNotification(notification, this);
    echoNotification("LOG_ERROR");
    break;
  case Notification::StatusWarning:
    newNotification = new WarningNotification(notification, this);
    echoNotification("LOG_WARNING");
    break;
  case Notification::StatusViewChildren:
    newNotification = new MultiNotification(notification, this);
    break;
  default:
    qDebug() << "invalid status for making notification";
    break;
  }
  return newNotification;
}

void NotificationPresenter::processWidgetsOpacity()
{
  for(auto& w : mNotificationWidgets)//can be improved
  {
    w->processOpacity(mOpacityUpdateTimerInterval);
  }
}

void NotificationPresenter::resizeEvent(QResizeEvent* event)
{
  QWidget::resizeEvent(event);
  requestRegionUpdate();
}

void NotificationPresenter::moveEvent(QMoveEvent* event)
{
  QWidget::moveEvent(event);
//  requestRegionUpdate();
}

void NotificationPresenter::showEvent(QShowEvent *event)
{
  QWidget::showEvent(event);
  requestRegionUpdate();
}

void NotificationPresenter::paintEvent(QPaintEvent *event)
{
  QWidget::paintEvent(event);
}

void NotificationPresenter::UpdateRegion()
{
  QRegion region;
  for(auto& w : mNotificationWidgets)//can be improved
  {
    region += w->geometry();
  }
  if(region.isEmpty())
  {
    region += QRect(0,0,1,1);
  }
  setMask(region);
  mDbgOpacityRegion = region;
  mRegionUpdateRequested = false;
}
