#include "view_page_library.h"
#include "logic/tree_model_presentation.h"
#include "ui_view_page_library.h"
#include "ui/known_view_names.h"
#include "ui/genesis_window.h"
#include "ui/itemviews/data_checkable_headerview.h"
#include "ui/dialogs/templates/confirm.h"
#include "ui/dialogs/templates/tree_confirmation.h"
#include "ui/widgets/plot_template_list_widget.h"
#include "logic/known_context_tag_names.h"
#include "logic/context_root.h"
#include "logic/tree_model_dynamic_library_compounds_additional.h"
#include "logic/tree_model_item.h"
#include "logic/notification.h"
#include "logic/structures/library/library_structures.h"
#include "genesis_style/style.h"
#include "api/api_rest.h"
#include <QVBoxLayout>
#include <QAction>
#include <QMenu>
#include <QRandomGenerator>

using namespace LibraryFilter;

namespace Views
{
  ViewPageLibrary::ViewPageLibrary(QWidget* parent) :
    View(parent),
    ui(new Ui::ViewPageLibrary)
  {
    setupUi();
    ConnectSignals();
  }

  ViewPageLibrary::~ViewPageLibrary()
  {
    delete ui;
  }

  void ViewPageLibrary::setupUi()
  {
    setStyleSheet(Style::Genesis::GetUiStyle());
    ui->setupUi(this);
    setupCompoundTab();
    setupCoefficientTab();
    ui->specificsFilterCombo->setToolTip(tr("Specifics are disabled for \"Compounds\" tab"));
    ui->contentContainer->setStyleSheet("QWidget#contentContainer {background: white;}");
    ui->caption->setStyleSheet(Style::Genesis::Fonts::H1());
    ui->caption->setText(tr("Library"));
    ui->searchLineEdit->setStyleSheet("QLineEdit {border: none; background: transparent; placeholder-text-color: rgba(0, 32, 51, 35%);}");
    ui->searchLineEdit->setPlaceholderText(tr("Search by name"));
    NameSearchAction = new QAction(QIcon(":/resource/icons/search.png"), "", ui->searchLineEdit);
    NameSearchAction->setVisible(!ui->searchLineEdit->text().isEmpty());
    NameSearchAction->setToolTip(tr("Search by name"));
    NameSearchClearAction = new QAction(QIcon(":/resource/icons/icon_action_cross.png"), "", ui->searchLineEdit);
    NameSearchClearAction->setToolTip(tr("Clear search"));
    ui->searchLineEdit->addAction(NameSearchAction, QLineEdit::ActionPosition::LeadingPosition);
    ui->searchLineEdit->addAction(NameSearchClearAction, QLineEdit::ActionPosition::TrailingPosition);
    ui->filesFilterCombo->clear();
    ui->classifiersFilterCombo->clear();
    ui->specificsFilterCombo->clear();
    ui->groupFilterCombo->clear();
    ui->filesFilterCombo->addItem(tr("All files"), static_cast<int>(FileType::All));
    ui->filesFilterCombo->addItem(tr("System"), static_cast<int>(FileType::System));
    ui->filesFilterCombo->addItem(tr("User"), static_cast<int>(FileType::User));
    ui->filesFilterCombo->addItem(tr("Favorite"), static_cast<int>(FileType::Favorite));
    ui->classifiersFilterCombo->addItem(tr("All classifiers"), static_cast<int>(Classifiers::All));
    ui->classifiersFilterCombo->addItem(tr("GC_PID"), static_cast<int>(Classifiers::GC_PID));
    ui->classifiersFilterCombo->addItem(tr("GC_MS"), static_cast<int>(Classifiers::GC_MS));
    // ui->classifiersFilterCombo->addItem(tr("Saturated"), static_cast<int>(Classifiers::Saturated));
    // ui->classifiersFilterCombo->addItem(tr("Aromatic"), static_cast<int>(Classifiers::Aromatic));
    ui->classifiersFilterCombo->addItem(tr("Pyrolysis"), static_cast<int>(Classifiers::Pyrolysis));
    ui->classifiersFilterCombo->addItem(tr("Gas composition"), static_cast<int>(Classifiers::GasComposition));
    ui->classifiersFilterCombo->addItem(tr("Isotopy"), static_cast<int>(Classifiers::Isotopy));
    ui->classifiersFilterCombo->addItem(tr("Water composition"), static_cast<int>(Classifiers::WaterComposition));
    ui->specificsFilterCombo->addItem(tr("All specifics"), static_cast<int>(Specifics::All));
    ui->specificsFilterCombo->addItem(tr("Genesis"), static_cast<int>(Specifics::Genesis));
    ui->specificsFilterCombo->addItem(tr("Maturity"), static_cast<int>(Specifics::Maturity));
    ui->specificsFilterCombo->addItem(tr("Biodegradation"), static_cast<int>(Specifics::Biodegradation));
    ui->specificsFilterCombo->addItem(tr("Sedimentation conditions"), static_cast<int>(Specifics::SedimentationConditions));
    ui->specificsFilterCombo->addItem(tr("Not specified"), static_cast<int>(Specifics::NotSpecified));
    ui->specificsFilterCombo->setEnabled(false);

    //    ui->compoundSplitter->setSizes({ {INT_MAX, 0} });

    this->style()->polish(this);

    std::function<void(QWidget*)> eventFilterInstall = [this](QWidget* widget)
    {
      for(auto& c : widget->findChildren<QComboBox*>())
      {
        c->setFocusPolicy(Qt::StrongFocus);
        c->installEventFilter(this);
      }
      for(auto& c : widget->findChildren<QAbstractSpinBox*>())
      {
        c->setFocusPolicy(Qt::StrongFocus);
        c->installEventFilter(this);
      }
      for(auto& c : widget->findChildren<QTabBar*>())
      {
        c->setFocusPolicy(Qt::StrongFocus);
        c->installEventFilter(this);
      }
    };
    eventFilterInstall(ui->compoundsDetailsStackWgt);
    eventFilterInstall(ui->coefficientsDetailsStackWgt);
    eventFilterInstall(ui->stwdtPlotTemplate);
  }

  void ViewPageLibrary::setupCompoundTab()
  {
    ui->compoundsAddTableCaption->setStyleSheet(Style::Genesis::GetH2());
    ui->compoundCaption->setStyleSheet(Style::Genesis::GetH2());
    ui->compoundAddCaption->setStyleSheet(Style::Genesis::GetH2());
    ui->compoundModifyCaption->setStyleSheet(Style::Genesis::GetH2());
    ui->compoundsAddTableView->setStyleSheet(Style::ApplySASS("QTreeView::branch:selected{border: 3px solid; "
      "border-left-color: transparent; "
      "border-right-color: @brandColor;"
      "border-top-color: transparent;"
      "border-bottom-color: transparent;}"));
    ui->compoundsAddTableView->style()->polish(ui->compoundsAddTableView);
    ui->compoundsAddTableView->header()->setStretchLastSection(false);
    ui->compoundConfirmAdditionBtn->setProperty("blue", true);
    ui->compoundConfirmModifyingBtn->setProperty("blue", true);
    ui->compoundCancelAdditionBtn->setProperty("secondary", true);
    ui->compoundCancelModifyingBtn->setProperty("secondary", true);
    ui->compoundConfirmAdditionBtn->style()->polish(ui->compoundConfirmAdditionBtn);
    ui->compoundConfirmModifyingBtn->style()->polish(ui->compoundConfirmModifyingBtn);
    ui->compoundCancelAdditionBtn->style()->polish(ui->compoundCancelAdditionBtn);
    ui->compoundCancelModifyingBtn->style()->polish(ui->compoundCancelModifyingBtn);

    mCompundAddingModel = new TreeModelDynamicLibraryCompoundsAdditional(ui->compoundsAddTableView);

    mCompundAddingProxyModel = mCompundAddingModel->GetPresentationModel();
    {
      using CS = TreeModelDynamicLibraryCompounds::Column;
      QSet<int> FilteredColumns =
      {
        CS::CHECKED,
        CS::CLASSIFIER_ID,
        CS::COMPOUND_CLASS_TYPE_ID,
        CS::LIBRARY_ELEMENT_ID,
        CS::LIBRARY_GROUP_ID,
        CS::ALL_GROUPS_TITLES,
        CS::PDF_LOAD_DATE,
        CS::PDF_SIZE,
        CS::XLSX_SIZE,
        CS::XLSX_LOAD_DATE,
        CS::XLSX_TITLE,
        CS::PDF_TITLE,
        CS::LAST_COLUMN,
        CS::MASS_SPECTRUM,
        CS::INTENSITY_SPECTRUM,
        CS::TEMPLATE_TYPE_ID,
        CS::TEMPLATE_X_AXIS_ID,
        CS::TEMPLATE_Y_AXIS_ID,
        CS::TEMPLATE_Z_AXIS_ID,
        CS::TEMPLATE_OBJECTS,
        CS::TEMPLATE_X_AXIS_TITLE,
        CS::TEMPLATE_Y_AXIS_TITLE,
        CS::TEMPLATE_Z_AXIS_TITLE,
        CS::TEMPLATE_AXES_TITLES,
      };
      auto columns = mCompundAddingModel->getColumns().keys();
      QSet<int> acceptedColumns = QSet<int>(columns.begin(), columns.end()).subtract(FilteredColumns);
      mCompundAddingProxyModel->SetAcceptedColumns(acceptedColumns);
    }

    ui->templateListWidget->setPageLibrary(this);

    ui->compoundsAddTableView->setModel(mCompundAddingProxyModel);
    ui->compoundsAddTableView->header()->resizeSections(QHeaderView::ResizeToContents);

    ui->compoundsTablesStackWgt->setCurrentIndex(CTPView);
    ui->compoundsDetailsStackWgt->setCurrentIndex(CDPEmpty);
    ui->compoundControlMenuBtn->setProperty("menu_secondary", true);

    auto editCompoundAction = new QAction(tr("Edit"));
    ShareAccessCompoundAction = new QAction(tr("Share access"));
    auto removeCompoundAction = new QAction(tr("Remove"));
    MakeSystemCompoundAction = new QAction(tr("Make system"));

    auto compoundMenu = new QMenu(ui->compoundControlMenuBtn);
    compoundMenu->addAction(editCompoundAction);
    ShareAccessCompoundAction->setMenu(new QMenu(compoundMenu));
    compoundMenu->addAction(ShareAccessCompoundAction);
    compoundMenu->addAction(removeCompoundAction);
    compoundMenu->addAction(MakeSystemCompoundAction);
    MakeSystemCompoundAction->setVisible(Core::GenesisContextRoot::Get()->GetContextUser()->GetData(Names::UserContextTags::kSuperuser).toBool());

    connect(editCompoundAction, &QAction::triggered, this, &ViewPageLibrary::beginCompoundModifying);
    connect(removeCompoundAction, &QAction::triggered, this,
            [this]()
            {
              auto dw = ui->compoundDetailsWidget;
              removeCompound(dw->libraryGroupId(), dw->title());
            });
    connect(MakeSystemCompoundAction, &QAction::triggered, this, &ViewPageLibrary::makeCompoundSystemic);
    connect(ShareAccessCompoundAction->menu(), &QMenu::aboutToShow, this, &ViewPageLibrary::updateCompoundSharingMenu);

    ui->compoundControlMenuBtn->setMenu(compoundMenu);
    ui->compoundControlMenuBtn->style()->polish(ui->compoundControlMenuBtn);
    ui->compoundControlMenuBtn->setProperty("disabled_for_system", false);
    ui->compoundsDetailsStackWgt->setCurrentIndex(CDPEmpty);
  }

