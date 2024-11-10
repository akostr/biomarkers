#include "view_page_module_selection.h"
#include "ui/widgets/module_plate.h"
#include "ui_view_page_module_selection.h"
#include "ui/known_view_names.h"
#include "ui/genesis_window.h"
#include "genesis_style/style.h"
#include "logic/known_context_tag_names.h"
#include "logic/known_context_names.h"
#include "logic/context_root.h"

#include <QJsonDocument>

namespace Views
{
ViewPageModulesSelection::ViewPageModulesSelection(QWidget *parent) :
  View(parent),
  ui(new Ui::ViewPageModulesSelection)
{
  ui->setupUi(this);
  ui->modulesGridCaption->setStyleSheet(Style::Genesis::Fonts::H1());
  ReservoirPlate = new ModulePlate(this);
  ReservoirPlate->setActivePixmap(QPixmap(":/resource/icons/modules/reservoir_icon_rgb@2x.png"));
  ReservoirPlate->setInactivePixmap(QPixmap(":/resource/icons/modules/reservoir_icon_gs@2x.png"));
  ReservoirPlate->setPixmap(QPixmap(":/resource/icons/modules/reservoir_icon_rgb@2x.png"));
  ReservoirPlate->setHeadColor(QColor(166, 223, 255));
  ReservoirPlate->setAvailable(false);
  ReservoirPlate->setTitle(tr("Reservoir geochemistry"));
  ReservoirPlate->setDetails(tr("CDF import, markup, obtaining data tables, calculation of height ratio matrices, import of tables, construction of PCA (and calculation of the Hotelling criterion), MRC, PLC."));

  BiomarkersPlate = new ModulePlate(this);
  BiomarkersPlate->setActivePixmap(QPixmap(":/resource/icons/modules/biomarkers_icon_rgb@2x.png"));
  BiomarkersPlate->setInactivePixmap(QPixmap(":/resource/icons/modules/biomarkers_icon_gs@2x.png"));
  BiomarkersPlate->setPixmap(QPixmap(":/resource/icons/modules/biomarkers_icon_rgb@2x.png"));
  BiomarkersPlate->setHeadColor(QColor(133, 221, 179));
  BiomarkersPlate->setAvailable(false);
  BiomarkersPlate->setTitle(tr("Biomarkers"));
  BiomarkersPlate->setDetails(tr("Library, CDF import, labeling with identification, getting data tables, calculation of coefficient tables, importing tables, plotting from the library, building PCA (and calculating the Hotelling criterion)"));

  PlotsPlate = new ModulePlate(this);
  PlotsPlate->setActivePixmap(QPixmap(":/resource/icons/modules/graphics_icon_rgb@2x.png"));
  PlotsPlate->setInactivePixmap(QPixmap(":/resource/icons/modules/graphics_icon_gs@2x.png"));
  PlotsPlate->setPixmap(QPixmap(":/resource/icons/modules/graphics_icon_gs@2x.png"));
  PlotsPlate->setHeadColor(QColor(QRgb(0xfff4e2)));
  PlotsPlate->setAvailable(false);
  PlotsPlate->setTitle(tr("Plots"));
  PlotsPlate->setDetails("<p>" + tr("Standard plots: line, histogram, radial, scatter plot, radial plot; graphics tablets") + "</p> <p><a href=\"https://geobasis.spbu.ru\">" + tr("Details") + "</a></p>");

  auto modulesContext = Core::GenesisContextRoot::Get()->GetContextModules();
  connect(ReservoirPlate, &ModulePlate::clicked, this, [this, modulesContext]()
          {
            if(ReservoirPlate->Available())
              View::ForseSetModule(Names::ModulesContextTags::MReservoir);
            // GenesisWindow::Get()->SwitchModule(Names::ModulesContextTags::MReservoir);
          });
  connect(BiomarkersPlate, &ModulePlate::clicked, this, [this, modulesContext]()
          {
            if(BiomarkersPlate->Available())
              View::ForseSetModule(Names::ModulesContextTags::MBiomarkers);
                  // GenesisWindow::Get()->SwitchModule(Names::ModulesContextTags::MBiomarkers);
          });
  connect(PlotsPlate, &ModulePlate::clicked, this, [this, modulesContext]()
          {
            if(PlotsPlate->Available())
              View::ForseSetModule(Names::ModulesContextTags::MPlots);
                  // GenesisWindow::Get()->SwitchModule(Names::ModulesContextTags::MPlots);
          });

  ui->gridLayout->addWidget(ReservoirPlate, 0, 0);
  ui->gridLayout->addWidget(BiomarkersPlate, 0, 1);
  ui->gridLayout->addWidget(PlotsPlate, 0, 2);

  ui->descriptionLabel->setTextInteractionFlags(Qt::TextBrowserInteraction);
  ui->descriptionLabel->setOpenExternalLinks(true);

  ui->detailsLabel->setVisible(false);
}

ViewPageModulesSelection::~ViewPageModulesSelection()
{
  delete ui;
}
}//namespace Views


void Views::ViewPageModulesSelection::ApplyContextUser(const QString &dataId, const QVariant &data)
{
  bool isReset = isDataReset(dataId, data);
  auto context = Core::GenesisContextRoot::Get()->GetContextUser();
  bool isAuthorized = context->GetData(Names::UserContextTags::kUserState).toInt() == Names::UserContextTags::UserState::authorized;
  if(isReset)
  {
    auto availableModules = context->GetData(Names::UserContextTags::kAvailableModules).value<QList<int>>();
    ReservoirPlate->setAvailable(false);
    BiomarkersPlate->setAvailable(false);
    PlotsPlate->setAvailable(false);
    if(isAuthorized)
    {
      for(auto m : availableModules)
      {
        Names::ModulesContextTags::Module module = (Names::ModulesContextTags::Module)m;
        switch(module)
        {
        case Names::ModulesContextTags::Module::MReservoir:
          ReservoirPlate->setAvailable(true);
          break;
        case Names::ModulesContextTags::MBiomarkers:
          BiomarkersPlate->setAvailable(true);
          break;
        case Names::ModulesContextTags::MPlots:
          PlotsPlate->setAvailable(true);
          break;
        case Names::ModulesContextTags::MNoModule:
        case Names::ModulesContextTags::MLast:
          break;
        }
      }
    }
  }
  else if( dataId == Names::UserContextTags::kAvailableModules)
  {
    auto availableModules = data.value<QList<int>>();
    ReservoirPlate->setAvailable(false);
    BiomarkersPlate->setAvailable(false);
    PlotsPlate->setAvailable(false);
    if(isAuthorized)
    {
      for(auto m : availableModules)
      {
        Names::ModulesContextTags::Module module = (Names::ModulesContextTags::Module)m;
        switch(module)
        {
        case Names::ModulesContextTags::Module::MReservoir:
          ReservoirPlate->setAvailable(true);
          break;
        case Names::ModulesContextTags::MBiomarkers:
          BiomarkersPlate->setAvailable(true);
          break;
        case Names::ModulesContextTags::MPlots:
          PlotsPlate->setAvailable(true);
          break;
        case Names::ModulesContextTags::MNoModule:
        case Names::ModulesContextTags::MLast:
          break;
        }
      }
    }
  }
}
