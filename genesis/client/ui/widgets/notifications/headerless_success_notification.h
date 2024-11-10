#ifndef HEADERLESS_SUCCESS_NOTIFICATION_H
#define HEADERLESS_SUCCESS_NOTIFICATION_H

#include <ui/widgets/notifications/notification_widget.h>

namespace Ui {
class HeaderlessInfoNotification;
}

class HeaderlessSuccessNotification : public NotificationWidget
{
  Q_OBJECT

public:
  explicit HeaderlessSuccessNotification(Notification* notification, QWidget *parent = nullptr);
  ~HeaderlessSuccessNotification();

private:
  Ui::HeaderlessInfoNotification *ui;

private:
  void setupUi();
  void updateData();

  // NotificationWidget interface
protected:
  void onNotificationUpdate() override;
};

#endif // HEADERLESS_SUCCESS_NOTIFICATION_H
