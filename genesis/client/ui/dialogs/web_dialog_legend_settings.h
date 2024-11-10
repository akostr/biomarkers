#pragma once
#ifndef WEB_DIALOG_LEGEND_SETTINGS_H
#define WEB_DIALOG_LEGEND_SETTINGS_H

#include "ui/dialogs/templates/info.h"

namespace Ui
{
  class LegendSettingsWidget;
};

namespace Dialogs
{
  class WebDialogLegendSettings : public Templates::Info
  {
    Q_OBJECT
  public:
    explicit WebDialogLegendSettings(QWidget* parent = nullptr);
    ~WebDialogLegendSettings();


    void setApplyToAll(bool apply);
    bool applyToAll() const;

    QInternal::DockPosition legendPosition() const;
    void setLegendPosition(QInternal::DockPosition position);

    Qt::Alignment legendAlign() const;
    void setLegendAlign(Qt::Alignment align);

  private:
    Ui::LegendSettingsWidget* mUi = nullptr;

    void setupUi();
    void connectSignals();

    void updateAlignComboBox(int pos);
  };
}
#endif