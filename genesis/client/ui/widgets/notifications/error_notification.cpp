#include "error_notification.h"
#include "ui_error_notification.h"
#include <genesis_style/style.h>
#include <QMetaEnum>

ErrorNotification::ErrorNotification(Notification* notification, QWidget *parent) :
  NotificationWidget(notification, parent),
  ui(new Ui::ErrorNotification)
{
  setupUi();
}

ErrorNotification::~ErrorNotification()
{
  delete ui;
}

void ErrorNotification::setupUi()
{
  mContent->setObjectName("ErrorNotificationContent");
  ui->setupUi(mContent);
  ui->headerLabel->setProperty("style", "header");
  ui->errorLabel->hide();
  connect(ui->closeBtn, &QPushButton::clicked, this, &NotificationWidget::onNotificationClosed);
  updateData();
}

void ErrorNotification::updateData()
{
  if(!mNotification)
    return;
  QString header = mNotification->GetHeader();
  QString text = mNotification->GetText();

  if(header.isEmpty())
    header = tr("Error");

  ui->headerLabel->setText(header);
  ui->textLabel->setText(text);

  if(text.isEmpty() && !ui->textLabel->isHidden())
    ui->textLabel->hide();
  else if(!text.isEmpty() && ui->textLabel->isHidden())
    ui->textLabel->show();

  auto error = mNotification->GetData("network_error").value<QNetworkReply::NetworkError>();
  if(error != QNetworkReply::NoError)
  {
    auto errStr = mNotification->GetData("error_type").toString();
    auto e = QMetaEnum::fromType<QNetworkReply::NetworkError>().valueToKey(error);
    errStr = tr("Code:") + " " + QString::number((int)error) + ", " + tr("description:") + " \"" + e + "\"";
    ui->errorLabel->setText(errStr);
    if(ui->errorLabel->isHidden())
      ui->errorLabel->show();
  }
  else
  {
    if(!ui->errorLabel->isHidden())
      ui->errorLabel->hide();
  }
}

void ErrorNotification::onNotificationUpdate()
{
  updateData();
}
