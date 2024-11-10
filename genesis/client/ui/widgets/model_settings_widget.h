#pragma once
#ifndef MODEL_SETTINGS_WIDGET_H
#define MODEL_SETTINGS_WIDGET_H

#include <QVBoxLayout>
#include <QWidget>
#include <QPushButton>

namespace Widgets
{
  class ModelSettingsWidget : public QWidget
  {
    Q_OBJECT

  public:
    ModelSettingsWidget(QWidget* parent);
    ~ModelSettingsWidget() = default;

    void SetMenu(QMenu* menu);

  private:
    void SetupUi();

    QMenu* WidgetMenu = nullptr;
  };
}
#endif