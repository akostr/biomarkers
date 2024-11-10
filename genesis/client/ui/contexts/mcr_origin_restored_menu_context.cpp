#include "mcr_origin_restored_menu_context.h"

McrOriginRestoredMenuContext::McrOriginRestoredMenuContext(QWidget* parent)
  : QMenu(parent)
{
  SetupUi();
}

void McrOriginRestoredMenuContext::SetupUi()
{
  ExcludeAction = addAction(tr("Select for exclude from model"));
  CancelExcludeAction = addAction(tr("Cancel exclude from model"));
  BuildForNewModel = addAction(tr("Build for new model"));
}
