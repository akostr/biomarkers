#include "menu_header_widget.h"
#include "ui_menu_header_widget.h"
#include "genesis_style/style.h"
#include "logic/context_root.h"
#include "logic/known_context_tag_names.h"
#include <ui/genesis_window.h>

#include <QStyle>
#include <QMenu>

const char* MenuHeaderWidget::kReservoirModuleName = "reservoir";
const char* MenuHeaderWidget::kBiomarkerModuleName = "biomarkers";
const char* MenuHeaderWidget::kPlotsModuleName = "plots";
const char* MenuHeaderWidget::kModulePropertyName = "module_style";

#include <QProxyStyle>
class ModulesMenuProxyStyle: public QProxyStyle {
public:
  ModulesMenuProxyStyle(QStyle* style = 0) : QProxyStyle(style) { }
  ModulesMenuProxyStyle(const QString& key) : QProxyStyle(key) { }
  virtual int pixelMetric(QStyle::PixelMetric metric, const QStyleOption* option = 0, const QWidget* widget = 0 ) const {
    switch ( metric ) {
    case QStyle::PM_SmallIconSize:
      return 24; //pixmap size
    default:
      return QProxyStyle::pixelMetric( metric, option, widget ); // return default values for the rest
    }
  }
};

MenuHeaderWidget::MenuHeaderWidget(QWidget *parent) :
  QWidget(parent),
  ui(new Ui::MenuHeaderWidget),
  mCurrentModule(0)
{
  setAttribute(Qt::WA_StyledBackground);
  setStyleSheet(Style::Genesis::GetMainMenuHeaderStyle());
  ui->setupUi(this);
  ui->subLogoTextLabel->setStyleSheet(Style::Genesis::Fonts::RegularBold());
  // style()->polish(ui->menuButton);
  mMenu = new QMenu(this);
  mMenu->setStyle(new ModulesMenuProxyStyle());
  mMenu->style()->setParent(mMenu);

  mActionToReservoirModule = new QAction(tr("Reservoir geochemical"), mMenu);
  mActionToReservoirModule->setIcon(QIcon(":/resource/icons/modules/small/reservoir@2x.png"));
  mActionToBiomarkersModule = new QAction(tr("Biomarkers"), mMenu);
  mActionToBiomarkersModule->setIcon(QIcon(":/resource/icons/modules/small/biomarkers@2x.png"));
  mActionToPlotsModule = new QAction(tr("Plots"), mMenu);
  mActionToModuleSelection = new QAction(tr("Modules selection"), mMenu);

  connect(mActionToReservoirModule, &QAction::triggered, this, []()
          {
            GenesisWindow::Get()->SwitchModule(Names::ModulesContextTags::MReservoir);
          });
  connect(mActionToBiomarkersModule, &QAction::triggered, this, []()
          {
            GenesisWindow::Get()->SwitchModule(Names::ModulesContextTags::MBiomarkers);
          });
  connect(mActionToPlotsModule, &QAction::triggered, this, []()
          {
            GenesisWindow::Get()->SwitchModule(Names::ModulesContextTags::MNoModule);
          });
  connect(mActionToModuleSelection, &QAction::triggered, this, []()
          {
            GenesisWindow::Get()->SwitchModule(Names::ModulesContextTags::MNoModule);
          });

  mMenu->addAction(mActionToReservoirModule);
  mMenu->addAction(mActionToBiomarkersModule);
  mMenu->addAction(mActionToPlotsModule);
  mMenu->addAction(mActionToModuleSelection);
  ui->menuButton->setMenu(mMenu);
  setCurrentModule(Names::ModulesContextTags::MReservoir);
}

MenuHeaderWidget::~MenuHeaderWidget()
{
  delete ui;
}

void MenuHeaderWidget::setCurrentModule(int module)
{
  using namespace Names::ModulesContextTags;
  mCurrentModule = module;
  Module m = (Module)module;
  switch(m)
  {
  case MReservoir:
    setModuleStyle(kReservoirModuleName);
    break;
  case MBiomarkers:
    setModuleStyle(kBiomarkerModuleName);
    break;
  case MPlots:
    setModuleStyle(kPlotsModuleName);
    break;
  case MNoModule:
  case MLast:
    setModuleStyle("");
    break;
  }
  formMenu();
}

void MenuHeaderWidget::setAvailableModulesList(const QList<int> &availableModules)
{
  mAvailableModules.clear();
  for(auto& m : availableModules)
    mAvailableModules.insert(m);
  formMenu();
}

QString MenuHeaderWidget::moduleStyle() const
{
  return mModule;
}

void MenuHeaderWidget::formMenu()
{
  using namespace Names::ModulesContextTags;
  QSet<int> allOfModules = {MReservoir, MBiomarkers, MPlots};
  auto missingModules = allOfModules;
  missingModules.subtract(mAvailableModules);
  missingModules.insert(mCurrentModule);
  for(auto& m : missingModules)
  {
    switch(Module(m))
    {
    case Names::ModulesContextTags::MReservoir:
      mActionToReservoirModule->setVisible(false);
      break;
    case Names::ModulesContextTags::MBiomarkers:
      mActionToBiomarkersModule->setVisible(false);
      break;
    case Names::ModulesContextTags::MPlots:
      mActionToPlotsModule->setVisible(false);
      break;
    case Names::ModulesContextTags::MNoModule:
    case Names::ModulesContextTags::MLast:
      break;
    }
  }
  for(auto& m : mAvailableModules)
  {
    if(m == mCurrentModule)
      continue;
    switch(Module(m))
    {
    case Names::ModulesContextTags::MReservoir:
      mActionToReservoirModule->setVisible(true);
      break;
    case Names::ModulesContextTags::MBiomarkers:
      mActionToBiomarkersModule->setVisible(true);
      break;
    case Names::ModulesContextTags::MPlots:
      mActionToPlotsModule->setVisible(true);
      break;
    case Names::ModulesContextTags::MNoModule:
    case Names::ModulesContextTags::MLast:
      break;
    }
  }
}

void MenuHeaderWidget::setModuleStyle(const QString &newModule)
{
  if (mModule == newModule)
    return;
  mModule = newModule;
  // ui->menuButton->setProperty(kModulePropertyName, mModule);

  if(mModule == kReservoirModuleName)
  {
    ui->subLogoTextLabel->setText(tr("Reservoir geochemistry"));
    ui->logoLabel->setPixmap(QPixmap(":/resource/icons/modules/small/reservoir@2x.png"));
  }
  else if(mModule == kBiomarkerModuleName)
  {
    ui->subLogoTextLabel->setText(tr("Biomarkers"));
    ui->logoLabel->setPixmap(QPixmap(":/resource/icons/modules/small/biomarkers@2x.png"));
  }
  else if(mModule == kPlotsModuleName)
  {
    ui->subLogoTextLabel->setText(tr("Visualisation"));
    ui->logoLabel->setPixmap(QPixmap(":/resource/icons/modules/small/graphics@2x.png"));
  }
  else
    ui->subLogoTextLabel->setText(tr("UNKNOWN MODULE"));

  // style()->polish(this);
  // style()->polish(ui->menuButton);
  // style()->polish(mMenu);
  emit moduleStyleChanged();
}


void MenuHeaderWidget::paintEvent(QPaintEvent *event)
{
  QWidget::paintEvent(event);
}
