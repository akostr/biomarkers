#include "build_model_context.h"
#include "genesis_style/style.h"
#include "logic/known_context_tag_names.h"

namespace Widgets
{
  BuildModelContext::BuildModelContext(QWidget* parent)
    : QMenu(parent)
  {
    SetupUi();
  }

  void BuildModelContext::SetActiveModule(int module)
  {
    auto newModule = (Names::ModulesContextTags::Module)module;
    switch(newModule)
    {
    case Names::ModulesContextTags::MReservoir:
      BuildMCRAction->setVisible(true);
      BuildPLSAction->setVisible(true);
      BuildPLSPredictSubMenuAction->setVisible(true);
      break;
    case Names::ModulesContextTags::MBiomarkers:
      BuildMCRAction->setVisible(false);
      BuildPLSAction->setVisible(false);
      BuildPLSPredictSubMenuAction->setVisible(false);
      break;
    case Names::ModulesContextTags::MPlots:
    case Names::ModulesContextTags::MNoModule:
    case Names::ModulesContextTags::MLast:
      //just a stub
      BuildMCRAction->setVisible(false);
      BuildPLSAction->setVisible(false);
      BuildPLSPredictSubMenuAction->setVisible(false);
      break;
    }
  }

  void BuildModelContext::SetupUi()
  {
    setStyleSheet(Style::Genesis::GetUiStyle());

    BuildPCAAction = addAction(tr("Build P_C_A"));
    BuildMCRAction = addAction(tr("Build M_C_R"));
    BuildPLSPredictSubMenuAction = addAction(tr("Build P_L_S"));
    const auto subMenu = new QMenu(this);
    BuildPLSPredictSubMenuAction->setMenu(subMenu);
    BuildPLSAction = subMenu->addAction(tr("Build P_L_S model"));
    BuildPLSPredictAction = subMenu->addAction(tr("Build P_L_S predict"));
  }
}
