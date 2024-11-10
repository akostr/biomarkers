#include "dialog.h"
#include "ui_dialog.h"
#include <QStyle>
#include <genesis_style/style.h>

namespace Dialogs
{
namespace Templates
{
//PUT YOUR INTERNAL DIALOG CONTENT INTO ui->externalContentContainer!!!
//and TODO: rework this awful naming later. for now - i'm done ^_^.
Dialog::Dialog(QWidget *parent, QDialogButtonBox::StandardButtons buttons, QWidget *content)
  : WebDialog(parent, nullptr, QSize(), buttons),
    ui(new Ui::Dialog),
    mInternalContent(new QWidget(Content))
{
  ui->setupUi(mInternalContent);
  setupUi();
  applySettings(Settings());

  Content->layout()->addWidget(mInternalContent);
  auto layout = new QVBoxLayout(ui->externalContentContainer);
  ui->externalContentContainer->setLayout(layout);
  layout->setContentsMargins(ui->horizontalLayout->contentsMargins());
  if(content)
    layout->addWidget(content);
}

Dialog::Dialog(QWidget *parent, const Settings &settings, QDialogButtonBox::StandardButtons buttons, QWidget *content)
  : WebDialog(parent, nullptr, QSize(), buttons),
    ui(new Ui::Dialog),
    mInternalContent(new QWidget(Content))
{
  ui->setupUi(mInternalContent);
  setupUi();
  applySettings(settings);

  Content->layout()->addWidget(mInternalContent);
  auto layout = new QVBoxLayout(ui->externalContentContainer);
  ui->externalContentContainer->setLayout(layout);
  layout->setContentsMargins(ui->horizontalLayout->contentsMargins());
  if(content)
    layout->addWidget(content);
}

Dialog::~Dialog()
{
  delete ui;
}

void Dialog::applySettings(const Settings &newSettings)
{
  mSettings = newSettings;
  ui->dialogHeaderLabel->setText(mSettings.dialogHeader);
  for(auto& b : ButtonBox->buttons())
  {
    auto btnCode = ButtonBox->standardButton(b);

    if(mSettings.buttonsNames.contains(btnCode))
      b->setText(mSettings.buttonsNames[btnCode]);

    if(mSettings.buttonsProperties.contains(btnCode))
    {
      auto map = mSettings.buttonsProperties[btnCode];
      for(auto iter = map.constBegin(); iter != map.constEnd(); iter++)
        b->setProperty(iter.key().toStdString().c_str(), iter.value());
    }
    b->style()->polish(b);
  }
}

QWidget *Dialog::getContent()
{
  return ui->externalContentContainer;
}

Dialog::Settings Dialog::getSettings() const
{
  return mSettings;
}

void Dialog::setupUi()
{
  ui->dialogHeaderLabel->setStyleSheet(Style::Genesis::Fonts::ModalHead());
  connect(ui->closeButton, &QPushButton::clicked, this, &WebDialog::Reject);
}

}
}
