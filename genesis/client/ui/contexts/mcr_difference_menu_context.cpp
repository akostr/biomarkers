#include "mcr_difference_menu_context.h"

McrDifferentMenuContext::McrDifferentMenuContext(QWidget* parent)
  : QMenu(parent)
{
  SetupUi();
}

void McrDifferentMenuContext::SetTitleCount(int count)
{
  TitleAction->setText(tr("PICKED %n SAMPLES", "", count));
}

void McrDifferentMenuContext::SetupUi()
{
  TitleAction = addAction("");
  SetTitleCount(0);
  TitleAction->setDisabled(true);
  addSeparator();
  BuildOriginRestoredAction = addAction(tr("Build origin and restored"));
  ExcludeFromModelAction = addAction(tr("Select for exclude from model"));
  CancelExcludeAction = addAction(tr("Cacnel exclude from model"));
  BuildForNewModel = addAction(tr("Build for new model"));
}
