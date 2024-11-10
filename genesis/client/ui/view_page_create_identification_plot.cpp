#include "view_page_create_identification_plot.h"
#include "ui_view_page_create_identification_plot.h"
#include "logic/context_root.h"
#include "logic/enums.h"
#include "logic/known_context_tag_names.h"
#include "logic/tree_model_item.h"
#include "genesis_style/style.h"
#include "logic/service/ijson_serializer.h"
#include "logic/service/service_locator.h"
#include "logic/models/reference_analysis_data_table_model.h"
#include "ui/item_models/analysis_data_table_model.h"

#include "genesis_window.h"
#include "known_view_names.h"
#include "widgets/identification_plot_widget.h"
#include <QMenu>
#include <QAction>
#include <QStyle>
#include "api/api_rest.h"

#include "logic/known_json_tag_names.h"
#include "logic/markup/genesis_markup_enums.h"

using namespace LibraryFilter;

Q_DECLARE_METATYPE(Double2DVector);

namespace Views
{

ViewPageCreateIdentificationPlot::ViewPageCreateIdentificationPlot(QWidget *parent)
    : View(parent)
    , ui(new Ui::ViewPageCreateIdentificationPlot)
{

  setupUi();
  setupConnections();
  mModel = new Models::AnalysisDataTableModel(this);
}

ViewPageCreateIdentificationPlot::~ViewPageCreateIdentificationPlot()
{
  delete ui;
}

void ViewPageCreateIdentificationPlot::setupUi()
{
  setStyleSheet(Style::Genesis::GetUiStyle());
  ui->setupUi(this);
  ui->mainWidget->setAttribute(Qt::WA_StyledBackground, true);
  ui->mainWidget->setStyleSheet("background: white;");

  ui->captionLabel->setStyleSheet(Style::Genesis::Fonts::H1());
  ui->backButton->setStyleSheet(Style::Genesis::Fonts::H1());

  ui->stackedWidget->setCurrentWidget(ui->selectedPlotPage);

  ui->searchLineEdit->setPlaceholderText(tr("Search by name"));
  mNameSearchAction = new QAction(QIcon(":/resource/icons/search.png"), "", ui->searchLineEdit);
  mNameSearchAction->setToolTip(tr("Search by name"));
  mNameSearchClearAction = new QAction(QIcon(":/resource/icons/icon_action_cross.png"), "", ui->searchLineEdit);
  mNameSearchClearAction->setToolTip(tr("Clear search"));
  ui->searchLineEdit->addAction(mNameSearchAction, QLineEdit::ActionPosition::LeadingPosition);
  ui->searchLineEdit->addAction(mNameSearchClearAction, QLineEdit::ActionPosition::TrailingPosition);
  ui->filesFilterCombo->clear();
  ui->filesFilterCombo->addItem(tr("All"), static_cast<int>(FileType::All));
  ui->filesFilterCombo->addItem(tr("System"), static_cast<int>(FileType::System));
  ui->filesFilterCombo->addItem(tr("User"), static_cast<int>(FileType::User));
  ui->filesFilterCombo->addItem(tr("Favorite"), static_cast<int>(FileType::Favorite));
  ui->classifiersFilterCombo->clear();
  ui->classifiersFilterCombo->addItem(tr("All"), static_cast<int>(Classifiers::All));
  ui->classifiersFilterCombo->addItem(tr("GC_PID"), static_cast<int>(Classifiers::GC_PID));
  ui->classifiersFilterCombo->addItem(tr("GC_MS"), static_cast<int>(Classifiers::GC_MS));
  ui->classifiersFilterCombo->addItem(tr("Pyrolysis"), static_cast<int>(Classifiers::Pyrolysis));
  ui->classifiersFilterCombo->addItem(tr("Gas composition"), static_cast<int>(Classifiers::GasComposition));
  ui->classifiersFilterCombo->addItem(tr("Isotopy"), static_cast<int>(Classifiers::Isotopy));
  ui->classifiersFilterCombo->addItem(tr("Water composition"), static_cast<int>(Classifiers::WaterComposition));
  hideClearAction();
  ui->createPlotButton->setDisabled(false);
  ui->stackedWidget->setCurrentWidget(ui->selectedPlotPage);
}

void ViewPageCreateIdentificationPlot::setupConnections()
{
  connect(ui->backButton, &QPushButton::clicked, this, &ViewPageCreateIdentificationPlot::showPreviousPage);
  connect(ui->plotListWidget, &IdentificationPlotListWidget::showPreviousPage, this, &ViewPageCreateIdentificationPlot::showPreviousPage);
  connect(ui->closeButton, &QPushButton::clicked, this, &ViewPageCreateIdentificationPlot::showPreviousPage);
  connect(ui->createPlotButton, &QPushButton::clicked, this, &ViewPageCreateIdentificationPlot::createAndShowPlots);
  connect(ui->filesFilterCombo, &QComboBox::currentIndexChanged, this, &ViewPageCreateIdentificationPlot::applyFilter);
  connect(ui->classifiersFilterCombo, &QComboBox::currentIndexChanged, this, &ViewPageCreateIdentificationPlot::applyFilter);
  connect(ui->searchLineEdit, &QLineEdit::textChanged, this, &ViewPageCreateIdentificationPlot::applyFilter);
  connect(mNameSearchAction, &QAction::triggered, this, &ViewPageCreateIdentificationPlot::applyFilter);
  connect(mNameSearchClearAction, &QAction::triggered, ui->searchLineEdit, &QLineEdit::clear);
  connect(ui->searchLineEdit, &QLineEdit::textChanged, this, &ViewPageCreateIdentificationPlot::hideClearAction);

  connect(ui->templateListWidget->getPlotListModel(), &TreeModelDynamicLibraryTemplates::dataChanged, this,
          [this](const QModelIndex& tleft, const QModelIndex& tright, QList<int> roles)
          {
            if (!roles.contains(Qt::CheckStateRole) ||
                tright.column() < TreeModelDynamicLibraryTemplates::CHECKED ||
                tleft.column() > TreeModelDynamicLibraryTemplates::CHECKED)
              return;
            const auto count = ui->templateListWidget->getPlotListModel()->checkedCount();
            ui->createPlotButton->setDisabled(count == 0);
          });
}

void ViewPageCreateIdentificationPlot::applyFilter()
{
  ui->templateListWidget->FilterByState(Structures::LibraryFilterState(
    {
      ui->searchLineEdit->text(),
      QString(),
      static_cast<LibraryFilter::FileType>(ui->filesFilterCombo->currentIndex()),
      static_cast<LibraryFilter::Classifiers>(ui->classifiersFilterCombo->currentIndex()),
    }));
}

void ViewPageCreateIdentificationPlot::hideClearAction()
{
  mNameSearchClearAction->setVisible(!ui->searchLineEdit->text().isEmpty());
}

void ViewPageCreateIdentificationPlot::ApplyContextCreationPlot(const QString& dataId/*dataId*/, const QVariant& data/*data*/)
{
  auto isReset = isDataReset(dataId, data);
  if (isReset)
  {
    mTableId = Core::GenesisContextRoot::Get()->GetCreationPlot()->GetData(Names::CreationPlot::TableId).toInt();
    mParameterId = Core::GenesisContextRoot::Get()->GetCreationPlot()->GetData(Names::CreationPlot::ParameterId).toInt();
    const auto serializer = Service::ServiceLocator::Instance().Resolve<Model::ReferenceAnalysisDataTableModel, Service::IJsonSerializer>();
    auto modelJson = Core::GenesisContextRoot::Get()->GetCreationPlot()->GetData(Names::CreationPlot::ModelJson).toJsonObject();
    auto model = serializer->ToModel(modelJson);
    mModel->SetModel(model);
    mModel->SetCurrentParameter(mParameterId);
  }
  else
  {
    if (dataId == Names::CreationPlot::TableId)
    {
      mTableId = data.toInt();
    }
    else if (dataId == Names::CreationPlot::ParameterId)
    {
      mParameterId = data.toInt();
      mModel->SetCurrentParameter(mParameterId);
    }
    else if (dataId == Names::CreationPlot::ModelJson)
    {
      const auto serializer = Service::ServiceLocator::Instance().Resolve<Model::ReferenceAnalysisDataTableModel, Service::IJsonSerializer>();
      auto modelJson = Core::GenesisContextRoot::Get()->GetCreationPlot()->GetData(Names::CreationPlot::ModelJson).toJsonObject();
      auto model = serializer->ToModel(modelJson);
      mModel->SetModel(model);
      mModel->SetCurrentParameter(mParameterId);
    }
  }
  clear();
}

void ViewPageCreateIdentificationPlot::SwitchToPage(const QString& pageId)
{
  clear();
  View::SwitchToPage(pageId);
}

void ViewPageCreateIdentificationPlot::showPreviousPage()
{
  if (ui->stackedWidget->currentWidget() == ui->selectedPlotPage)
    GenesisWindow::Get()->ShowPage(ViewPageNames::ViewPageAnalysisDataTablePageName);
  else if (ui->stackedWidget->currentWidget() == ui->plotListPage)
    setPage(SelectedPage);
}

void ViewPageCreateIdentificationPlot::createAndShowPlots()
{
  createPlots();
  setPage(CreatedPlotsPage);
}

void ViewPageCreateIdentificationPlot::createPlots()
{
  std::unordered_map<int, QString> possibleData
    {
      { GenesisMarkup::PeakHeight, JsonTagNames::ValuesHeightData },
      { GenesisMarkup::PeakArea, JsonTagNames::ValuesAreaData },
      { GenesisMarkup::PeakCovatsIndex, JsonTagNames::ValuesCovatsData },
      { GenesisMarkup::PeakRetentionTime, JsonTagNames::ValuesRetTimeData },
      { GenesisMarkup::PeakMarkerWindow, JsonTagNames::ValuesCustomData }
    };
  QList<int> needToCreate = ui->templateListWidget->getPlotListModel()->CheckedItemIds();
  QPointer<TreeModelDynamicLibraryTemplates> model = ui->templateListWidget->getPlotListModel();
  ui->plotListWidget->createPlots(needToCreate, model, mTableId, possibleData[mModel->GetCurrentParameter()]);
}

void ViewPageCreateIdentificationPlot::clear()
{
  ui->templateListWidget->showTemplatesForTable(mTableId);
  ui->plotListWidget->clear();
  setPage(SelectedPage);
}

void ViewPageCreateIdentificationPlot::setPage(int pageIndex)
{
  ui->stackedWidget->setCurrentIndex(pageIndex);
}

}