  void ViewPageLibrary::setupCoefficientTab()
  {
    ui->coefficientAddTableCaption->setStyleSheet(Style::Genesis::GetH2());
    ui->coefficientsCaption->setStyleSheet(Style::Genesis::GetH2());
    ui->coefficientsAddCaption->setStyleSheet(Style::Genesis::GetH2());
    ui->coefficientsModifyCaption->setStyleSheet(Style::Genesis::GetH2());
    ui->coefficientAddTableView->setStyleSheet(Style::ApplySASS("QTreeView::branch:selected{border: 3px solid; "
      "border-left-color: transparent; "
      "border-right-color: @brandColor;"
      "border-top-color: transparent;"
      "border-bottom-color: transparent;}"));
    ui->coefficientAddTableView->style()->polish(ui->coefficientAddTableView);
    ui->coefficientAddTableView->header()->setStretchLastSection(false);
    ui->coefficientsConfirmAdditionBtn->setProperty("blue", true);
    ui->coefficientsConfirmModifyingBtn->setProperty("blue", true);
    ui->coefficientsCancelAdditionBtn->setProperty("secondary", true);
    ui->coefficientsCancelModifyingBtn->setProperty("secondary", true);
    ui->coefficientsConfirmAdditionBtn->style()->polish(ui->coefficientsConfirmAdditionBtn);
    ui->coefficientsConfirmModifyingBtn->style()->polish(ui->coefficientsConfirmModifyingBtn);
    ui->coefficientsCancelAdditionBtn->style()->polish(ui->coefficientsCancelAdditionBtn);
    ui->coefficientsCancelModifyingBtn->style()->polish(ui->coefficientsCancelModifyingBtn);

    mCoefficientAddingModel = new TreeModelDynamicLibraryCompoundsAdditional(ui->compoundsAddTableView, ElementType::COEFFICIENT);
    mCoefficientAddingProxyModel = mCoefficientAddingModel->GetPresentationModel();
    {
      using CS = TreeModelDynamicLibraryCompounds::Column;
      QSet<int> FilteredColumns =
      {
        CS::CHECKED,
        CS::CLASSIFIER_ID,
        CS::COMPOUND_CLASS_TYPE_ID,
        CS::LIBRARY_ELEMENT_ID,
        CS::LIBRARY_GROUP_ID,
        CS::ALL_GROUPS_TITLES,
        CS::PDF_LOAD_DATE,
        CS::PDF_SIZE,
        CS::PDF_TITLE,
        CS::LAST_COLUMN,
        CS::INTENSITY_SPECTRUM,
        CS::TEMPLATE_TYPE_ID,
        CS::TEMPLATE_X_AXIS_ID,
        CS::TEMPLATE_Y_AXIS_ID,
        CS::TEMPLATE_Z_AXIS_ID,
        CS::TEMPLATE_OBJECTS,
        CS::TEMPLATE_X_AXIS_TITLE,
        CS::TEMPLATE_Y_AXIS_TITLE,
        CS::TEMPLATE_Z_AXIS_TITLE,
        CS::TEMPLATE_AXES_TITLES,
      };
      auto columns = mCoefficientAddingModel->getColumns().keys();
      QSet<int> acceptedColumns = QSet<int>(columns.begin(), columns.end()).subtract(FilteredColumns);
      mCoefficientAddingProxyModel->SetAcceptedColumns(acceptedColumns);
    }

    ui->coefficientAddTableView->setModel(mCoefficientAddingProxyModel);
    ui->coefficientAddTableView->header()->resizeSections(QHeaderView::ResizeToContents);

    ui->coefficientsWidget->setCaptionLabel(tr("Coefficient list"));
    ui->coefficientsControlMenuBtn->setProperty("menu_secondary", true);

    auto editCoefficientAction = new QAction(tr("Edit"));
    ShareAccessCoefficientAction = new QAction(tr("Share access"));
    auto removeCoefficientAction = new QAction(tr("Remove"));
    MakeSystemCoefficientAction = new QAction(tr("Make system"));

    auto coefficientMenu = new QMenu(ui->coefficientsControlMenuBtn);
    coefficientMenu->addAction(editCoefficientAction);
    ShareAccessCoefficientAction->setMenu(new QMenu(coefficientMenu));
    coefficientMenu->addAction(ShareAccessCoefficientAction);
    coefficientMenu->addAction(removeCoefficientAction);
    coefficientMenu->addAction(MakeSystemCoefficientAction);
    MakeSystemCoefficientAction->setVisible(Core::GenesisContextRoot::Get()->GetContextUser()->GetData(Names::UserContextTags::kSuperuser).toBool());

    ui->coefficientsControlMenuBtn->setMenu(coefficientMenu);
    ui->coefficientsControlMenuBtn->style()->polish(ui->coefficientsControlMenuBtn);
    ui->coefficientsControlMenuBtn->setProperty("disabled_for_system", false);
    ui->coefficientsDetailsStackWgt->setCurrentIndex(CDPEmpty);

    connect(editCoefficientAction, &QAction::triggered, this, &ViewPageLibrary::beginCoefficientModifying);
    connect(removeCoefficientAction, &QAction::triggered, this,
            [this]()
            {
              auto dw = ui->coefficientsDetailsWidget;
              removeCoefficient(dw->libraryGroupId(), dw->title());
            });
    connect(MakeSystemCoefficientAction, &QAction::triggered, this, &ViewPageLibrary::makeCoefficientSystemic);
    connect(ShareAccessCoefficientAction->menu(), &QMenu::aboutToShow, this, &ViewPageLibrary::updateCompoundSharingMenu);

  }

  void ViewPageLibrary::ConnectSignals()
  {
    connect(NameSearchAction, &QAction::triggered, this, &ViewPageLibrary::applyComboFilter);
    connect(NameSearchClearAction, &QAction::triggered, ui->searchLineEdit, &QLineEdit::clear);
    connect(ui->searchLineEdit, &QLineEdit::textChanged, this, &ViewPageLibrary::applyComboFilter);

    connect(ui->filesFilterCombo, &QComboBox::currentIndexChanged, this, &ViewPageLibrary::applyComboFilter);
    connect(ui->classifiersFilterCombo, &QComboBox::currentIndexChanged, this, &ViewPageLibrary::applyComboFilter);
    connect(ui->specificsFilterCombo, &QComboBox::currentIndexChanged, this, &ViewPageLibrary::applyComboFilter);
    connect(ui->groupFilterCombo, &QComboBox::currentTextChanged, this, &ViewPageLibrary::applyComboFilter);

    connectCompoundSignals();
    connectCoefficientSignals();
    connectTemplateSignals();
    connect(ui->tabWidget, &QTabWidget::currentChanged, this, &ViewPageLibrary::tabWidgetIndexChanged);


    connect(this, &ViewPageLibrary::attachmentsLoadingError, this,
      [this](int libraryGroupId)
      {
        TreeModelDynamic::ResetInstances("TreeModelDynamicLibraryCompounds");
        ui->compoundsWidget->setCurrentLibraryGroupId(libraryGroupId);
        ui->coefficientsWidget->setCurrentLibraryGroupId(libraryGroupId);
      }, Qt::QueuedConnection);

    //stack widget resizing handling (should work but it doesn't):
    connect(ui->addingDetailsStackWgt, &QStackedWidget::currentChanged, this,
      [this](int ind)
      {
        auto wgt = ui->addingDetailsStackWgt;
        for (int i = 0; i < wgt->count(); i++)
        {
          wgt->widget(i)->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
        }
        wgt->widget(ind)->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
        wgt->widget(ind)->adjustSize();
      });
  }

