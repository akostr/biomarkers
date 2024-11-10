#include "confirm.h"
#include "ui_confirm.h"
#include <QStyle>
#include <genesis_style/style.h>

namespace Dialogs
{
namespace Templates
{
//PUT YOUR INTERNAL DIALOG CONTENT INTO ui->externalContentContainer!!!
//and TODO: rework this awful naming later. for now - i'm done ^_^.
Confirm::Confirm(QWidget *parent, QDialogButtonBox::StandardButtons buttons, QWidget *content)
  : WebDialog(parent, nullptr, QSize(), buttons),
    ui(new Ui::Confirm),
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

Confirm::Confirm(QWidget *parent, const Settings &settings, QDialogButtonBox::StandardButtons buttons, QWidget *content)
  : WebDialog(parent, nullptr, QSize(), buttons),
    ui(new Ui::Confirm),
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

Confirm::~Confirm()
{
  delete ui;
}

void Confirm::applySettings(const Settings &newSettings)
{
  mSettings = newSettings;
  mSettings.dialogHeader = mSettings.dialogHeader.toUpper();
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

Confirm::Settings Confirm::getSettings()
{
  return mSettings;
}

QWidget *Confirm::getContent()
{
  return ui->externalContentContainer;
}

Confirm *Confirm::warning(QWidget *parent, QString header, QString text, QDialogButtonBox::StandardButtons buttons)
{
  Settings s;
  s.dialogHeader = header;
  auto content = new QLabel(text);
  return new Confirm(parent, s, buttons, content);
}

Confirm *Confirm::confirmation(QWidget *parent, QString header, QString text, QDialogButtonBox::StandardButtons buttons)
{
  Settings s;
  s.dialogHeader = header;
  auto content = new QLabel(text);
  return new Confirm(parent, s, buttons, content);
}

Confirm *Confirm::confirmationWithRedPrimary(QWidget *parent, QString header, QString text, QDialogButtonBox::StandardButtons buttons, const QString& okBtnName)
{
  Settings s;
  s.dialogHeader = header;
  s.buttonsProperties[QDialogButtonBox::Ok]["red"] = true;
  s.buttonsNames[QDialogButtonBox::Ok] = okBtnName;
  auto content = new QLabel(text);
  return new Confirm(parent, s, buttons, content);
}

Confirm *Confirm::confirm(QWidget* parent, QString message, std::function<void ()> onConfirm, std::function<void ()> onIgnore, std::function<void ()> onCancel)
{
  auto s = Settings();
  using B = QDialogButtonBox::StandardButton;
  s.dialogHeader = tr("Confirm action");
  s.buttonsNames[B::Ignore] = tr("Continue");
  s.buttonsProperties[B::Save]["blue"] = true;
  s.buttonsProperties[B::Ignore]["red"] = true;
  auto msgLabel = new QLabel(message);
  msgLabel->setWordWrap(true);
  auto dial = new Confirm(parent, s, B::Save | B::Ignore | B::Cancel, msgLabel);
  connect(dial, &WebDialog::Clicked, parent, [onConfirm, onIgnore, onCancel](B result)
          {
            switch(result)
            {
            case B::Save:
              onConfirm();
              return;
            case B::Ignore:
              onIgnore();
              return;
            default:
              onCancel();
              return;
            }
          });
  return dial;
}

Confirm *Confirm::confirm(QWidget *parent, QString message, std::function<void ()> onConfirm, std::function<void ()> onCancel)
{
  auto s = Settings();
  using B = QDialogButtonBox::StandardButton;
  s.dialogHeader = tr("Confirm action");
  s.buttonsProperties[B::Save]["blue"] = true;
  auto msgLabel = new QLabel(message);
  msgLabel->setWordWrap(true);
  auto dial = new Confirm(parent, s, B::Save | B::Cancel, msgLabel);
  connect(dial, &WebDialog::Clicked, parent, [onConfirm, onCancel](B result)
          {
            switch(result)
            {
            case B::Save:
              onConfirm();
              return;
            default:
              onCancel();
              return;
            }
          });
  return dial;
}

void Confirm::setupUi()
{
  ui->dialogHeaderLabel->setStyleSheet(Style::Genesis::Fonts::ModalHeadConfirmation());
  connect(ui->closeButton, &QPushButton::clicked, this, &WebDialog::Reject);
}
}
}
