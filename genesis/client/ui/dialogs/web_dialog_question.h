#pragma once
#ifndef WEB_DIALOG_QUESTION_H
#define WEB_DIALOG_QUESTION_H

#include "web_dialog.h"

namespace Ui
{
  class WebDialogQuestion;
}

namespace Dialogs
{
  class WebDialogQuestion : public WebDialog
  {
    Q_OBJECT

  public:
    WebDialogQuestion(QWidget* parent = nullptr);
    ~WebDialogQuestion();

    void SetQuestion(const QString& question);
    void SetTitle(const QString& title);

  private:
    Ui::WebDialogQuestion* WidgetUi = nullptr;
  };
}
#endif