  void ViewPageLibrary::connectCompoundSignals()
  {
    connect(ui->compoundsWidget, &LibraryCompoundWidget::CompoundReloaded, this, [&]()
      {
        const auto keys = ui->compoundsWidget->GetCompoundKeys();
        ui->addingAddCoefficientsWidget->SetKeywordData(keys);
        ui->coefficientsModifyWidget->SetKeywordData(keys);
      });
    connect(ui->compoundsWidget, &LibraryCompoundWidget::CompoundReloaded, this, &ViewPageLibrary::updateTemplateAxisList);

    connect(ui->compoundsWidget, &LibraryCompoundWidget::PickedItem, this, &ViewPageLibrary::onCompoundPicked);
    connect(ui->compoundsWidget, &LibraryCompoundWidget::PickedItem, ui->compoundDetailsWidget, [&]()
      {
        ui->splitter->setSizes({ {INT_MAX, INT_MAX} });
      }, Qt::SingleShotConnection);
    connect(ui->compoundsWidget, &LibraryCompoundWidget::AddCompoundActionTriggered,
      this, &ViewPageLibrary::beginCompoundAdding);
    connect(ui->compoundsWidget, &LibraryCompoundWidget::ActionModeCompoundTriggered,
      this, &ViewPageLibrary::actionModeTriggered);

    connect(ui->compoundsWidget, &LibraryCompoundWidget::CancelAccessCompound,
      this, &ViewPageLibrary::cancelActionModeTriggered);

    connect(ui->addingAddCompoundWidget, &AddCompoundWidget::dataChanged, this,
      [this](const QVariantMap& data)
      {
        auto item = mCompundAddingModel->MainItem();
        auto itemData = item->GetData();
        itemData.insert(data);
        item->SetData(itemData);
      });
    connect(ui->addingAddCompoundWidget, &AddCompoundWidget::titlesChanged,
      mCompundAddingModel,
      &TreeModelDynamicLibraryCompoundsAdditional::FindRelated);

    connect(mCompundAddingModel, &TreeModelDynamicLibraryCompoundsAdditional::subHeaderIndexChanged, this,
      [this](const QModelIndex& newSubHeaderIndex)
      {
        mCompundAddingProxyModel->invalidate();
        auto mappedIndex = mCompundAddingProxyModel->mapFromSource(newSubHeaderIndex);
        ui->compoundsAddTableView->setProperty("sub_header_index", mappedIndex);
        ui->compoundsAddTableView->setFirstColumnSpanned(mappedIndex.row(), mappedIndex.parent(), true);
      },
      Qt::QueuedConnection);//need to be queued or column span will be removed after model reset
    //wich will be emitted right after "subheader index changed" signal
    connect(ui->compoundsAddTableView->selectionModel(), &QItemSelectionModel::currentRowChanged, this,
      [this](const QModelIndex& current, const QModelIndex& previous)
      {
        auto indexVal = ui->compoundsAddTableView->property("sub_header_index");
        if (!indexVal.isValid() || indexVal.isNull())
          return;
        auto subHeaderIndex = indexVal.value<QModelIndex>();
        if (subHeaderIndex.isValid() && current.row() == subHeaderIndex.row())
          return;
        if (current.row() == mCompundAddingProxyModel->mapFromSource(mCompundAddingModel->MainItem()->GetIndex(1)).row())
        {
          ui->addingDetailsStackWgt->setCurrentIndex(CADPAdd);
        }
        else
        {
          ui->addingDetailsStackWgt->setCurrentIndex(CADPDetails);
          ui->addingCompoundDetailsWidged->setData(mCompundAddingModel->GetItem(mCompundAddingProxyModel->mapToSource(current))->GetData());

          //TODO: make this widget shorter by resizing scroll area content
//                ui->addingCompoundDetailsWidged->adjustSize()
//                ui->addingDetailsStackWgt->adjustSize();
//                ui->scrollArea_2->viewport()->adjustSize();
        }
      });
    connect(ui->compoundCancelAdditionBtn, &QPushButton::clicked, this,
      [&]()
      {
        disableUiCombobox(false);
        ui->compoundsDetailsStackWgt->setCurrentIndex(CDPDetails);
        ui->compoundsTablesStackWgt->setCurrentIndex(CTPView);
      });
    connect(ui->compoundConfirmAdditionBtn, &QPushButton::clicked, this, &ViewPageLibrary::onCompoundAdditionConfirmed);
    connect(ui->compoundControlMenuBtn, &QPushButton::clicked, this, &ViewPageLibrary::beginCompoundModifying);
    connect(ui->compoundCancelModifyingBtn, &QPushButton::clicked, this, &ViewPageLibrary::leaveCompoundModifying);
    connect(ui->compoundConfirmModifyingBtn, &QPushButton::clicked, this, &ViewPageLibrary::onCompoundModificationConfirmed);
  }

  void ViewPageLibrary::connectCoefficientSignals()
  {
    connect(ui->coefficientsWidget, &LibraryCoefficientWidget::CoefficientReloaded, this, &ViewPageLibrary::updateTemplateAxisList);
    connect(ui->coefficientsWidget, &LibraryCoefficientWidget::PickedItem, this, &ViewPageLibrary::onCoefficientPicked);
    connect(ui->coefficientsWidget, &LibraryCoefficientWidget::PickedItem, ui->coefficientsDetailsWidget, [&]()
      {
        ui->splitter_2->setSizes({ {INT_MAX, INT_MAX} });
      }, Qt::SingleShotConnection);

    connect(ui->coefficientsWidget, &LibraryCoefficientWidget::AddCoefficientActionTriggered,
      this, &ViewPageLibrary::beginCoefficientAdding);

    connect(ui->coefficientsWidget, &LibraryCoefficientWidget::ActionModeCoefficientTriggered,
      this, &ViewPageLibrary::actionModeTriggered);

    connect(ui->coefficientsWidget, &LibraryCoefficientWidget::CancelAccessCoefficient,
      this, &ViewPageLibrary::cancelActionModeTriggered);

    connect(ui->addingAddCoefficientsWidget, &AddCoefficientWidget::dataChanged, this,
      [this](const QVariantMap& data)
      {
        auto item = mCoefficientAddingModel->MainItem();
        auto itemData = item->GetData();
        itemData.insert(data);
        item->SetData(itemData);
      });

    connect(ui->addingAddCoefficientsWidget, &AddCoefficientWidget::titlesChanged,
      mCoefficientAddingModel,
      &TreeModelDynamicLibraryCompoundsAdditional::FindRelated);

    connect(mCoefficientAddingModel, &TreeModelDynamicLibraryCompoundsAdditional::subHeaderIndexChanged, this,
      [this](const QModelIndex& newSubHeaderIndex)
      {
        mCoefficientAddingProxyModel->invalidate();
        auto mappedIndex = mCoefficientAddingProxyModel->mapFromSource(newSubHeaderIndex);
        ui->coefficientAddTableView->setProperty("sub_header_index", mappedIndex);
        ui->coefficientAddTableView->setFirstColumnSpanned(mappedIndex.row(), mappedIndex.parent(), true);
      },
      Qt::QueuedConnection);

    connect(ui->coefficientAddTableView->selectionModel(), &QItemSelectionModel::currentRowChanged, this,
      [this](const QModelIndex& current, const QModelIndex& previous)
      {
        auto indexVal = ui->coefficientAddTableView->property("sub_header_index");
        if (!indexVal.isValid() || indexVal.isNull())
          return;
        auto subHeaderIndex = indexVal.value<QModelIndex>();
        if (subHeaderIndex.isValid() && current.row() == subHeaderIndex.row())
          return;
        if (current.row() == mCoefficientAddingProxyModel->mapFromSource(mCoefficientAddingModel->MainItem()->GetIndex(1)).row())
        {
          ui->addingCoefficientsDetailsStackWgt->setCurrentIndex(CADPAdd);
        }
        else
        {
          ui->addingCoefficientsDetailsStackWgt->setCurrentIndex(CADPDetails);
          ui->addingCoefficientsDetailsWidged->setData(mCoefficientAddingModel->GetItem(mCoefficientAddingProxyModel->mapToSource(current))->GetData());
        }
      });
    connect(ui->coefficientsCancelAdditionBtn, &QPushButton::clicked, this,
      [&]()
      {
        disableUiCombobox(false);
        ui->coefficientsDetailsStackWgt->setCurrentIndex(CDPDetails);
        ui->coefficientTablesStackWgt->setCurrentIndex(CTPView);
      });
    connect(ui->coefficientsConfirmAdditionBtn, &QPushButton::clicked, this, &ViewPageLibrary::onCoefficientAdditionConfirmed);
    connect(ui->coefficientsControlMenuBtn, &QPushButton::clicked, this, &ViewPageLibrary::beginCoefficientModifying);
    connect(ui->coefficientsCancelModifyingBtn, &QPushButton::clicked, this, &ViewPageLibrary::leaveCoefficientModifying);
    connect(ui->coefficientsConfirmModifyingBtn, &QPushButton::clicked, this, &ViewPageLibrary::onCoefficientModificationConfirmed);
  }

  void ViewPageLibrary::connectTemplateSignals()
  {
    ui->templateEditPlotWidget->setModel(ui->templateListWidget->getPlotTemplateModel());

    connect(ui->templateListWidget, &PlotTemplateListWidget::nextStep, this, [&]() {
      ui->templateEditPlotWidget->setPlotType(ui->templateListWidget->getPlotType());
      ui->stwdtPlotTemplate->setCurrentWidget(ui->templateEditPlotWidget);
      ui->templateEditPlotWidget->setEditMode(ui->templateListWidget->getMode() != PlotTemplateListWidget::Mode::AddMode);
    });

    connect(ui->templateEditPlotWidget, &PlotTemplateWidget::previousStep, this,
            [&]() {
      ui->stwdtPlotTemplate->setCurrentWidget(ui->templateListWidget);
    });

    connect(ui->templateEditPlotWidget, &PlotTemplateWidget::accept, ui->templateListWidget, &PlotTemplateListWidget::saveEditableTemplate);
    connect(ui->templateEditPlotWidget, &PlotTemplateWidget::accept, this,
            [&]() {
      ui->stwdtPlotTemplate->setCurrentWidget(ui->templateListWidget);
    });

    connect(ui->templateEditPlotWidget, &PlotTemplateWidget::cancel, this,
            [&]() {
      ui->stwdtPlotTemplate->setCurrentWidget(ui->templateListWidget);
      ui->templateListWidget->leaveModifying();
    });

    connect (ui->templateListWidget, &PlotTemplateListWidget::disableFiltes, this, &ViewPageLibrary::disableUiCombobox);

    connect(ui->templateListWidget, &PlotTemplateListWidget::ActionModeTriggered,
            this, &ViewPageLibrary::actionModeTriggered);

    connect(ui->templateListWidget, &PlotTemplateListWidget::CancelAccess,
            this, &ViewPageLibrary::cancelActionModeTriggered);
  }

  void ViewPageLibrary::applyComboFilter()
  {
    const auto searchText = ui->searchLineEdit->text();
    Structures::LibraryFilterState state =
        {
          searchText,
          ui->groupFilterCombo->currentText(),
          static_cast<LibraryFilter::FileType>(ui->filesFilterCombo->currentIndex()),
          static_cast<LibraryFilter::Classifiers>(ui->classifiersFilterCombo->currentIndex()),
          static_cast<LibraryFilter::Specifics>(ui->specificsFilterCombo->currentIndex())
        };
    ui->compoundsWidget->FilterByState(state);
    ui->coefficientsWidget->FilterByState(state);
    ui->templateListWidget->FilterByState(state);
    NameSearchClearAction->setVisible(!searchText.isEmpty());
  }

  void ViewPageLibrary::beginCompoundAdding()
  {
    disableUiCombobox(true);

    ui->compoundsDetailsStackWgt->setCurrentIndex(CDPAdd);
    ui->compoundsTablesStackWgt->setCurrentIndex(CTPAdd);

    mCompundAddingModel->Clear();
    ui->addingAddCompoundWidget->clearAttachments();
    ui->addingAddCompoundWidget->clear();

    ui->compoundsDetailsStackWgt->setCurrentIndex(CDPAdd);
    ui->addingDetailsStackWgt->setCurrentIndex(CADPAdd);
    ui->compoundsTablesStackWgt->setCurrentIndex(CTPAdd);
  }

