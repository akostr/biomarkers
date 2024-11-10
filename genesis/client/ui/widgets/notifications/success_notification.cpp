#include "success_notification.h"
#include "ui_success_notification.h"
#include <genesis_style/style.h>
#include <QStyle>

SuccessNotification::SuccessNotification(Notification* notification, QWidget *parent) :
  NotificationWidget(notification, parent),
  ui(new Ui::SuccessNotification)
{
  setupUi();
}

SuccessNotification::~SuccessNotification()
{
    delete ui;
}

void SuccessNotification::setupUi()
{
  mContent->setObjectName("SuccessNotificationContent");
  ui->setupUi(mContent);
  ui->headerLabel->setProperty("style", "header");
  connect(ui->closeBtn, &QPushButton::clicked, this, &NotificationWidget::onNotificationClosed);
  updateData();
}

void SuccessNotification::updateData()
{
  if(!mNotification)
    return;
  QString header = mNotification->GetHeader();
  if(header.isEmpty())
    header = tr("Success");
  ui->headerLabel->setText(header);
  ui->textLabel->setText(mNotification->GetText());
}

void SuccessNotification::onNotificationUpdate()
{
  updateData();
}
