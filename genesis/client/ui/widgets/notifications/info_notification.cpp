#include "info_notification.h"
#include "ui_info_notification.h"
#include <genesis_style/style.h>

InfoNotification::InfoNotification(Notification *notification, QWidget *parent) :
  NotificationWidget(notification, parent),
  ui(new Ui::InfoNotification)
{
  setupUi();
}

InfoNotification::~InfoNotification()
{
  delete ui;
}

void InfoNotification::setupUi()
{
  mContent->setObjectName("InfoNotificationContent");
  ui->setupUi(mContent);
  ui->headerLabel->setProperty("style", "header");
  connect(ui->closeBtn, &QPushButton::clicked, this, &NotificationWidget::onNotificationClosed);
  updateData();
}

void InfoNotification::updateData()
{
  if(!mNotification)
    return;
  QString header = mNotification->GetHeader();
  if(header.isEmpty())
    header = tr("Information");//Уведомление?
  ui->headerLabel->setText(header);
  ui->textLabel->setText(mNotification->GetText());
}


void InfoNotification::onNotificationUpdate()
{
  updateData();
}
