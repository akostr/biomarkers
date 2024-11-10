#pragma once

#include <QWidget>
#include "settings_structs.h"

namespace Ui {
  class FontSettingsWidget;
}

class FontSettingsWidget : public QWidget
{
  Q_OBJECT

public:
  explicit FontSettingsWidget(QWidget *parent = nullptr);
  ~FontSettingsWidget();

  void SetSettings(const FontSettings& settings);
  FontSettings GetSettings() const;

signals:
  void SettingsChanged(FontSettings setts);

protected slots:
  void OnSettingsChanged();

protected:
  Ui::FontSettingsWidget *ui;
};
