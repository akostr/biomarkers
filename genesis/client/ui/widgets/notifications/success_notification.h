#ifndef SUCCESS_NOTIFICATION_H
#define SUCCESS_NOTIFICATION_H

#include <ui/widgets/notifications/notification_widget.h>

namespace Ui {
class SuccessNotification;
}

class SuccessNotification : public NotificationWidget
{
  Q_OBJECT

public:
  explicit SuccessNotification(Notification* notification, QWidget *parent = nullptr);
  ~SuccessNotification();

private:
  Ui::SuccessNotification *ui;

private:
  void setupUi();
  void updateData();

  // NotificationWidget interface
protected:
  void onNotificationUpdate() override;
};

#endif // SUCCESS_NOTIFICATION_H
