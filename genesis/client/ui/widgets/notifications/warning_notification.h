#ifndef WARNING_NOTIFICATION_H
#define WARNING_NOTIFICATION_H

#include <ui/widgets/notifications/notification_widget.h>

namespace Ui {
class WarningNotification;
}

class WarningNotification : public NotificationWidget
{
  Q_OBJECT

public:
  explicit WarningNotification(Notification* notification, QWidget *parent = nullptr);
  ~WarningNotification();

private:
  Ui::WarningNotification *ui;

private:
  void setupUi();
  void updateData();

  // NotificationWidget interface
protected:
  void onNotificationUpdate() override;
};

#endif // WARNING_NOTIFICATION_H
