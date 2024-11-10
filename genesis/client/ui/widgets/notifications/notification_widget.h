#ifndef NOTIFICATIONWIDGET_H
#define NOTIFICATIONWIDGET_H

#include <QWidget>
#include <logic/notification.h>

class QGraphicsOpacityEffect;
class NotificationWidget : public QWidget
{
  Q_OBJECT
public:
  explicit NotificationWidget(Notification* notification, QWidget *parent = nullptr);
  virtual void onNotificationClosed();
  void processOpacity(int delayMs);
  int getDecayInterval();

protected:
  QWidget* mContent;
  Notification* mNotification;
  QGraphicsOpacityEffect* mOpacityEffect;
  bool mDecayingOn = false;
  int mDecayInterval = -1;
  bool mIsFinished = false;

protected:
  virtual void onNotificationUpdate();
  bool event(QEvent *event) override;

signals:
  void updateRequired();

  // QWidget interface
protected:
  void showEvent(QShowEvent *event) override;
  void hideEvent(QHideEvent *event) override;
  void mouseMoveEvent(QMouseEvent *event) override;
  void enterEvent(QEnterEvent *event) override;
  void leaveEvent(QEvent *event) override;
};

#endif // NOTIFICATIONWIDGET_H
