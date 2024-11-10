#pragma once
#ifndef WEB_DIALOG_RATIO_MATRIX_SETTINGS_H
#define WEB_DIALOG_RATIO_MATRIX_SETTINGS_H

//#include "web_dialog.h"
#include <ui/dialogs/templates/dialog.h>

class DialogLineEditEx;

namespace Ui
{
  class WebDialogRatioMatrixSettings;
}

namespace Dialogs
{
  class WebDialogRatioMatrixSettings : public Templates::Dialog
  {
    Q_OBJECT

  public:
    WebDialogRatioMatrixSettings(QWidget* parent = nullptr);
    ~WebDialogRatioMatrixSettings();

    QString GetTitle() const;
    QString GetComment() const;
    double GetWindowSize() const;
    double GetMinimum() const;
    double GetMaximum() const;
    bool GetIsIncludeMarkers() const;

  private:
    Ui::WebDialogRatioMatrixSettings* WidgetUi = nullptr;

    void SetupUi();
    QPointer<DialogLineEditEx> LineEditEx;
  };
}
#endif
