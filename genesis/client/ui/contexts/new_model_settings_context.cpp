#include "new_model_settings_context.h"

namespace Widgets
{
  NewModelSettingsContext::NewModelSettingsContext(QWidget* parent)
    : AnalysisModelSettingsContext(parent)
  {
    Setup();
  }

  void NewModelSettingsContext::Setup()
  {
    const auto saveNewModelAction = addAction(QIcon(":/resource/icons/icon_save.png"), tr("Save new model"));
    insertAction(FirstAction, saveNewModelAction);
    connect(saveNewModelAction, &QAction::triggered, this, &NewModelSettingsContext::SaveNewModel);
  }
}