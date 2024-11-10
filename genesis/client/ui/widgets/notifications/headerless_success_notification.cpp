#include "headerless_success_notification.h"
#include "ui_headerless_success_notification.h"
#include <genesis_style/style.h>

HeaderlessSuccessNotification::HeaderlessSuccessNotification(Notification* notification, QWidget *parent) :
  NotificationWidget(notification, parent),
  ui(new Ui::HeaderlessInfoNotification)
{
  setupUi();
}

HeaderlessSuccessNotification::~HeaderlessSuccessNotification()
{
  delete ui;
}

void HeaderlessSuccessNotification::setupUi()
{
  mContent->setObjectName("HeaderlessSuccessNotificationContent");
  ui->setupUi(mContent);
  connect(ui->closeBtn, &QPushButton::clicked, this, &NotificationWidget::onNotificationClosed);
  updateData();
}

void HeaderlessSuccessNotification::updateData()
{
  if(!mNotification)
    return;
  ui->textLabel->setText(mNotification->GetText());
}

void HeaderlessSuccessNotification::onNotificationUpdate()
{
  updateData();
}
