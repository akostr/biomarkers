#include "web_dialog_check_code_word.h"
#include "ui_web_dialog_check_code_word.h"

#include <genesis_style/style.h>

#include <QLayout>
#include <QStyle>
#include <QPushButton>
#include <QAction>

namespace Dialogs
{
namespace Templates
{
WebDialogCheckCodeWord::WebDialogCheckCodeWord(const QString& codeWord, QWidget* parent) :
  Confirm(parent, QDialogButtonBox::Ok | QDialogButtonBox::Cancel)
  , ui(new Ui::WebDialogCheckCodeWord)
  , mContent(new QWidget(nullptr))
  , mCodeWord(codeWord)
{
  setupUi();
}

WebDialogCheckCodeWord::~WebDialogCheckCodeWord()
{
  delete ui;
}

void WebDialogCheckCodeWord::setupUi()
{
  Dialogs::Templates::Confirm::Settings s;
  s.dialogHeader = tr("Access to the project");
  s.buttonsNames = { {QDialogButtonBox::Ok, tr("Enter")},
                   {QDialogButtonBox::Cancel, tr("Cancel")} };
  applySettings(s);
  ButtonBox->button(QDialogButtonBox::Ok)->setDisabled(true);

  ui->setupUi(mContent);
  mContent->setContentsMargins(0, 0, 0, 0);
  mContent->layout()->setContentsMargins(0, 0, 0, 0);
  getContent()->layout()->setContentsMargins(0, 0, 0, 0);
  getContent()->layout()->addWidget(mContent);

  ui->label->setStyleSheet(Style::Genesis::Fonts::RegularLight());
  ui->lineEdit->setFocus();
  ui->lineEdit->setEchoMode(QLineEdit::EchoMode::Password);
  ui->lineEdit->setStyleSheet(Style::ApplySASS("QLineEdit {color:@uiInputElementColorText;} QLineEdit[text=\"\"] {color:@uiInputElementColorTextDisabled;}\n"
    "QLineEdit[style=\"error\"] {border-color: @errorColor;/*border: 2px solid @errorColor; border-radius: 4px;*/}\n"
    "QLineEdit[style=\"success\"] {border-color: @successColor;/*border: 2px solid @successColor; border-radius: 4px;*/}\n"));

  auto action = ui->lineEdit->addAction(QIcon(), QLineEdit::ActionPosition::TrailingPosition);
  action->setIcon(QIcon(":/resource/icons/password_mode_on.png"));
  connect(action, &QAction::triggered, this, [&, action]
  {
    if (ui->lineEdit->echoMode() == QLineEdit::EchoMode::Password)
    {
      ui->lineEdit->setEchoMode(QLineEdit::EchoMode::Normal);
      action->setIcon(QIcon(":/resource/icons/password_mode_off.png"));
    }
    else
    {
      ui->lineEdit->setEchoMode(QLineEdit::EchoMode::Password);
      action->setIcon(QIcon(":/resource/icons/password_mode_on.png"));
    }
  });

  connect(ui->lineEdit, &QLineEdit::textChanged, this, [this](const QString& text)
  {
    if (text == mCodeWord)
    {
      ui->lineEdit->setProperty("style", "success");
      ui->lineEdit->style()->polish(ui->lineEdit);
      ButtonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
    }
    else
    {
      ui->lineEdit->setProperty("style", "error");
      ui->lineEdit->style()->polish(ui->lineEdit);
      ButtonBox->button(QDialogButtonBox::Ok)->setDisabled(true);
    }
  });
}
}
}
