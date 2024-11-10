#include "web_dialog_question.h"

#include "ui_web_dialog_question.h"

#include "genesis_style/style.h"

#include <QPushButton>

namespace Dialogs
{
  WebDialogQuestion::WebDialogQuestion(QWidget* parent)
    : WebDialog(parent, nullptr, QSize(), QDialogButtonBox::StandardButton::Ok | QDialogButtonBox::StandardButton::Cancel)
  {
    WidgetUi = new Ui::WebDialogQuestion();
    auto body = new QWidget();
    WidgetUi->setupUi(body);
    Content->layout()->addWidget(body);
    body->setStyleSheet(Style::Genesis::GetUiStyle());
    WidgetUi->TitleLabel->setStyleSheet(Style::Genesis::GetH2());
    WidgetUi->QuestionLabel->setStyleSheet(Style::Genesis::GetH3());
    auto okBtn = ButtonBox->button(QDialogButtonBox::StandardButton::Ok);
    okBtn->setStyleSheet("background-color: @errorColor");
    okBtn->setText(tr("Delete"));
  }

  WebDialogQuestion::~WebDialogQuestion()
  {
    delete WidgetUi;
  }

  void WebDialogQuestion::SetQuestion(const QString& question)
  {
    WidgetUi->QuestionLabel->setText(question);
  }

  void WebDialogQuestion::SetTitle(const QString& title)
  {
    WidgetUi->TitleLabel->setText(title);
  }
}
