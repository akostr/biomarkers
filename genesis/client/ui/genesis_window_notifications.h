#pragma once

#include "../logic/notification.h"

#include <QObject>
#include <QWidget>
#include <QFrame>
#include <QScrollArea>
#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QStackedLayout>

////////////////////////////////////////////////////
//// Fwds
class NotificationPresenter;

////////////////////////////////////////////////////
//// Notifications presenter
class NotificationWidget;
class NotificationPresenter : public QWidget
{
  Q_OBJECT
public:
  NotificationPresenter(QWidget* parent);
  ~NotificationPresenter();

  virtual void resizeEvent(QResizeEvent* event) override;
  virtual void moveEvent(QMoveEvent* event) override;
  virtual void showEvent(QShowEvent *event) override;
  virtual void paintEvent(QPaintEvent *event) override;

public slots:
  void UpdateRegion();

private:
  void SetupUi();
  void addNotificationToQueue(Notification* notification);
  void processNotificationQueue();
  NotificationWidget* makeNotification(Notification* notification);
  void processWidgetsOpacity();

private slots:
  void onNotificationAdded(Notification* notification);
  void onNotificationWidgetClosed();
  void requestRegionUpdate();

private:
  //// Ui
  QPointer<QVBoxLayout>               Layout;
  QPointer<QVBoxLayout>               LayoutOuterVertical;
  QPointer<QHBoxLayout>               LayoutOuterHorizontal;
  QPointer<QVBoxLayout>               LayoutViews;
  QList<Notification*>                mNotificationsQueue;
  bool                                mRegionUpdateRequested = false;
  QRegion                             mDbgOpacityRegion;
  QList<NotificationWidget*>          mNotificationWidgets;
  QTimer*                             mOpacityUpdateTimer;
  int                                 mOpacityUpdateTimerInterval;
};