  void ViewPageLibrary::beginCoefficientAdding()
  {
    disableUiCombobox(true);

    ui->coefficientsDetailsStackWgt->setCurrentIndex(CDPAdd);
    ui->coefficientTablesStackWgt->setCurrentIndex(CTPAdd);

    mCoefficientAddingModel->Clear();
    ui->addingAddCoefficientsWidget->clearAttachments();
    ui->addingAddCoefficientsWidget->clear();

    ui->coefficientsDetailsStackWgt->setCurrentIndex(CDPAdd);
    ui->addingCoefficientsDetailsStackWgt->setCurrentIndex(CADPAdd);
  }

  void ViewPageLibrary::disableNotCurrentTab(bool isDisable)
  {
    // remember active tab
    const auto index = ui->tabWidget->currentIndex();
    for (int i = 0; i < ui->tabWidget->count(); i++)
      ui->tabWidget->setTabEnabled(i, !isDisable);
    ui->tabWidget->setTabEnabled(index, true);
    ui->tabWidget->setCurrentIndex(index);
  }

  void ViewPageLibrary::disableUiCombobox(bool isDisable)
  {
    ui->filesFilterCombo->setDisabled(isDisable);
    ui->classifiersFilterCombo->setDisabled(isDisable);
    ui->groupFilterCombo->setDisabled(isDisable);
    ui->specificsFilterCombo->setDisabled(isDisable);
    ui->compoundsWidget->hideActionMenu(isDisable);
    ui->coefficientsWidget->hideActionMenu(isDisable);
    if (!isDisable && ui->tabWidget->currentIndex() == static_cast<int>(Tab::Compounds))
      ui->specificsFilterCombo->setDisabled(true);
    disableNotCurrentTab(isDisable);
    ui->searchLineEdit->setDisabled(isDisable);
  }

  void ViewPageLibrary::disableUiForOpenShare(bool disable)
  {
    ui->groupFilterCombo->setDisabled(disable);
    ui->compoundControlMenuBtn->setDisabled(disable || ui->compoundControlMenuBtn->property("disabled_for_system").toBool());
    ui->coefficientsControlMenuBtn->setDisabled(disable || ui->coefficientsControlMenuBtn->property("disabled_for_system").toBool());
    ui->compoundsWidget->hideActionMenu(disable);
    ui->coefficientsWidget->hideActionMenu(disable);
    disableNotCurrentTab(disable);
    ui->templateListWidget->disableUiForOpenShare(disable);
  }

  void ViewPageLibrary::leaveCompoundAdding()
  {
    disableUiCombobox(false);
    ui->compoundsDetailsStackWgt->setCurrentIndex(CDPEmpty);
    ui->compoundsTablesStackWgt->setCurrentIndex(CTPView);
  }

  void ViewPageLibrary::leaveCoefficientAdding()
  {
    disableUiCombobox(false);
    ui->coefficientsDetailsStackWgt->setCurrentIndex(CDPEmpty);
    ui->coefficientTablesStackWgt->setCurrentIndex(CTPView);
  }

  void ViewPageLibrary::actionModeTriggered()
  {
    disableUiForOpenShare(true);
  }

  void ViewPageLibrary::cancelActionModeTriggered()
  {
    disableUiForOpenShare(false);
  }

  bool ViewPageLibrary::checkInputValidity(const QVariantMap& data, QString* msg)
  {
    auto validAndNotNull = [](const QVariant& val)->bool
      {return val.isValid() && !val.isNull(); };
    auto checkByTag = [&validAndNotNull, &data](const QString& tag)->bool
    {return validAndNotNull(data[tag]); };
    bool dataValidity = checkByTag("classifier_title") &&
                        checkByTag("short_title") &&
                        checkByTag("full_title") &&
                        checkByTag("coefficient_formula");
    bool someStringIsEmpty = data["classifier_title"].toString().isEmpty() ||
      data["short_title"].toString().isEmpty() ||
      data["full_title"].toString().isEmpty() ||
      data["coefficient_formula"].toString().isEmpty();
    if (msg)
    {
      auto makeMsg = [msg](const QString fieldTag)
        {
          if (msg->isEmpty())
            *msg = tr("Field %1 is empty").arg(fieldTag);
          else
            msg->append(QString("\n") + tr("Field %1 is empty").arg(fieldTag));
        };
      if (data["classifier_title"].toString().isEmpty())
        makeMsg(tr("classifier_title"));
      if (data["short_title"].toString().isEmpty())
        makeMsg(tr("short_title"));
      if (data["full_title"].toString().isEmpty())
        makeMsg(tr("full_title"));
      if(data["coefficient_formula"].toString().isEmpty())
        makeMsg(tr("coefficient_formula"));
    }

    return dataValidity && !someStringIsEmpty;
  }

  void ViewPageLibrary::beginCompoundModifying()
  {
    auto item = ui->compoundsWidget->getCurrentItem();
    if (!item)
    {
      Notification::NotifyWarning(tr("Pick item for modifying"));
      return;
    }
    disableUiCombobox(true);
    ui->modifyWidget->clear();
    ui->modifyWidget->setData(item->GetData());

    ui->compoundsDetailsStackWgt->setCurrentIndex(CDPModify);
  }

  void ViewPageLibrary::beginCoefficientModifying()
  {
    auto item = ui->coefficientsWidget->getCurrentItem();
    if (!item)
    {
      Notification::NotifyWarning(tr("Pick item for modifying"));
      return;
    }
    disableUiCombobox(true);
    ui->coefficientsModifyWidget->clear();
    ui->coefficientsModifyWidget->setData(item->GetData());

    ui->coefficientsDetailsStackWgt->setCurrentIndex(CDPModify);
  }

  void ViewPageLibrary::leaveCompoundModifying()
  {
    disableUiCombobox(false);
    ui->compoundsDetailsStackWgt->setCurrentIndex(CDPDetails);
  }

  void ViewPageLibrary::leaveCoefficientModifying()
  {
    disableUiCombobox(false);
    ui->coefficientsDetailsStackWgt->setCurrentIndex(CDPDetails);
  }

  void ViewPageLibrary::onCompoundPicked(const QVariantMap& item)
  {
    if (item.isEmpty())
    {
      ui->compoundsDetailsStackWgt->setCurrentIndex(CDPEmpty);
      return;
    }
    switch (ui->compoundsDetailsStackWgt->currentIndex())
    {
    case CDPEmpty:
      ui->compoundsDetailsStackWgt->setCurrentIndex(CDPDetails);
      //no break here!
    case CDPDetails:
    {
      ui->compoundDetailsWidget->setData(item);
      ui->compoundCaption->setText(item["full_title"].toString());
      auto userContext = Core::GenesisContextRoot::Get()->GetContextUser();
      bool isSystemItem = item["system"].toBool();
      bool isSuperuser = userContext->GetData(Names::UserContextTags::kSuperuser).toBool();
      bool isDisabled = isSystemItem && !isSuperuser;
      ui->compoundControlMenuBtn->setDisabled(isDisabled);
      ui->compoundControlMenuBtn->setProperty("disabled_for_system", isDisabled);
      MakeSystemCompoundAction->setVisible(!isSystemItem && isSuperuser);
      ShareAccessCompoundAction->setVisible(!isSystemItem);
    }
    break;
    case CDPModify:
      if (ui->modifyWidget->hasChanges())
      {
        auto confirmation = Dialogs::Templates::Confirm::confirmation(this, tr("Warning"),
          tr("You have unsaved changes, wich you will lose if continue"),
          QDialogButtonBox::Save | QDialogButtonBox::Ignore | QDialogButtonBox::Discard);
        auto settings = confirmation->getSettings();
        settings.buttonsProperties[QDialogButtonBox::Ignore]["red"] = true;
        confirmation->applySettings(settings);
        connect(confirmation, &Dialogs::Templates::Confirm::Clicked, this,
          [this, item, confirmation](QDialogButtonBox::StandardButton button)
          {
            switch (button)
            {
            case QDialogButtonBox::Save:
              break;
            case QDialogButtonBox::Cancel:
              break;
            case QDialogButtonBox::Ignore:
              ui->modifyWidget->setData(item);
              break;
            default:
              break;
            }
            confirmation->deleteLater();
          });
        confirmation->Open();
      }
      else
      {
        ui->modifyWidget->setData(item);
      }
      break;
    default:
      break;
    }
  }

  void ViewPageLibrary::onCoefficientPicked(const QVariantMap& item)
  {
    if (item.isEmpty())
    {
      ui->coefficientsDetailsStackWgt->setCurrentIndex(CDPEmpty);
      return;
    }
    switch (ui->coefficientsDetailsStackWgt->currentIndex())
    {
    case CDPEmpty:
      ui->coefficientsDetailsStackWgt->setCurrentIndex(CDPDetails);
      //no break here!
    case CDPDetails:
    {
      ui->coefficientsDetailsWidget->setData(item);
      ui->coefficientsCaption->setText(item["full_title"].toString());
      const auto userContext = Core::GenesisContextRoot::Get()->GetContextUser();
      const bool isSystemItem = item["system"].toBool();
      const bool isSuperuser = userContext->GetData(Names::UserContextTags::kSuperuser).toBool();
      const bool isDisabled = isSystemItem && !isSuperuser;
      ui->coefficientsControlMenuBtn->setDisabled(isDisabled);
      ui->coefficientsControlMenuBtn->setProperty("disabled_for_system", isDisabled);
      MakeSystemCoefficientAction->setVisible(!isSystemItem && isSuperuser);
      ShareAccessCoefficientAction->setVisible(!isSystemItem);
    }
    break;
    case CDPModify:
      if (ui->coefficientsModifyWidget->hasChanges())
      {
        auto confirmation = Dialogs::Templates::Confirm::confirmation(this, tr("Warning"),
          tr("You have unsaved changes, wich you will lose if continue"),
          QDialogButtonBox::Save | QDialogButtonBox::Ignore | QDialogButtonBox::Discard);
        auto settings = confirmation->getSettings();
        settings.buttonsProperties[QDialogButtonBox::Ignore]["red"] = true;
        confirmation->applySettings(settings);
        connect(confirmation, &Dialogs::Templates::Confirm::Clicked, this,
          [this, item, confirmation](QDialogButtonBox::StandardButton button)
          {
            if (button == QDialogButtonBox::Ignore)
              ui->coefficientsModifyWidget->setData(item);
            confirmation->deleteLater();
          });
        confirmation->Open();
      }
      else
      {
        ui->coefficientsModifyWidget->setData(item);
      }
      break;
    default:
      break;
    }
  }

  void ViewPageLibrary::handleEmptyData(const QVariantMap& item)
  {
    ui->compoundsDetailsStackWgt->setCurrentIndex(
      item.isEmpty()
      ? CDPEmpty
      : CDPDetails);
  }

