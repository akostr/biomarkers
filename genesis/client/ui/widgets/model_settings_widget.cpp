#include "model_settings_widget.h"

#include <QPushButton>
#include <QMenu>
#include <QWidgetAction>

#include <genesis_style/style.h>

namespace Widgets
{
  ModelSettingsWidget::ModelSettingsWidget(QWidget* parent)
    : QWidget(parent)
  {
    SetupUi();
  }


  void ModelSettingsWidget::SetMenu(QMenu* menu)
  {
    if(const auto button = findChild<QPushButton*>())
    {
      button->setMenu(menu);
    }
  }

  void ModelSettingsWidget::SetupUi()
  {
    const auto modelMenuButton = new QPushButton(tr("Model settings"));
    modelMenuButton->setStyleSheet(
      "QPushButton::menu-indicator{"
      "image: url(:/resource/controls/collpse_dn.png);"
      "width: 10px;right: 3px;}"
      "QMenu{border: 1px solid transparent;}");
    modelMenuButton->setProperty("menu_secondary", true);

    const auto widgetLayout = new QVBoxLayout(this);
    widgetLayout->addWidget(modelMenuButton);
    setLayout(widgetLayout);
  }
}
