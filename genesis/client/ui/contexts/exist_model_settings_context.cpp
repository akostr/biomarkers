#include "exist_model_settings_context.h"

namespace Widgets
{
  ExistModelSettingsContext::ExistModelSettingsContext(QWidget* parent)
    : AnalysisModelSettingsContext(parent)
  {
    Setup();
  }

  void ExistModelSettingsContext::Setup()
  {
    SaveChangesAction = addAction(QIcon(":/resource/icons/icon_save.png"), tr("Save changes"));
    SaveChangesAction->setShortcut(QKeySequence(QKeyCombination(Qt::ControlModifier, Qt::Key_S)));
    insertAction(FirstAction, SaveChangesAction);
    connect(SaveChangesAction, &QAction::triggered, this, &ExistModelSettingsContext::SaveChanges);

    const auto saveAsNewAction = addAction(QIcon(":/resource/icons/icon_save.png"), tr("Save as new model"));
    saveAsNewAction->setShortcut(QKeySequence(QKeyCombination(Qt::ControlModifier | Qt::ShiftModifier, Qt::Key_S)));
    insertAction(SaveChangesAction, saveAsNewAction);
    connect(saveAsNewAction, &QAction::triggered, this, &ExistModelSettingsContext::SaveAsNew);
  }
}