  void ViewPageLibrary::beginSelectedCompoundSharing()
  {
    auto libraryGroupId = ui->compoundDetailsWidget->libraryGroupId();
    ui->compoundsWidget->startItemSharing(libraryGroupId);
  }

  void ViewPageLibrary::beginSelectedCoefficientSharing()
  {
    auto libraryGroupId = ui->coefficientsDetailsWidget->libraryGroupId();
    ui->coefficientsWidget->startItemSharing(libraryGroupId);
  }

  void ViewPageLibrary::beginSelectedCompoundRemoving()
  {
    auto libraryGroupId = ui->compoundDetailsWidget->libraryGroupId();
    ui->compoundsWidget->startItemRemoving(libraryGroupId);
  }

  void ViewPageLibrary::beginSelectedCoefficientRemoving()
  {
    auto libraryGroupId = ui->coefficientsDetailsWidget->libraryGroupId();
    ui->coefficientsWidget->startItemRemoving(libraryGroupId);
  }

  void ViewPageLibrary::makeCompoundSystemic()
  {
    auto libraryGroupId = ui->compoundDetailsWidget->libraryGroupId();
    API::REST::MakeCompoundSystemic(libraryGroupId,
      [](QNetworkReply*, QJsonDocument doc)
      {
        auto obj = doc.object();
        // qDebug().noquote() << doc.toJson();
        if (obj["error"].toBool())
        {
          Notification::NotifyError(obj["msg"].toString(), tr("Error while making compound systemic"));
          TreeModelDynamic::ResetInstances("TreeModelDynamicLibraryCompounds");
          return;
        }
        Notification::NotifySuccess(obj["msg"].toString(), "Compound maked systemic");
        TreeModelDynamic::ResetInstances("TreeModelDynamicLibraryCompounds");
      },
      [](QNetworkReply*, QNetworkReply::NetworkError err)
      {
        Notification::NotifyError(tr("Network error"), err);
        TreeModelDynamic::ResetInstances("TreeModelDynamicLibraryCompounds");
      });
  }

  void ViewPageLibrary::makeCoefficientSystemic()
  {
    auto libraryGroupId = ui->coefficientsDetailsWidget->libraryGroupId();
    API::REST::MakeCompoundSystemic(libraryGroupId,
      [](QNetworkReply*, QJsonDocument doc)
      {
        auto obj = doc.object();
        // qDebug().noquote() << doc.toJson();
        if (obj["error"].toBool())
        {
          Notification::NotifyError(obj["msg"].toString(), tr("Error while making compound systemic"));
          TreeModelDynamic::ResetInstances("TreeModelDynamicLibraryCompounds");
          return;
        }
        Notification::NotifySuccess(obj["msg"].toString(), "Compound maked systemic");
        TreeModelDynamic::ResetInstances("TreeModelDynamicLibraryCompounds");
      },
      [](QNetworkReply*, QNetworkReply::NetworkError err)
      {
        Notification::NotifyError(tr("Network error"), err);
        TreeModelDynamic::ResetInstances("TreeModelDynamicLibraryCompounds");
      });
  }

  void ViewPageLibrary::onCompoundModificationConfirmed()
  {
    if (!ui->modifyWidget->hasChanges())
    {
      leaveCompoundModifying();
      return;
    }
    QString validityErrorMsg;
    auto data = ui->modifyWidget->getData();
    if (!checkInputValidity(data, &validityErrorMsg))
    {
      Notification::NotifyError(validityErrorMsg, tr("Input error"));
      return;
    }

    auto loadingOverlayId = GenesisWindow::Get()->ShowOverlay(tr("Uploading data"));

    auto makeValue = [](const QString& str)->QJsonValue
      {
        return str.isEmpty() ? QJsonValue() : QJsonValue(str);
      };

    QJsonObject outdata;// = QJsonObject::fromVariantMap(data);//TODO: check it

    outdata["element_type_id"] = 1;
    outdata["library_element_id"] = data["library_element_id"].toInt();
    outdata["short_title"] = makeValue(data["short_title"].toString());
    outdata["full_title"] = makeValue(data["full_title"].toString());
    outdata["classifier_name"] = makeValue(data["classifier_title"].toString());
    outdata["compound_class_name"] = makeValue(data["compound_class_title"].toString());
    outdata["group_name"] = makeValue(ui->groupFilterCombo->currentText());
    outdata["specific_name"] = "Генезис";//TODO: find out wtf is it
    outdata["tic_sim"] = makeValue(data["tic_sim"].toString());
    outdata["chemical_formula"] = makeValue(data["chemical_formula"].toString());
    outdata["mz"] = makeValue(data["mz"].toString());
    outdata["dimension"] = makeValue(data["dimension"].toString());
    outdata["kovats_index"] = data["kovats_index"].toDouble();
    outdata["kovats_index_description"] = makeValue(data["kovats_index_description"].toString());
    outdata["analysis_methodology"] = makeValue(data["analysis_methodology"].toString());
    outdata["bibliographical_reference"] = makeValue(data["bibliographical_reference"].toString());
    outdata["literature_source"] = makeValue(data["literature_source"].toString());
    int pdfFileId = data["pdf_id"].toInt();

    QString name = outdata["short_title"].toString();
    QString groupName = outdata["group_name"].toString();
    API::REST::AddNewOrModifyCompound(outdata,
      [name, groupName, this, pdfFileId, loadingOverlayId](QNetworkReply*, QJsonDocument doc)
      {
        auto obj = doc.object();
        if (obj["error"].toBool())
        {
          Notification::NotifyError(doc.object()["msg"].toString(), "Server error");
          GenesisWindow::Get()->RemoveOverlay(loadingOverlayId);
          return;
        }
        bool hasAnyAdditionalOperation = false;
        Notification::NotifySuccess(tr("Compound %1 added to group %2").arg(name).arg(groupName));
        leaveCompoundModifying();
        int createdLibraryGroupId = obj["created_library_group_id"].toInt();
        auto pdfOp = ui->modifyWidget->getPdfOperation();
        auto specOp = ui->modifyWidget->getSpectrumOperation();
        if (pdfOp == ModifyCompoundWidget::OPRemove || specOp == ModifyCompoundWidget::OPRemove)
        {
          removeAttachments(createdLibraryGroupId,
            pdfFileId,
            loadingOverlayId,
            pdfOp == ModifyCompoundWidget::OPRemove,
            specOp == ModifyCompoundWidget::OPRemove);
          connect(this, &ViewPageLibrary::compoundUpdatingComplete, this,
            [this, createdLibraryGroupId]()
            {
              TreeModelDynamic::ResetInstances("TreeModelDynamicLibraryCompounds");
              ui->compoundsWidget->setCurrentLibraryGroupId(createdLibraryGroupId);
            }, Qt::SingleShotConnection);
          hasAnyAdditionalOperation = true;
        }
        QString pdfPath;
        QString specPath;
        if (pdfOp == ModifyCompoundWidget::OPModify)
          pdfPath = ui->modifyWidget->getPdfPath();
        if (specOp == ModifyCompoundWidget::OPModify)
          specPath = ui->modifyWidget->getSpectrumPath();

        if (!pdfPath.isEmpty() || !specPath.isEmpty())
        {
          bool hasAnyUploading = loadAttachments(createdLibraryGroupId, loadingOverlayId, pdfPath, specPath);
          if (!hasAnyUploading)
          {
            TreeModelDynamic::ResetInstances("TreeModelDynamicLibraryCompounds");
            ui->compoundsWidget->setCurrentLibraryGroupId(obj["created_library_group_id"].toInt());
            GenesisWindow::Get()->RemoveOverlay(loadingOverlayId);
          }
          else
          {
            connect(this, &ViewPageLibrary::compoundUpdatingComplete, this,
              [this, createdLibraryGroupId]()
              {
                ui->compoundsWidget->setCurrentLibraryGroupId(createdLibraryGroupId);
              }, Qt::SingleShotConnection);
          }
          hasAnyAdditionalOperation = true;
        }
        else
        {
          TreeModelDynamic::ResetInstances("TreeModelDynamicLibraryCompounds");
        }
        if (!hasAnyAdditionalOperation)
        {
          GenesisWindow::Get()->RemoveOverlay(loadingOverlayId);
        }

      },
      [loadingOverlayId](QNetworkReply*, QNetworkReply::NetworkError e)
      {
        Notification::NotifyError(tr("Network error"), e);
        GenesisWindow::Get()->RemoveOverlay(loadingOverlayId);
      });
  }

