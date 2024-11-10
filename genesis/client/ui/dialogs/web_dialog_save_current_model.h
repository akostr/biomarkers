#pragma once
#ifndef WEB_DIALOG_SAVE_CURRENT_MODEL_H
#define WEB_DIALOG_SAVE_CURRENT_MODEL_H

#include "web_dialog.h"

namespace Dialogs
{
  class WebDialogSaveCurrentModel : public WebDialogBox
  {
    Q_OBJECT
  public:
    WebDialogSaveCurrentModel(const QString& text, QWidget* parent = nullptr);
    ~WebDialogSaveCurrentModel() = default;
  };
}
#endif
