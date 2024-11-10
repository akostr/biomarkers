#pragma once

#include <QWidget>
#include "settings_structs.h"

namespace Ui {
  class CurveSettingsWidget;
}

class CurveSettingsWidget : public QWidget
{
  Q_OBJECT

public:
  explicit CurveSettingsWidget(QWidget *parent = nullptr);
  ~CurveSettingsWidget();

  void SetSettings(const CurveSettings& settings);
  CurveSettings GetSettings() const;

signals:
  void SettingsChanged(CurveSettings setts);

protected slots:
  void OnSettingsChanged();

protected:
  Ui::CurveSettingsWidget* Ui;
};