  void ViewPageLibrary::onCoefficientModificationConfirmed()
  {
    if (!ui->coefficientsModifyWidget->hasChanges())
    {
      leaveCoefficientModifying();
      return;
    }
    QString validityErrorMsg;
    auto data = ui->coefficientsModifyWidget->getData();
    if (!checkInputValidity(data, &validityErrorMsg))
    {
      Notification::NotifyError(validityErrorMsg, tr("Input error"));
      return;
    }

    auto loadingOverlayId = GenesisWindow::Get()->ShowOverlay(tr("Uploading data"));

    auto makeValue = [](const QString& str)->QJsonValue
      {
        return str.isEmpty() ? QJsonValue() : QJsonValue(str);
      };

    QJsonObject outdata;// = QJsonObject::fromVariantMap(data);//TODO: check it

    outdata["element_type_id"] = 2;
    outdata["library_element_id"] = data["library_element_id"].toInt();
    outdata["short_title"] = makeValue(data["short_title"].toString());
    outdata["full_title"] = makeValue(data["full_title"].toString());
    outdata["classifier_name"] = makeValue(data["classifier_title"].toString());
    outdata["compound_class_name"] = makeValue(data["compound_class_title"].toString());
    outdata["group_name"] = makeValue(ui->groupFilterCombo->currentText());
    outdata["specific_name"] = makeValue(data["specific_name"].toString());
    outdata["specific_id"] = data["specific_id"].toInt();
    outdata["tic_sim"] = makeValue(data["tic_sim"].toString());
    outdata["chemical_formula"] = makeValue(data["chemical_formula"].toString());
    outdata["coefficient_formula"] = makeValue(data["coefficient_formula"].toString());
    outdata["mz"] = makeValue(data["mz"].toString());
    outdata["dimension"] = makeValue(data["dimension"].toString());
    outdata["analysis_methodology"] = makeValue(data["analysis_methodology"].toString());
    outdata["bibliographical_reference"] = makeValue(data["bibliographical_reference"].toString());
    outdata["literature_source"] = makeValue(data["literature_source"].toString());
    int pdfFileId = data["pdf_id"].toInt();

    QString name = outdata["short_title"].toString();
    QString groupName = outdata["group_name"].toString();
    API::REST::AddNewOrModifyCompound(outdata,
      [name, groupName, this, pdfFileId, loadingOverlayId](QNetworkReply*, QJsonDocument doc)
      {
        auto obj = doc.object();
        if (obj["error"].toBool())
        {
          Notification::NotifyError(doc.object()["msg"].toString(), "Server error");
          GenesisWindow::Get()->RemoveOverlay(loadingOverlayId);
          return;
        }
        bool hasAnyAdditionalOperation = false;
        Notification::NotifySuccess(tr("Coefficient %1 added to group %2").arg(name).arg(groupName));
        leaveCoefficientModifying();
        int createdLibraryGroupId = obj["created_library_group_id"].toInt();
        auto pdfOp = ui->coefficientsModifyWidget->getPdfOperation();
        if (pdfOp == ModifyCompoundWidget::OPRemove)
        {
          removeAttachments(createdLibraryGroupId,
            pdfFileId,
            loadingOverlayId,
            pdfOp == ModifyCompoundWidget::OPRemove, false);
          connect(this, &ViewPageLibrary::coefficientUpdatingComplete, this,
            [this, createdLibraryGroupId]()
            {
              TreeModelDynamic::ResetInstances("TreeModelDynamicLibraryCompounds");
              ui->coefficientsWidget->setCurrentLibraryGroupId(createdLibraryGroupId);
            }, Qt::SingleShotConnection);
          hasAnyAdditionalOperation = true;
        }
        QString pdfPath;
        if (pdfOp == ModifyCompoundWidget::OPModify)
          pdfPath = ui->coefficientsModifyWidget->getPdfPath();

        if (!pdfPath.isEmpty())
        {
          bool hasAnyUploading = loadAttachments(createdLibraryGroupId, loadingOverlayId, pdfPath, "");
          if (!hasAnyUploading)
          {
            TreeModelDynamic::ResetInstances("TreeModelDynamicLibraryCompounds");
            ui->coefficientsWidget->setCurrentLibraryGroupId(obj["created_library_group_id"].toInt());
            GenesisWindow::Get()->RemoveOverlay(loadingOverlayId);
          }
          else
          {
            connect(this, &ViewPageLibrary::coefficientUpdatingComplete, this,
              [this, createdLibraryGroupId]()
              {
                TreeModelDynamic::ResetInstances("TreeModelDynamicLibraryCompounds");
                ui->coefficientsWidget->setCurrentLibraryGroupId(createdLibraryGroupId);
              }, Qt::SingleShotConnection);
          }
          hasAnyAdditionalOperation = true;
        }
        else
        {
          TreeModelDynamic::ResetInstances("TreeModelDynamicLibraryCompounds");
        }
        if (!hasAnyAdditionalOperation)
        {
          GenesisWindow::Get()->RemoveOverlay(loadingOverlayId);
        }

      },
      [loadingOverlayId](QNetworkReply*, QNetworkReply::NetworkError e)
      {
        Notification::NotifyError(tr("Network error"), e);
        GenesisWindow::Get()->RemoveOverlay(loadingOverlayId);
      });
  }

  void ViewPageLibrary::onCompoundAdditionConfirmed()
  {
    auto selModel = ui->compoundsAddTableView->selectionModel();
    auto currentInd = selModel->currentIndex();
    bool isNewCompound = false;
    if (!currentInd.isValid())
    {
      currentInd = mCompundAddingProxyModel->mapFromSource(mCompundAddingModel->MainItem()->GetIndex(1));
      isNewCompound = true;
    }
    else
    {
      isNewCompound = currentInd.row() == mCompundAddingProxyModel->mapFromSource(mCompundAddingModel->MainItem()->GetIndex(1)).row();
    }

    bool isWrongItemPicked = currentInd.row() == mCompundAddingProxyModel->mapFromSource(mCompundAddingModel->SubHeaderIndex()).row();
    if (isWrongItemPicked)
    {
      Notification::NotifyError(tr("Pick compound from table"));
      return;
    }

    auto currentItem = mCompundAddingModel->GetItem(mCompundAddingProxyModel->mapToSource(currentInd));
    auto pdfAttachment = ui->addingAddCompoundWidget->getAttachment();
    auto spectrumAttachment = ui->addingAddCompoundWidget->getSpectrumAttachment();
    QString pdfPath;
    QString specPath;
    if (pdfAttachment.exists())
      pdfPath = pdfAttachment.absoluteFilePath();

    if (spectrumAttachment.exists())
      specPath = spectrumAttachment.absoluteFilePath();

    auto loadingOverlayId = GenesisWindow::Get()->ShowOverlay(tr("Uploading data"));

    if (isNewCompound)
    {//add new compound
      QString validityErrorMsg;
      if (!checkInputValidity(mCompundAddingModel->MainItem()->GetData(), &validityErrorMsg))
      {
        Notification::NotifyError(validityErrorMsg, tr("Input error"));
        GenesisWindow::Get()->RemoveOverlay(loadingOverlayId);
        return;
      }
      auto data = mCompundAddingModel->MainItem()->GetData();//take data directly from new item
      QJsonObject outdata = QJsonObject::fromVariantMap(data);//TODO: check it
      auto makeValue = [](const QString& str)->QJsonValue
        {return str.isEmpty() ? QJsonValue() : QJsonValue(str); };
      outdata["element_type_id"] = 1;
      outdata["short_title"] = makeValue(data["short_title"].toString());
      outdata["full_title"] = makeValue(data["full_title"].toString());
      outdata["classifier_name"] = makeValue(data["classifier_title"].toString());
      outdata["compound_class_name"] = makeValue(data["compound_class_title"].toString());
      outdata["group_name"] = makeValue(ui->groupFilterCombo->currentText());
      outdata["specific_name"] = "Генезис";//TODO: find out wtf is it
      outdata["tic_sim"] = makeValue(data["tic_sim"].toString());
      outdata["chemical_formula"] = makeValue(data["chemical_formula"].toString());
      outdata["mz"] = makeValue(data["mz"].toString());
      outdata["dimension"] = makeValue(data["dimension"].toString());
      outdata["analysis_methodology"] = makeValue(data["analysis_methodology"].toString());
      outdata["bibliographical_reference"] = makeValue(data["bibliographical_reference"].toString());
      outdata["literature_source"] = makeValue(data["literature_source"].toString());

      QString name = outdata["short_title"].toString();
      QString groupName = outdata["group_name"].toString();
      API::REST::AddNewOrModifyCompound(outdata,
        [name, groupName, this, pdfPath, specPath, loadingOverlayId](QNetworkReply*, QJsonDocument doc)
        {
          auto obj = doc.object();
          if (obj["error"].toBool())
          {
            Notification::NotifyError(doc.object()["msg"].toString(), "Server error");
            GenesisWindow::Get()->RemoveOverlay(loadingOverlayId);
            return;
          }
          Notification::NotifySuccess(tr("Compound %1 added to group %2").arg(name).arg(groupName));
          leaveCompoundAdding();
          auto libraryGroupId = obj["created_library_group_id"].toInt();
          auto hasAnyUploading = loadAttachments(libraryGroupId, loadingOverlayId, pdfPath, specPath);
          if (!hasAnyUploading)
          {
            TreeModelDynamic::ResetInstances("TreeModelDynamicLibraryCompounds");
            ui->compoundsWidget->setCurrentLibraryGroupId(obj["created_library_group_id"].toInt());
          }
          else
          {
            connect(this, &ViewPageLibrary::compoundUpdatingComplete, this,
              [this, libraryGroupId]()
              {
                TreeModelDynamic::ResetInstances("TreeModelDynamicLibraryCompounds");
                ui->compoundsWidget->setCurrentLibraryGroupId(libraryGroupId);
              }, Qt::SingleShotConnection);
          }
          GenesisWindow::Get()->RemoveOverlay(loadingOverlayId);
        },
        [loadingOverlayId](QNetworkReply*, QNetworkReply::NetworkError e)
        {
          Notification::NotifyError(tr("Network error"), e);
          GenesisWindow::Get()->RemoveOverlay(loadingOverlayId);
        });
    }
    else
    {//add existed compound
      int id = currentItem->GetData("library_group_id").toInt();
      QString name = currentItem->GetData("short_title").toString();
      QString groupName = ui->groupFilterCombo->currentText();
      API::REST::AddExistedCompound(groupName, id,
        [name, groupName, this, loadingOverlayId](QNetworkReply*, QJsonDocument doc)
        {
          auto obj = doc.object();
          if (obj["error"].toBool())
          {
            Notification::NotifyError(doc.object()["msg"].toString(), "Server error");
            GenesisWindow::Get()->RemoveOverlay(loadingOverlayId);
            return;
          }
          Notification::NotifySuccess(tr("Compound %1 added to group %2").arg(name).arg(groupName));
          leaveCompoundAdding();
          TreeModelDynamic::ResetInstances("TreeModelDynamicLibraryCompounds");
          ui->compoundsWidget->setCurrentLibraryGroupId(obj["library_group_id"].toInt());
          GenesisWindow::Get()->RemoveOverlay(loadingOverlayId);
        },
        [loadingOverlayId](QNetworkReply*, QNetworkReply::NetworkError e)
        {
          Notification::NotifyError(tr("Network error"), e);
          GenesisWindow::Get()->RemoveOverlay(loadingOverlayId);
        });
    }
  }

