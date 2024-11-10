#pragma once

#include "ui/dialogs/templates/dialog.h"
#include "logic/markup/markup_step_controller.h"

class ChromatogramWidgetSettingsParameretsDialog;

namespace Ui {
  class ChromatogramWidgetSettingsDialogRefactor;
}

class ChromatogramWidgetSettingsDialogRefactor : public Dialogs::Templates::Dialog
{
  Q_OBJECT

public:
  ChromatogramWidgetSettingsDialogRefactor(GenesisMarkup::ChromatogrammModelPtr model,
    const GenesisMarkup::StepInfo& stepInfo, int startTab = 0, QWidget* parent = nullptr);
  ~ChromatogramWidgetSettingsDialogRefactor();

  void Accept() override;
  void Reject() override;

  void setSettings(const GenesisMarkup::ChromaSettings& settings);
  GenesisMarkup::ChromaSettings getSettings();
  QMap<QUuid, GenesisMarkup::ChromaSettings> getIntervalsSettings();
  QMap<QUuid, QPair<double, double>> getDeprecationIntervals() const;
  int getOriginSmoothLvl() const;
  int getNewSmoothLvl() const;

private:
  Ui::ChromatogramWidgetSettingsDialogRefactor* ui;
  QWidget* mContent;

  GenesisMarkup::ChromatogrammModelPtr mChromatogramModel;
  GenesisMarkup::StepInfo mStepInfo;
  QPushButton* mSetDefaultSettings;
  QPointer<ChromatogramWidgetSettingsParameretsDialog> mMainWaidget;

  QMap <QUuid, QPointer<ChromatogramWidgetSettingsParameretsDialog>> mCustomWidgetsMap;
  QList<QUuid> mDeletedIntervalUid;
  QMap<QUuid, QPair<double, double>> mDeprecationIntervals;

protected:
  void setupUi();
};
