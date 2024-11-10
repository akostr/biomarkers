#ifndef ERROR_NOTIFICATION_H
#define ERROR_NOTIFICATION_H

#include <ui/widgets/notifications/notification_widget.h>

namespace Ui {
class ErrorNotification;
}

class ErrorNotification : public NotificationWidget
{
  Q_OBJECT

public:
  explicit ErrorNotification(Notification* notification, QWidget *parent = nullptr);
  ~ErrorNotification();

private:
  Ui::ErrorNotification *ui;

private:
  void setupUi();
  void updateData();

  // NotificationWidget interface
protected:
  void onNotificationUpdate() override;
};

#endif // ERROR_NOTIFICATION_H