  void ViewPageLibrary::onCoefficientAdditionConfirmed()
  {
    auto selModel = ui->coefficientAddTableView->selectionModel();
    auto currentInd = selModel->currentIndex();
    bool isNewCoefficient = false;
    if (!currentInd.isValid())
    {
      currentInd = mCoefficientAddingProxyModel->mapFromSource(mCoefficientAddingModel->MainItem()->GetIndex(1));
      isNewCoefficient = true;
    }
    else
    {
      isNewCoefficient = currentInd.row() == mCoefficientAddingProxyModel->mapFromSource(mCoefficientAddingModel->MainItem()->GetIndex(1)).row();
    }

    bool isWrongItemPicked = currentInd.row() == mCoefficientAddingProxyModel->mapFromSource(mCoefficientAddingModel->SubHeaderIndex()).row();
    if (isWrongItemPicked)
    {
      Notification::NotifyError(tr("Pick coefficint from table"));
      return;
    }

    auto currentItem = mCoefficientAddingModel->GetItem(mCoefficientAddingProxyModel->mapToSource(currentInd));
    auto pdfAttachment = ui->addingAddCoefficientsWidget->getAttachment();
    QString pdfPath;
    if (pdfAttachment.exists())
      pdfPath = pdfAttachment.absoluteFilePath();

    auto loadingOverlayId = GenesisWindow::Get()->ShowOverlay(tr("Uploading data"));

    if (isNewCoefficient)
    {//add new compound
      QString validityErrorMsg;
      if (!checkInputValidity(mCoefficientAddingModel->MainItem()->GetData(), &validityErrorMsg))
      {
        Notification::NotifyError(validityErrorMsg, tr("Input error"));
        GenesisWindow::Get()->RemoveOverlay(loadingOverlayId);
        return;
      }
      auto data = mCoefficientAddingModel->MainItem()->GetData();//take data directly from new item
      QJsonObject outdata = QJsonObject::fromVariantMap(data);//TODO: check it
      auto makeValue = [](const QString& str)->QJsonValue
        {return str.isEmpty() ? QJsonValue() : QJsonValue(str); };
      outdata["element_type_id"] = 2;
      outdata["short_title"] = makeValue(data["short_title"].toString());
      outdata["full_title"] = makeValue(data["full_title"].toString());
      outdata["classifier_name"] = makeValue(data["classifier_title"].toString());
      outdata["compound_class_name"] = makeValue(data["compound_class_title"].toString());
      outdata["group_name"] = makeValue(ui->groupFilterCombo->currentText());
      outdata["specific_name"] = makeValue(data["specific_name"].toString());
      outdata["specific_id"] = data["specific_id"].toInt();
      outdata["tic_sim"] = makeValue(data["tic_sim"].toString());
      outdata["chemical_formula"] = makeValue(data["chemical_formula"].toString());
      outdata["mz"] = makeValue(data["mz"].toString());
      outdata["dimension"] = makeValue(data["dimension"].toString());
      outdata["analysis_methodology"] = makeValue(data["analysis_methodology"].toString());
      outdata["bibliographical_reference"] = makeValue(data["bibliographical_reference"].toString());
      outdata["literature_source"] = makeValue(data["literature_source"].toString());

      QString name = outdata["short_title"].toString();
      QString groupName = outdata["group_name"].toString();
      API::REST::AddNewOrModifyCompound(outdata,
        [name, groupName, this, pdfPath, loadingOverlayId](QNetworkReply*, QJsonDocument doc)
        {
          auto obj = doc.object();
          if (obj["error"].toBool())
          {
            Notification::NotifyError(doc.object()["msg"].toString(), "Server error");
            GenesisWindow::Get()->RemoveOverlay(loadingOverlayId);
            return;
          }
          Notification::NotifySuccess(tr("Coefficient %1 added to group %2").arg(name).arg(groupName));
          leaveCoefficientAdding();
          auto libraryGroupId = obj["created_library_group_id"].toInt();
          auto hasAnyUploading = loadAttachments(libraryGroupId, loadingOverlayId, pdfPath, "");
          if (!hasAnyUploading)
          {
            TreeModelDynamic::ResetInstances("TreeModelDynamicLibraryCompounds");
            ui->coefficientsWidget->setCurrentLibraryGroupId(obj["created_library_group_id"].toInt());
            GenesisWindow::Get()->RemoveOverlay(loadingOverlayId);
          }
          else
          {
            connect(this, &ViewPageLibrary::coefficientUpdatingComplete, this,
              [this, libraryGroupId]()
              {
                TreeModelDynamic::ResetInstances("TreeModelDynamicLibraryCompounds");
                ui->coefficientsWidget->setCurrentLibraryGroupId(libraryGroupId);
              }, Qt::SingleShotConnection);
          }
          GenesisWindow::Get()->RemoveOverlay(loadingOverlayId);
        },
        [loadingOverlayId](QNetworkReply*, QNetworkReply::NetworkError e)
        {
          Notification::NotifyError(tr("Network error"), e);
          GenesisWindow::Get()->RemoveOverlay(loadingOverlayId);
        });
    }
    else
    {//add existed compound
      int id = currentItem->GetData("library_group_id").toInt();
      QString name = currentItem->GetData("short_title").toString();
      QString groupName = ui->groupFilterCombo->currentText();
      API::REST::AddExistedCoefficient(groupName, id,
        [name, groupName, this, loadingOverlayId](QNetworkReply*, QJsonDocument doc)
        {
          auto obj = doc.object();
          if (obj["error"].toBool())
          {
            Notification::NotifyError(doc.object()["msg"].toString(), "Server error");
            GenesisWindow::Get()->RemoveOverlay(loadingOverlayId);
            return;
          }
          Notification::NotifySuccess(tr("Coefficient %1 added to group %2").arg(name).arg(groupName));
          leaveCoefficientAdding();
          TreeModelDynamic::ResetInstances("TreeModelDynamicLibraryCompounds");
          ui->coefficientsWidget->setCurrentLibraryGroupId(obj["library_group_id"].toInt());
          GenesisWindow::Get()->RemoveOverlay(loadingOverlayId);
        },
        [loadingOverlayId](QNetworkReply*, QNetworkReply::NetworkError e)
        {
          Notification::NotifyError(tr("Network error"), e);
          GenesisWindow::Get()->RemoveOverlay(loadingOverlayId);
        });
    }
  }

  void ViewPageLibrary::updateCompoundSharingMenu()
  {
    auto saMenu = ShareAccessCompoundAction->menu();
    saMenu->clear();
    auto loadingAction = saMenu->addAction(tr("loading..."));
    loadingAction->setEnabled(false);
    int libraryGroupId = ui->compoundDetailsWidget->libraryGroupId();
    API::REST::Tables::GetAllowedGroups(libraryGroupId,
        [this, loadingAction, saMenu, libraryGroupId](QNetworkReply*, QVariantMap data)
        {
          saMenu->removeAction(loadingAction);
          auto children = data["children"].toList();
          for(auto& ch : children)
          {
            auto map = ch.toMap();
            auto title = map["group_title"].toString();
            auto action = saMenu->addAction(title);
            connect(action, &QAction::triggered, this, [this, libraryGroupId, title](){share(libraryGroupId, title);});
          }

        },
        [loadingAction, saMenu](QNetworkReply*, QNetworkReply::NetworkError)
        {
          saMenu->removeAction(loadingAction);
        });
  }

  void ViewPageLibrary::updateCoefficientSharingMenu()
  {
    auto saMenu = ShareAccessCoefficientAction->menu();
    saMenu->clear();
    auto loadingAction = saMenu->addAction(tr("loading..."));
    loadingAction->setEnabled(false);
    int libraryGroupId = ui->coefficientsDetailsWidget->libraryGroupId();
    API::REST::Tables::GetAllowedGroups(libraryGroupId,
        [this, loadingAction, saMenu, libraryGroupId](QNetworkReply*, QVariantMap data)
        {
          saMenu->removeAction(loadingAction);
          auto children = data["children"].toList();
          for(auto& ch : children)
          {
            auto map = ch.toMap();
            auto title = map["group_title"].toString();
            auto action = saMenu->addAction(title);
            connect(action, &QAction::triggered, this, [this, libraryGroupId, title](){share(libraryGroupId, title);});
          }

        },
        [loadingAction, saMenu](QNetworkReply*, QNetworkReply::NetworkError)
        {
          saMenu->removeAction(loadingAction);
        });
  }

  void ViewPageLibrary::share(int libraryGroupId, const QString &group)
  {
    API::REST::addLibraryGroupCompound({libraryGroupId}, group,
        [](QNetworkReply*, QJsonDocument json)
        {
          if (json["error"].toBool())
          {
            Notification::NotifyError(tr("Failed to share access from database"), json["msg"].toString());
          }
          else
          {
            Notification::NotifySuccess("Complete open access for compound", "Success");
            TreeModel::ResetInstances("TreeModelDynamicLibraryCompounds");
          }
        },
        [](QNetworkReply*, QNetworkReply::NetworkError err)
        {
          Notification::NotifyError(tr("Network error"), err);
        });
  }

  void ViewPageLibrary::removeCompound(int libraryGroupId, QString name)
  {
    auto dial = Dialogs::Templates::TreeConfirmation::DeleteCompoundsWithDependencies({{libraryGroupId, name}}, this);
    connect(dial, &WebDialog::Accepted, this,
            [this]()
            {
              ui->compoundsDetailsStackWgt->setCurrentIndex(CDPEmpty);
              TreeModel::ResetInstances("TreeModelDynamicLibraryCompounds");
            });
  }

  void ViewPageLibrary::removeCoefficient(int libraryGroupId, QString name)
  {
    auto dial = Dialogs::Templates::TreeConfirmation::DeleteCompoundsWithDependencies({{libraryGroupId, name}}, this);
    connect(dial, &WebDialog::Accepted, this,
            [this]()
            {
              ui->coefficientsDetailsStackWgt->setCurrentIndex(CDPEmpty);
              TreeModel::ResetInstances("TreeModelDynamicLibraryCompounds");
            });
  }

  // void ViewPageLibrary::updatePlotTemplatesMenu()
  // {

  // }

  bool ViewPageLibrary::loadAttachments(int libraryGroupId, QUuid overlayId, QString pdfPath, QString spectrumPath)
  {
    bool hasAnyUploading = false;
    if (!pdfPath.isEmpty())
    {
      QString requestKey = QUuid::createUuid().toString();
      addRequestAwait(requestKey, overlayId);
      QFile pdf(pdfPath);
      if (!pdf.open(QIODevice::ReadOnly))
      {
        Notification::NotifyError(tr("Cant open pdf attachment (%1)").arg(pdfPath), tr("File read error"));
        attachmentsLoadingErrorForId = libraryGroupId;
        closeRequest(requestKey);

        //TODO: error situation, switch to "modify" tab to let user resolve it
      }
      else
      {
        API::REST::uploadPdfAttachmentToCompound(libraryGroupId, pdf.readAll(), pdfPath,
          [this, pdfPath, requestKey, libraryGroupId](QNetworkReply*, QJsonDocument doc)
          {
            auto obj = doc.object();
            if (obj["error"].toBool())
            {
              Notification::NotifyError(doc.object()["msg"].toString(), "Server error");
              attachmentsLoadingErrorForId = libraryGroupId;
              closeRequest(requestKey);

              //TODO: error situation, switch to "modify" tab to let user resolve it

              return;
            }
            Notification::NotifySuccess(tr("file %1 uploaded").arg(pdfPath));
            closeRequest(requestKey);
          },
          [this, requestKey, libraryGroupId](QNetworkReply*, QNetworkReply::NetworkError e)
          {
            Notification::NotifyError(tr("Network error"), e);
            attachmentsLoadingErrorForId = libraryGroupId;
            closeRequest(requestKey);
            //TODO: error situation, switch to "modify" tab to let user resolve it
          });
        hasAnyUploading = true;
      }
    }
    if (!spectrumPath.isEmpty())
    {
      QString requestKey = QUuid::createUuid().toString();
      addRequestAwait(requestKey, overlayId);
      QFile spectrum(spectrumPath);
      if (!spectrum.open(QIODevice::ReadOnly))
      {
        Notification::NotifyError(tr("Cant open pdf attachment (%1)").arg(spectrumPath), tr("File read error"));
        attachmentsLoadingErrorForId = libraryGroupId;
        closeRequest(requestKey);

        //TODO: error situation, switch to "modify" tab to let user resolve it
      }
      else
      {
        API::REST::uploadSpectrumAttachmentToCompound(libraryGroupId, spectrum.readAll(), spectrumPath,
          [this, requestKey, spectrumPath, libraryGroupId](QNetworkReply*, QJsonDocument doc)
          {
            auto obj = doc.object();
            if (obj["error"].toBool())
            {
              Notification::NotifyError(doc.object()["msg"].toString(), "Server error");
              attachmentsLoadingErrorForId = libraryGroupId;
              closeRequest(requestKey);

              //TODO: error situation, switch to "modify" tab to let user resolve it

              return;
            }
            Notification::NotifySuccess(tr("file %1 uploaded").arg(spectrumPath));
            closeRequest(requestKey);
          },
          [this, requestKey, libraryGroupId](QNetworkReply*, QNetworkReply::NetworkError e)
          {
            Notification::NotifyError(tr("Network error"), e);
            attachmentsLoadingErrorForId = libraryGroupId;
            closeRequest(requestKey);
          });
        hasAnyUploading = true;
      }
    }
    return hasAnyUploading;
  }

