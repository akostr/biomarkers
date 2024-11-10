#pragma once

#include "logic/markup/markup_step_controller.h"
#include <QWidget>
#include <QPushButton>

namespace Ui {
  class ChromatogramWidgetSettingsParameretsDialog;
}

class ChromatogramWidgetSettingsParameretsDialog : public QWidget
{
  Q_OBJECT

public:
  ChromatogramWidgetSettingsParameretsDialog(GenesisMarkup::ChromatogrammModelPtr model,
    const GenesisMarkup::StepInfo& stepInfo, bool hideSmoothLevel = false,
    QPushButton* setDefaultSettings = nullptr, QWidget* parent = nullptr);
  ~ChromatogramWidgetSettingsParameretsDialog();

  void setSettings(const GenesisMarkup::ChromaSettings& settings);
  GenesisMarkup::ChromaSettings getSettings();
  int getOriginSmoothLvl() const;
  int getNewSmoothLvl() const;
  void setOriginBaseLine();

private:
  Ui::ChromatogramWidgetSettingsParameretsDialog* ui;

  GenesisMarkup::ChromatogrammModelPtr mChromatogramModel;
  GenesisMarkup::StepInfo mStepInfo;
  int mOriginSmoothLvl;
  GenesisMarkup::TBaseLineDataModel mBLineDataModelPtr;
  QPushButton* mSetDefaultSettings = nullptr;

  GenesisMarkup::ChromaSettings mSettings;
  bool mForbidden = true;

protected:
  void setupUi();
  void update();
  void checkDefaultSettings();
};
