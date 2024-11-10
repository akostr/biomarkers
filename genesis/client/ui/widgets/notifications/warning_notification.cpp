#include "warning_notification.h"
#include "ui_warning_notification.h"

WarningNotification::WarningNotification(Notification *notification, QWidget *parent) :
  NotificationWidget(notification, parent),
  ui(new Ui::WarningNotification)
{
  setupUi();
}

WarningNotification::~WarningNotification()
{
  delete ui;
}

void WarningNotification::setupUi()
{
  mContent->setObjectName("WarningNotificationContent");
  ui->setupUi(mContent);
  ui->headerLabel->setProperty("style", "header");
  connect(ui->closeBtn, &QPushButton::clicked, this, &NotificationWidget::onNotificationClosed);
  updateData();
}

void WarningNotification::updateData()
{
  if(!mNotification)
    return;
  QString header = mNotification->GetHeader();
  if(header.isEmpty())
    header = tr("Warning");
  ui->headerLabel->setText(header);
  ui->textLabel->setText(mNotification->GetText());
}

void WarningNotification::onNotificationUpdate()
{
  updateData();
}