  void ViewPageLibrary::removeAttachments(int libraryGroupId, int pdfFileId, QUuid overlayId, bool pdfToRemove, bool spectrumToRemove)
  {
    if (pdfToRemove)
    {
      QString requestKey = QUuid::createUuid().toString();
      addRequestAwait(requestKey, overlayId);
      if (pdfFileId > 0)
      {
        API::REST::removePdfAttachmentFromCompound(pdfFileId,
          [this, requestKey, pdfFileId, libraryGroupId](QNetworkReply*, QJsonDocument doc)
          {
            auto obj = doc.object();
            if (obj["error"].toBool())
            {
              Notification::NotifyError(doc.object()["msg"].toString(), "Server error");
              ui->compoundsWidget->setCurrentLibraryGroupId(libraryGroupId);
              TreeModelDynamic::ResetInstances("TreeModelDynamicLibraryCompounds");
              closeRequest(requestKey);
              return;
            }
            Notification::NotifySuccess(tr("file %1 removed").arg(pdfFileId));
            ui->compoundsWidget->setCurrentLibraryGroupId(libraryGroupId);
            TreeModelDynamic::ResetInstances("TreeModelDynamicLibraryCompounds");
            closeRequest(requestKey);
          },
          [this, requestKey, libraryGroupId](QNetworkReply*, QNetworkReply::NetworkError e)
          {
            Notification::NotifyError(tr("Network error"), e);
            ui->compoundsWidget->setCurrentLibraryGroupId(libraryGroupId);
            TreeModelDynamic::ResetInstances("TreeModelDynamicLibraryCompounds");
            closeRequest(requestKey);
          });
      }
    }
    if (spectrumToRemove)
    {
      QString requestKey = QUuid::createUuid().toString();
      addRequestAwait(requestKey, overlayId);
      API::REST::removeSpectrumAttachmentFromCompound(libraryGroupId,
        [this, requestKey, libraryGroupId](QNetworkReply*, QJsonDocument doc)
        {
          auto obj = doc.object();
          if (obj["error"].toBool())
          {
            Notification::NotifyError(doc.object()["msg"].toString(), "Server error");
            ui->compoundsWidget->setCurrentLibraryGroupId(libraryGroupId);
            TreeModelDynamic::ResetInstances("TreeModelDynamicLibraryCompounds");
            closeRequest(requestKey);
            return;
          }
          Notification::NotifySuccess(tr("spectrum removed from %1").arg(libraryGroupId));
          ui->compoundsWidget->setCurrentLibraryGroupId(libraryGroupId);
          TreeModelDynamic::ResetInstances("TreeModelDynamicLibraryCompounds");
          closeRequest(requestKey);
        },
        [this, requestKey, libraryGroupId](QNetworkReply*, QNetworkReply::NetworkError e)
        {
          Notification::NotifyError(tr("Network error"), e);
          ui->compoundsWidget->setCurrentLibraryGroupId(libraryGroupId);
          TreeModelDynamic::ResetInstances("TreeModelDynamicLibraryCompounds");
          closeRequest(requestKey);
        });
    }
  }

  void ViewPageLibrary::addRequestAwait(const QString& requestKey, QUuid overlayId)
  {
    awaitingRequestsMutex.lock();
    mAwaitingRequests[overlayId] << requestKey;
    awaitingRequestsMutex.unlock();
  }

  void ViewPageLibrary::closeRequest(const QString& requestKey)
  {
    bool emitErrorSignal = false;
    bool emitFinishSignal = false;
    awaitingRequestsMutex.lock();
    for (auto iter = mAwaitingRequests.keyValueBegin(); iter != mAwaitingRequests.keyValueEnd(); iter++)
    {
      if (iter->second.contains(requestKey))
      {
        iter->second.removeOne(requestKey);
        if (iter->second.isEmpty())
        {
          GenesisWindow::Get()->RemoveOverlay(iter->first);
          mAwaitingRequests.remove(iter->first);
          if (attachmentsLoadingErrorForId != -1)
          {
            emitErrorSignal = true;
          }
          else
          {
            emitFinishSignal = true;
          }
        }
        break;
      }
    }
    awaitingRequestsMutex.unlock();
    if (emitErrorSignal)
    {
      int tmp = attachmentsLoadingErrorForId;
      attachmentsLoadingErrorForId = -1;
      emit attachmentsLoadingError(tmp);
    }
    else if (emitFinishSignal)
    {
      emit compoundUpdatingComplete();
    }
  }

  void ViewPageLibrary::tabWidgetIndexChanged(int index)
  {
    isTabUpdateInProgress = true;
    Core::GenesisContextRoot::Get()->GetContextLibrary()->SetData(Names::LibraryContextTags::kCurrentTabInd, index);
    isTabUpdateInProgress = false;
    if (index == static_cast<int>(Tab::PlotTemplates))
    {
      ui->stwdtPlotTemplate->setVisible(true);
      ui->tabWidget->setCurrentWidget(ui->templatesTab);
      ui->stwdtPlotTemplate->setCurrentWidget(ui->templateListWidget);
    }
    if (index > static_cast<int>(Tab::PlotTemplates))
      return;

    auto notCompound = index > static_cast<int>(Tab::Compounds);
    ui->specificsFilterCombo->setEnabled(notCompound);
    ui->specificsFilterCombo->setToolTip(notCompound
      ? ""
      : tr("Specifics are disabled for \"Compounds\" tab"));
  }

  void ViewPageLibrary::ApplyContextLibrary(const QString& dataId, const QVariant& data)
  {
    auto isReset = isDataReset(dataId, data);
    if (isReset)
    {
      ui->tabWidget->setCurrentIndex(Core::GenesisContextRoot::Get()->GetContextLibrary()->GetData(Names::LibraryContextTags::kCurrentTabInd).toInt());
    }
    else if (!isTabUpdateInProgress && dataId == Names::LibraryContextTags::kCurrentTabInd)
    {
      ui->tabWidget->setCurrentIndex(data.toInt());
    }
    applyComboFilter();
  }

  void ViewPageLibrary::ApplyContextUser(const QString& dataId, const QVariant& data)
  {
    bool isReset = isDataReset(dataId, data);

    if (isReset)
    {
      QStringList availableGroups = Core::GenesisContextRoot::Get()->GetContextUser()->GetData(Names::UserContextTags::kGroups).toStringList();
      if (dataId == Names::UserContextTags::kGroups)
        availableGroups = data.toStringList();

      ui->groupFilterCombo->blockSignals(true);
      ui->groupFilterCombo->clear();
      if (!availableGroups.isEmpty())
      {
        ui->groupFilterCombo->addItems(availableGroups);
        ui->groupFilterCombo->setCurrentIndex(0);
      }
      ui->groupFilterCombo->blockSignals(false);
      applyComboFilter();
      if (MakeSystemCompoundAction)
        MakeSystemCompoundAction->setVisible(Core::GenesisContextRoot::Get()->GetContextUser()->GetData(Names::UserContextTags::kSuperuser).toBool());
      if (MakeSystemCoefficientAction)
        MakeSystemCoefficientAction->setVisible(Core::GenesisContextRoot::Get()->GetContextUser()->GetData(Names::UserContextTags::kSuperuser).toBool());
    }
    else if (dataId == Names::UserContextTags::kCurrentGroup)
    {
      Structures::LibraryFilterState state =
          {
              ui->searchLineEdit->text(),
              data.toString(),
              static_cast<LibraryFilter::FileType>(ui->filesFilterCombo->currentIndex()),
              static_cast<LibraryFilter::Classifiers>(ui->classifiersFilterCombo->currentIndex()),
              static_cast<LibraryFilter::Specifics>(ui->specificsFilterCombo->currentIndex())
          };
      ui->compoundsWidget->FilterByState(state);
      ui->coefficientsWidget->FilterByState(state);
      ui->templateListWidget->FilterByState(state);
    }
    else if (dataId == Names::UserContextTags::kSuperuser)
    {
      if (MakeSystemCompoundAction)
        MakeSystemCompoundAction->setVisible(data.toBool());
      if (MakeSystemCoefficientAction)
        MakeSystemCoefficientAction->setVisible(data.toBool());
    }
  }

  void ViewPageLibrary::updateTemplateAxisList()
  {
    auto compounds = ui->compoundsWidget->GetCompoundKeys();
    auto coefficients = ui->coefficientsWidget->GetCoefficientKeys();
    std::copy(compounds.begin(), compounds.end(), std::back_inserter(coefficients));
    std::sort(coefficients.begin(), coefficients.end(),
              [](const Structures::KeyString& item1, const Structures::KeyString& item2) -> bool {
      return item1.fullName < item2.fullName;
    });
    ui->templateListWidget->setAxesList(coefficients);
  }

  bool Views::ViewPageLibrary::eventFilter(QObject *watched, QEvent *event)
  {
    if(event->type() == QEvent::Wheel)
    {
      if(watched->isWidgetType() && !((QWidget*)watched)->hasFocus())
      {
        event->ignore();
        return true;
      }
    }
    return false;
  }

}//namespace Views

