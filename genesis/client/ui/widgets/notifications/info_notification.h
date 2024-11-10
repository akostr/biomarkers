#ifndef INFO_NOTIFICATION_H
#define INFO_NOTIFICATION_H

#include <ui/widgets/notifications/notification_widget.h>

namespace Ui {
class InfoNotification;
}

class InfoNotification : public NotificationWidget
{
  Q_OBJECT

public:
  explicit InfoNotification(Notification* notification, QWidget *parent = nullptr);
  ~InfoNotification();

private:
  Ui::InfoNotification *ui;

private:
  void setupUi();
  void updateData();

  // NotificationWidget interface
protected:
  void onNotificationUpdate() override;
};

#endif // INFO_NOTIFICATION_H
