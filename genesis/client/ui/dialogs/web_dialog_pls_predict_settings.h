#pragma once
#ifndef WEB_DIALOG_PREDICT_SETTINGS_H
#define WEB_DIALOG_PREDICT_SETTINGS_H

#include "web_dialog.h"
#include "logic/structures/analysis_model_structures.h"

namespace Ui
{
  class WebDialogPlsPredictSettings;
}

namespace Dialogs
{
  class WebDialogPlsPredictSettings : public WebDialog
  {
    Q_OBJECT

  public:
    WebDialogPlsPredictSettings(QWidget* parent = nullptr);
    ~WebDialogPlsPredictSettings();

    void SetItems(const Structures::ParentAnalysisIds& names);
    QString GetSelected() const;

    bool IsManual() const;
    bool IsEmpty() const;
    bool IsAuto() const;

  private:
    void SetupUi();
    void CheckLayer(const QString& currenModelName);
    Structures::ParentAnalysisIds Analysis;
    Ui::WebDialogPlsPredictSettings* DialogWidget = nullptr;
  };
}
#endif