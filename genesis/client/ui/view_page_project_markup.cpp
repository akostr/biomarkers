#include "view_page_project_markup.h"

#include <genesis_style/style.h>

#include "logic/markup/baseline_data_model.h"
#include "logic/markup/commands.h"
#include "logic/tree_model.h"
#include "logic/markup/genesis_markup_forward_declare.h"
#include "logic/genesis_context_step_based_markup.h"
#include "logic/context_root.h"
#include "logic/markup/markup_data_model.h"
#include "logic/markup/sub_markup_step_controller.h"
#include "logic/markup/biomarkers_markup_step_controller.h"
#include "logic/markup/biomarkers_sub_markup_step_controller.h"

#include "ui/dialogs/markup_save_as_dialog.h"
#include "ui/dialogs/markup_save_table_dialog.h"
#include <ui/widgets/markup/dialog_add_chromatogramms.h>
#include <ui/widgets/markup/stepper_widget.h>
#include "ui/widgets/markup/chromatogram_widget_refactor.h"
#include "ui/widgets/markup/table_marker_widget.h"
#include <ui/widgets/markup/gp_stackedwidget.h>
#include "ui/widgets/markup/numeric_data_table.h"
#include "ui/widgets/markup/reservoir_markup_widget.h"
#include "ui/widgets/markup/identification_widget.h"
#include "known_view_names.h"
#include "logic/known_context_tag_names.h"
#include "ui/dialogs/templates/save_edit.h"
#include <ui/dialogs/markup/intermarkers_transfer_dialog.h>
#include <ui/dialogs/web_dialog_ratio_matrix_settings.h>
#include <ui/dialogs/markup/web_dialog_add_chromatogramms.h>
#include <ui/dialogs/templates/confirm.h>
#include <ui/dialogs/markup/biomarkers_markup_save_table_dialog.h>
#include <ui/genesis_window.h>

#include <QLabel>
#include <QToolBar>
#include <QVBoxLayout>
#include <QStackedLayout>
#include <ui/pin_layout.h>
#include <QScrollArea>
#include <QToolBar>
#include <QComboBox>
#include <QLabel>
#include <QPushButton>
#include <QMenu>
#include <QHBoxLayout>
#include <QTableView>
#include <ui/dialogs/web_dialog_intermarkers_transfer.h>
#include <ui/dialogs/web_dialog_markers_transfer.h>
#include <logic/notification.h>
#include <logic/known_json_tag_names.h>

#include "ui_view_page_project_markup.h"

#include <QSet>

using namespace GenesisMarkup;
using namespace Widgets;
using namespace Names;
using namespace Core;

template <typename T>
QString enumToStr(T value)
{
  return QMetaEnum::fromType<T>().valueToKey(value);
}

ViewPageProjectMarkup::ViewPageProjectMarkup(QWidget *parent)
  : View(parent),
  ui(new Ui::MarkupPage)
{
  SetupUi();
}

ViewPageProjectMarkup::~ViewPageProjectMarkup()
{
  auto deleteController = [this](MarkupStepController* controller)
  {
    if(!controller)
      return;
    disconnect(controller, nullptr, this, nullptr);
    delete controller;
  };
  mCurrentController = nullptr;
  deleteController(mMainController);
  deleteController(mSubController);
  delete ui;
}

void ViewPageProjectMarkup::SetupUi()
{
  setStyleSheet(Style::Genesis::GetUiStyle());
  ui->setupUi(this);
  //// Self
  ui->splitter->setStretchFactor(0,0);
  ui->splitter->setStretchFactor(1,1);
  ui->splitter->handle(1)->setAttribute(Qt::WA_Hover);
  ui->stepperWidget->setStyleSheet(".QWidget {background: white; border-top-left-radius: 8px; border-top-right-radius: 8px; border-bottom-right-radius: 8px; border-bottom-left-radius: 8px;}");
  ui->steppersStackedWidget->setStyleSheet("background: white;");
  ui->prevStepButton->setProperty("primary", true);
  ui->nextStepButton->setProperty("blue", true);
  ui->completeButton->setProperty("blue", true);
  ui->detectPeaksButton->setProperty("blue", true);
  ui->configurateMarkupButton->setProperty("menu_secondary", true);
  ui->tabBackgroundWidget->setAutoFillBackground(true);
  ui->tabBackgroundWidget->setStyleSheet("QWidget {background-color: #ffffff; border-radius: 8px}");

  if (auto widgetStyle = style())
  {
    widgetStyle->polish(ui->prevStepButton);
    widgetStyle->polish(ui->nextStepButton);
    widgetStyle->polish(ui->completeButton);
    widgetStyle->polish(ui->detectPeaksButton);
    widgetStyle->polish(ui->configurateMarkupButton);
    widgetStyle->polish(ui->tabBackgroundWidget);
  }
  // markup widget
  {
    MarkupWidget = new GenesisMarkup::ReservoirMarkupWidget(this);

    connect(MarkupWidget, &ReservoirMarkupWidget::intervalsModeChanged, this, &ViewPageProjectMarkup::onIntervalsModeChanged);
    connect(MarkupWidget, &ReservoirMarkupWidget::remarkupCalled, this, &ViewPageProjectMarkup::onRemarkupCalled);
    connect(MarkupWidget, &ReservoirMarkupWidget::newCommand, this, &ViewPageProjectMarkup::onNewCommand);

    ui->tabBackgroundWidget->layout()->addWidget(MarkupWidget);
    MarkupWidget->addTab(new TableMarkerWidget(this), tr("Marker table"));
    MarkupWidget->addTab(new NumericDataTable(this), tr("Numeric table data"));

    // ui->tabWidget->AddTabWidget(MarkersTableWidget, tr("Marker table"), ChromatogrammsTabWidget::markersTable);
    // ui->tabWidget->AddTabWidget(new NumericDataTable(this), tr("Numeric table data"), ChromatogrammsTabWidget::numericTable);
    // connect(ui->tabWidget, &QTabWidget::currentChanged, [this]{
    //   onStartupLoadPlots();//OMG, WHY? WTF?!
    // });
  }
  // Caption & toolbar
  {
    ui->markupCaptionLabel->setStyleSheet(Style::Genesis::GetH1());

    ////Markup toolbar
    {
      ui->detectPeaksButton->setVisible(false);
      connect(ui->detectPeaksButton, &QPushButton::clicked,
              [this](){
                auto cmd = new NextStepCommand(mCurrentController,
                                               mCurrentController->getCurrentModel(),
                                               mCurrentController->getCurrentStepInfo().step);
                mCurrentController->pushCommand(cmd);
              });

      ConfigurateMarkupButtonMenu = new QMenu(ui->configurateMarkupButton);
      SaveAction = new QAction(tr("Save"), ConfigurateMarkupButtonMenu);
      SaveAction->setShortcut(QKeySequence(QKeyCombination(Qt::ControlModifier, Qt::Key_S)));
      //SaveAction->setIcon(QIcon("://resource/icons/icon_action_save.png"));
      connect(SaveAction, &QAction::triggered, this, [this]()
              {
                SaveMarkupStateOnBack();
              });
      ConfigurateMarkupButtonMenu->addAction(SaveAction);

      SaveAsAction = new QAction(tr("Save as"), ConfigurateMarkupButtonMenu);
      //SaveAsAction->setIcon(QIcon("://resource/icons/icon_action_save.png"));
      SaveAsAction->setShortcut(QKeySequence(QKeyCombination(Qt::ControlModifier | Qt::ShiftModifier, Qt::Key_S)));
      connect(SaveAsAction, &QAction::triggered, this, [this]()
              {
                SaveMarkupAs(true);
              });
      ConfigurateMarkupButtonMenu->addAction(SaveAsAction);

      RemarkupAction = new QAction(tr("ReMarkup chromatogramms"), ConfigurateMarkupButtonMenu);
      //RemarkupAction->setIcon(QIcon("://resource/icons/icon_remarkup_chroms.png"));
      connect(RemarkupAction, &QAction::triggered, this, [this]()
              {
                //TODO: make proper dialog for that
                auto content = new QWidget();
                content->setStyleSheet(Style::Genesis::GetUiStyle());
                auto layout = new QVBoxLayout();
                content->setLayout(layout);
                auto caption = new QLabel(tr("Pick chromatogramms for remarkup:"),content);
                caption->setStyleSheet(Style::Genesis::GetH2());
                layout->addWidget(caption);

                auto scrollArea = new QScrollArea(content);
                scrollArea->setMinimumHeight(150);
                auto scrollLayout = new QVBoxLayout();
                auto scrollContent = new QWidget(scrollArea);
                scrollContent->setMinimumSize(150,150);
                scrollLayout->setSizeConstraint(QLayout::SetMinAndMaxSize);
                scrollContent->setLayout(scrollLayout);

                QList<QCheckBox*> cbList;
                auto currentMarkupModel = mCurrentController->getCurrentModel();
                for(auto& chroma : currentMarkupModel->getChromatogrammsList())
                {
                  if(currentMarkupModel->hasMaster() && chroma.first == currentMarkupModel->getMaster()->id())
                    continue;

                  auto passport = mCurrentController->passportsModel().getPassport(chroma.first);
                  QCheckBox* cBox(nullptr);
                  if(passport)
                    cBox = new QCheckBox(passport->getData(PassportFilename).toString());
                  else
                    cBox = new QCheckBox(tr("#$%@!NAME ERROR! NO PASSPORT DATA!@%$#"));

                  cBox->setProperty("id", chroma.first);
                  cbList << cBox;
                  scrollLayout->addWidget(cBox);
                  scrollLayout->setStretch(scrollLayout->count() - 1, 0);
                }
                scrollLayout->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding));
                scrollLayout->setStretch(scrollLayout->count() - 1, 1);

                scrollArea->setWidget(scrollContent);
                scrollArea->setWidgetResizable(true);
                layout->addWidget(scrollArea);

                auto dial = new WebDialog(this, content, QSizeF(), QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
                connect(dial, &WebDialog::Accepted, this, [this, cbList]()
                        {
                          QList<int> ids;
                          for(auto& cBox : cbList)
                          {
                            if(cBox->isChecked())
                              ids << cBox->property("id").toInt();
                          }
                          if(ids.isEmpty())
                            return;
                          ReMarkupChromatogramms(ids);
                        });

                dial->Open();
              });
      ConfigurateMarkupButtonMenu->addAction(RemarkupAction);

      ImportChromatogrammsAction = new QAction(tr("Import"), ConfigurateMarkupButtonMenu);
      //ImportChromatogrammsAction->setIcon(QIcon("://resource/icons/icon_action_add_chroms.png"));
      connect(ImportChromatogrammsAction, &QAction::triggered, this, [this]()
              {
                ImportChromatogramms();
              });
      ConfigurateMarkupButtonMenu->addAction(ImportChromatogrammsAction);

      ToDataTableAction = new QAction(tr("To data table"), ConfigurateMarkupButtonMenu);
      //ToDataTableAction->setIcon(QIcon("://resource/icons/icon_action_to_data_table.png"));
      connect(ToDataTableAction, &QAction::triggered, this, &ViewPageProjectMarkup::TryToSwitchToTable);
      ConfigurateMarkupButtonMenu->addAction(ToDataTableAction);
      {
        auto titleModeMenu = ConfigurateMarkupButtonMenu->addMenu(tr("Show..."));
        auto actionGroup = new QActionGroup(titleModeMenu);
        actionGroup->setExclusive(true);
        TitleModeOrderAction = titleModeMenu->addAction(tr("Order number"));
        TitleModeOrderAction->setActionGroup(actionGroup);
        TitleModeOrderAction->setCheckable(true);
        TitleModeOrderAction->setProperty("mode", ChromatogrammTitlesDisplayModes::TitleModeOrder);
        connect(TitleModeOrderAction, &QAction::toggled, this, &ViewPageProjectMarkup::onUserChangedTitlesMode);
        TitleModeIndexAction = titleModeMenu->addAction(tr("Index"));
        TitleModeIndexAction->setActionGroup(actionGroup);
        TitleModeIndexAction->setCheckable(true);
        TitleModeIndexAction->setProperty("mode", ChromatogrammTitlesDisplayModes::TitleModeIndex);
        connect(TitleModeIndexAction, &QAction::toggled, this, &ViewPageProjectMarkup::onUserChangedTitlesMode);
        TitleModeRetTimeAction = titleModeMenu->addAction(tr("Retention time"));
        TitleModeRetTimeAction->setActionGroup(actionGroup);
        TitleModeRetTimeAction->setCheckable(true);
        TitleModeRetTimeAction->setProperty("mode", ChromatogrammTitlesDisplayModes::TitleModeRetentionTime);
        connect(TitleModeRetTimeAction, &QAction::toggled, this, &ViewPageProjectMarkup::onUserChangedTitlesMode);
        TitleModeCompoundAction = titleModeMenu->addAction(tr("Compound name"));
        TitleModeCompoundAction->setActionGroup(actionGroup);
        TitleModeCompoundAction->setCheckable(true);
        TitleModeCompoundAction->setProperty("mode", ChromatogrammTitlesDisplayModes::TitleModeCompoundTitle);
        connect(TitleModeCompoundAction, &QAction::toggled, this, &ViewPageProjectMarkup::onUserChangedTitlesMode);
        TitleModeCompoundAction->setVisible(false);

        TitleModeOrderAction->setChecked(true);
      }
      ConfigurateMarkupButtonMenu->addSeparator();

      auto setPlotHeightAction = new QWidgetAction(ConfigurateMarkupButtonMenu);
      {
        auto wgt = new QWidget(ConfigurateMarkupButtonMenu);
        setPlotHeightAction->setDefaultWidget(wgt);
        auto ml = new QVBoxLayout();
        auto menuPadding = Style::GetSASSValue("menuPadding").remove("px").toInt();
        ml->setContentsMargins(menuPadding + 1,menuPadding,menuPadding,menuPadding);
        // ml->setContentsMargins(6,10,2,12);
        ml->setSpacing(10);
        wgt->setLayout(ml);
        auto l = new QHBoxLayout();
        l->setContentsMargins(0,0,0,0);
        ml->addStretch();
        ml->addLayout(l);
        ml->addStretch();
        wgt->setStyleSheet("QWidget {background-color: transparent;}");
        auto label = new QLabel(tr("Set plot height:"), wgt);
        l->addWidget(label);
        l->addStretch();
        auto heightSpinBox = new QSpinBox(wgt);
        l->addWidget(heightSpinBox);
        heightSpinBox->setSuffix(" px");
        heightSpinBox->setMinimum(50);
        heightSpinBox->setMaximum(1000);
        heightSpinBox->setSingleStep(10);
        heightSpinBox->setValue(MarkupWidget->chromaHeight());
        heightSpinBox->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
        heightSpinBox->setProperty("tiny", true);
        heightSpinBox->adjustSize();
        setPlotHeightAction->setDefaultWidget(wgt);
        connect(heightSpinBox, &QSpinBox::valueChanged, this, [this](int value)
                {
                  MarkupWidget->setChromaHeight(value);
                  if(IdentificationTabWidget)
                  {
                    IdentificationTabWidget->setChromaHeight(value);
                  }
                });
      }
      ConfigurateMarkupButtonMenu->addAction(setPlotHeightAction);

      ui->configurateMarkupButton->setMenu(ConfigurateMarkupButtonMenu);
    }
  }

  ui->prevStepButton->setVisible(false);

  ui->nextStepButton->setVisible(false);
  ui->completeButton->setVisible(false);

  connect(ui->prevStepButton, &QPushButton::clicked, this,
          [this]()
          {
            mCurrentController->StepBackUiSlot();
          });

  connect(ui->nextStepButton, &QPushButton::clicked, this, &ViewPageProjectMarkup::onStepForward);
  connect(ui->completeButton, &QPushButton::clicked, this, &ViewPageProjectMarkup::onStepForward);
}

void ViewPageProjectMarkup::onMainModelChanged(MarkupModelPtr model, const StepInfo& stepInfo)
{
  handleUiInteractions(stepInfo);
  MarkupWidget->setMainMarkupModel(model, stepInfo);
  if(mFirstShow)
  {
    mFirstShow = false;
    MarkupWidget->setPlotRanges(MarkupWidget->wholeDataRange());
  }
}

void ViewPageProjectMarkup::onSubModelChanged(MarkupModelPtr model, const StepInfo &stepInfo)
{
  handleUiInteractions(stepInfo);
  MarkupWidget->setSubMarkupModel(model, stepInfo);
  if(mFirstShow)
  {
    mFirstShow = false;
    MarkupWidget->setPlotRanges(MarkupWidget->wholeDataRange());
  }
}

void ViewPageProjectMarkup::handleUiInteractions(const StepInfo &stepInfo)
{
  auto handle = [stepInfo](StepGuiInteractions flag, QWidget* wgt)
  {
    if(stepInfo.guiInteractions.testFlag(flag))
    {
      if(wgt->isHidden())
        wgt->show();
    }
    else
    {
      if(!wgt->isHidden())
        wgt->hide();
    }
  };
  auto handleAction = [stepInfo](StepGuiInteractions flag, QAction* a)
  {
    if(stepInfo.guiInteractions.testFlag(flag))
    {
      if(!a->isEnabled())
        a->setEnabled(true);
    }
    else
    {
      if(a->isEnabled())
        a->setEnabled(false);
    }
  };

  handle(SIGDetectPeaks, ui->detectPeaksButton);
  handle(SIGPrevStep, ui->prevStepButton);
  handle(SIGNextStep, ui->nextStepButton);
  handle(SIGComplete, ui->completeButton);
  handleAction(SIGRemarkupSlaves, RemarkupAction);
  handleAction(SIGSaveMarkup, SaveAction);
  handleAction(SIGSaveMarkupAs, SaveAsAction);
  handleAction(SIGImportChromatogramms, ImportChromatogrammsAction);
  handleAction(SIGSwitchToTable, ToDataTableAction);
}

void ViewPageProjectMarkup::ApplyContextMarkup(const QString& dataId, const QVariant& data)
{
  bool isReset = isDataReset(dataId, data);
  if(!isReset)
  {
    if(dataId == Names::MarkupContextTags::kMarkupTitle)
    {
      setViewHeader(data.toString());
    }
    else if(dataId == Names::MarkupContextTags::kMarkupId)
    {
      clearView();
      if (!data.isValid() || data.isNull() || data.toInt() == 0)
        return;
      //markup has static curves array because curves can not be changed, and always the same.
      MarkupDataModel::clearCurves();
      //move meta data to controller
      MarkupId = data.toInt();
      if(!loadingIsPending)
      {
        loadingIsPending = true;
        QMetaObject::invokeMethod(this, "loadMarkup", Qt::QueuedConnection, MarkupId);
      }
      // mMainController->LoadMarkup(MarkupId);
    }
    else if(dataId == Names::MarkupContextTags::kPeakTitleDisplayMode)
    {
      updateTitleMode(data.value<TChromatogrammTitlesDisplayMode>());
    }
  }
  else
  {
    auto context = GenesisContextRoot::Get()->GetContextMarkup();
    clearView();
    auto idData = context->GetData(Names::MarkupContextTags::kMarkupId);
    auto header = context->GetData()[Names::MarkupContextTags::kMarkupTitle].toString();
    auto mode = context->GetData(Names::MarkupContextTags::kPeakTitleDisplayMode).value<TChromatogrammTitlesDisplayMode>();
    setViewHeader(header);
    updateTitleMode(mode);

    if (!idData.isValid() || idData.isNull() || idData.toInt() == 0)
      return;
    //markup has static curves array because curves can not be changed, and always the same.
    MarkupDataModel::clearCurves();
    MarkupId = idData.toInt();
    if(!loadingIsPending)
    {
      loadingIsPending = true;
      QMetaObject::invokeMethod(this, "loadMarkup", Qt::QueuedConnection, MarkupId);
    }
    // mMainController->LoadMarkup(MarkupId);
  }

#ifdef RENDERDOC_API_DEBUG
  rdoc_api = nullptr;
  qDebug() << Q_FUNC_INFO;

  if(HMODULE mod = GetModuleHandleA("renderdoc.dll"))
  {
    qDebug() << "INIT SUCCESS";
    pRENDERDOC_GetAPI RENDERDOC_GetAPI =
      (pRENDERDOC_GetAPI)GetProcAddress(mod, "RENDERDOC_GetAPI");
    int ret = RENDERDOC_GetAPI(eRENDERDOC_API_Version_1_6_0, (void **)&rdoc_api);
    assert(ret == 1);
  }

  if(rdoc_api){
    rdoc_api->SetActiveWindow(nullptr, HWND(winId()));
    qDebug() << " in if(rdoc_api)";
    rdoc_api->StartFrameCapture(NULL, NULL);
    qDebug() << "IsFrameCapturing " << rdoc_api->IsFrameCapturing();
    qDebug() << "IsRemoteAccessConnected " << rdoc_api->IsRemoteAccessConnected();
    qDebug() << "IsTargetControlConnected " << rdoc_api->IsTargetControlConnected();
  }
#endif
}

void ViewPageProjectMarkup::ReMarkupChromatogramms(const QList<int> ids, const ChromaSettings *initialSettings)
{
  auto subStepper = new StepperWidget();
  auto ind = ui->steppersStackedWidget->addWidget(subStepper);
  ui->steppersStackedWidget->setCurrentIndex(ind);
  auto currentStep = mCurrentController->getCurrentStepInfo().step;

  auto currentModule = (Names::ModulesContextTags::Module)GenesisContextRoot::Get()->GetContextModules()->GetData(Names::ModulesContextTags::kModule).toInt();
  MarkupStepController* subController = nullptr;
  switch(currentModule)
  {
  case ModulesContextTags::MNoModule:
  case ModulesContextTags::MReservoir:
  case ModulesContextTags::MPlots:
  case ModulesContextTags::MLast:
  default:
  {
    subController = new SubMarkupStepController(currentStep, mCurrentController);
    break;
  }
  case ModulesContextTags::MBiomarkers:
  {
    subController = new BiomarkersSubMarkupStepController(currentStep, mCurrentController);
    break;
  }
  }

  subController->initialize();
  //TODO: make this on pointers, remove data copying:
  subController->setExternalPassportsModel(mCurrentController->passportsModelPtr());
  //
  setStepperToController(subStepper, subController);
  auto oldController = mCurrentController;
  setNewController(subController);

  connect(subController, &QObject::destroyed, this, [this]()
          {
            ui->steppersStackedWidget->setCurrentIndex(0);
          });

  connect(subController, &SubMarkupStepController::MarkupComplete, this, [oldController, subController, this]
    {
      oldController->merge(*subController);
      delete subController;
    }, Qt::QueuedConnection);

  auto newModel = oldController->getCopyWithoutMarkupWithExternalMasterAndIntervals(currentStep, ids);
  if(initialSettings)
  {
    for(auto& id : ids)
    {
      //silently update chroma settings to initial settings
      auto& chroma = const_cast<QHash<int, QVariant>&>(newModel->getChromatogramm(id)->internalDataReference());
      chroma[ChromatogrammSettingsBaseLineSmoothLvl] = initialSettings->BLineSmoothLvl;
      chroma[ChromatogrammSettingsMinimalPeakHeight] = initialSettings->Min_h;
      chroma[ChromatogrammSettingsNoisy] = initialSettings->Noisy;
      chroma[ChromatogrammSettingsMedianFilterWindowSize] = initialSettings->Window_size;
      chroma[ChromatogrammSettingsSignalToMedianRatio] = initialSettings->Sign_to_med;
      chroma[ChromatogrammSettingsDegreeOfApproximation] = initialSettings->Doug_peuck;
      chroma[ChromatogrammSettingsBoundsByMedian] = initialSettings->Med_bounds;
      chroma[ChromatogrammSettingsCoelution] = initialSettings->Coel;
      chroma[ChromatogrammSettingsViewSmooth] = initialSettings->View_smooth;
      auto blineEntities = newModel->getChromatogramm(id)->getEntities(TypeBaseline);
      if(!blineEntities.isEmpty())
      {
        auto bline = blineEntities.first();
        if(bline->hasDataAndItsValid(BaseLineData))
        {
          auto blineModelPtr = blineEntities.first()->getData(BaseLineData).value<TBaseLineDataModel>();
          if(blineModelPtr)
            blineModelPtr->setBaseLine(initialSettings->BLineSmoothLvl);
        }
      }
    }
  }
  subController->SetCurrentStepAndSwitchHisModel(Step0DataLoad,
                                                 newModel, true);
}

void ViewPageProjectMarkup::ReMarkupImportedChromatogramms(const QList<int> ids, const ChromaSettings initialSettings)
{
  auto subStepper = new StepperWidget();
  auto ind = ui->steppersStackedWidget->addWidget(subStepper);
  ui->steppersStackedWidget->setCurrentIndex(ind);
  auto currentStep = mCurrentController->getCurrentStepInfo().step;


  auto oldModel = mCurrentController->getCurrentModel();
  auto subController = new SubMarkupStepController(currentStep);
  subController->initialize();
  //TODO: make this on pointers, remove data copying:
  //  subController->setExternalPassportsModel(mCurrentController->passportsModelPtr());
  //
  setStepperToController(subStepper, subController);
  auto oldController = mCurrentController;
  setNewController(subController);

  connect(subController, &QObject::destroyed, this, [this]()
          {
            ui->steppersStackedWidget->setCurrentIndex(0);
          });

  connect(subController, &SubMarkupStepController::MarkupComplete, this, [oldController, subController, this]
    {
      oldController->merge(*subController);
      oldController->MergePassportDataWith(*subController);
      delete subController;
    }, Qt::QueuedConnection);

  subController->createNewMarkupForImport(ids, initialSettings, oldModel);
}

void ViewPageProjectMarkup::TryToSwitchToTable()
{
  auto context = Core::GenesisContextRoot::Get()->GetContextStepBasedMarkup();
  context->SetData(ContextTagNames::ReferenceId, mMainController->getCurrentModel()->getData(MarkupLastTableId).toInt());
  context->SetData(ContextTagNames::StepBasedMarkupId, MarkupId);
  GenesisWindow::Get()->ShowPage(ViewPageNames::ViewPageAnalysisDataTablePageName);
}

void ViewPageProjectMarkup::SaveMarkupStateOnBack()
{
  if(mSubController
          && mMainController->stepsInfo()[mMainController->currentStepIndex()].step == Step7Complete)
  {
    Notification::NotifyError(tr("ERROR can't save markup: remarkup is forbidden on step 7"));
    return;
  }
  if(auto controller = qobject_cast<SubMarkupStepController*>(mSubController))
  {
    QVariantMap saveData;
    QVariantMap subControllerData;
    auto stepIndex = controller->currentStepIndex();
    subControllerData["step_ind"] = stepIndex;
    subControllerData["target_step_ind"] = controller->targetStep();
    QSet<int> filesIds;
    {
      auto models = controller->stepModels();
      auto stepsInfo = controller->stepsInfo();
      auto model = models[stepsInfo[stepIndex].step];
      for(auto& markup : models)
      {
        auto idList = markup->getChromatogrammsIdList();
        filesIds.unite({idList.begin(), idList.end()});
      }
    }
    subControllerData["files_ids"] = QVariant::fromValue(filesIds);
    subControllerData["json"] = mSubController->save();
    saveData["sub_controller_data"] = subControllerData;

    mMainController->SaveMarkup(saveData);
  }
  else
  {
    mMainController->SaveMarkup(QVariant());
  }
}

void ViewPageProjectMarkup::SaveMarkupAs(bool instantLoad)
{
  auto dial = new MarkupSaveAsDialog(this);
  connect(dial, &WebDialog::Accepted, this, [dial, this, instantLoad]()
          {
            mMainController->SaveMarkupAs(dial->getTitle(), "", instantLoad);
          });
  dial->Open();
}

bool ViewPageProjectMarkup::isCommandStackClean()
{
  if(!mMainController)
    return true;
  if(!mSubController)
    return mMainController->getUndoStack()->isClean();
  return mMainController->getUndoStack()->isClean() && mSubController->getUndoStack()->isClean();
}

void ViewPageProjectMarkup::SaveReferenceOnBack(ChromatogrammModelPtr chroma)
{
  if(!chroma)
    return;

  Dialogs::Templates::SaveEdit::Settings s;
  s.titleHeader = tr("Reference saving");
  s.titlePlaceholder = tr("Reference markup name");
  s.titleTooltipsSet.empty = tr("Reference name can't be empty");
  s.titleTooltipsSet.forbidden = tr("There is other reference with that name");
  s.titleTooltipsSet.notChecked = tr("Reference name is not checked yet");
  s.titleTooltipsSet.valid = tr("Reference name is valid");
  s.header = tr("Reference saving");
  s.commentHeader = tr("Comment");
  s.commentPlaceholder = tr("Leave comment about reference");

  auto dial = new Dialogs::Templates::SaveEdit(this, s);
  connect(dial, &WebDialog::Accepted, this, [dial, chroma]()
          {
            QString title = dial->getTitle();
            API::REST::SaveReference(Core::GenesisContextRoot::Get()->GetContextMarkup()->GetData(Names::MarkupContextTags::kMarkupId).toInt(),
              title,
              dial->getComment(),
              chroma->id(),
              chroma->save(),
              [title](QNetworkReply* r, QJsonDocument doc)
              {
                auto obj = doc.object();
                if(obj["error"].toBool())
                {
                  Notification::NotifyError(obj["msg"].toString(), tr("Reference saving error"));
                }
                else
                {
                  TreeModel::ResetInstances("TreeModelDynamicProjectMarkupEtalonList");
                  Notification::NotifyError(tr("Reference \"%1\" saved").arg(title));
                }
              },
              [](QNetworkReply* r, QNetworkReply::NetworkError e)
              {
                Notification::NotifyError(tr("Reference saving network error"), e);
              });
          });
  dial->Open();
}

void ViewPageProjectMarkup::ImportChromatogramms()
{
  int referenceId = -1;
  if(auto master = mMainController->getCurrentModel()->getMaster())
    referenceId = master->getChromatogramValue(ChromatogrammReferenceId).toInt();

  auto dial = new Dialogs::WebDialogAddChromatogramms(mMainController->getCurrentModel()->getChromatogrammsIdList(), referenceId, this);
  auto overlayId = GenesisWindow::Get()->ShowOverlay(tr("Loading"));
  connect(dial, &Dialogs::WebDialogAddChromatogramms::modelsLoaded, this, [dial, overlayId]()
          {
            GenesisWindow::Get()->RemoveOverlay(overlayId);
            dial->Open();
          });
  QList<int> types;
  QStringList fileTypes = mCurrentController->getFileTypes();
  for(const auto &type: fileTypes)
  {
      if(type=="tic")
      {
          types.append(ProjectFiles::tic);
          continue;
      }
      if(type=="sim")
      {
          types.append(ProjectFiles::sim);
          continue;
      }
      if(type=="GC-MS")
      {
          types.append(ProjectFiles::GC_MS);
          continue;
      }
      if(type=="GC")
      {
          types.append(ProjectFiles::GC);
          continue;
      }
  }

  dial->loadModels(types, mCurrentController->getMZ());

  //  dial->loadModels({ProjectFiles::sim});
  connect(dial, &WebDialog::Accepted, this,
          [this, dial]()
          {
            if(dial->isReferenceMode())
            {
              mMainController->importReference(dial->getReferenceId(),
                                               dial->getReferenceFileId());
            }
            else if(dial->isChromasMode())
            {
              auto chromasIds = dial->getChromasIds();
              if (chromasIds.isEmpty())
                return;

              if(mMainController->getCurrentModel()->hasMaster())
              {
                ChromaSettings initialSettings = mMainController->getCurrentModel()->getMaster()->getChromaSettings();
                ReMarkupImportedChromatogramms(chromasIds, initialSettings);
              }
              else
              {
                ReMarkupImportedChromatogramms(chromasIds);
              }
            }
          });
}

void ViewPageProjectMarkup::onNewCommand(QUndoCommand *command)
{
  if(mCurrentController)
      mCurrentController->pushCommand(command);
}

void ViewPageProjectMarkup::onRemarkupCalled(int id, const ChromaSettings &newSettings)
{
  ChromaSettings localSettingsCopy = newSettings;
  ReMarkupChromatogramms({id}, &localSettingsCopy);
}

void ViewPageProjectMarkup::onIntervalsModeChanged(bool isIntervalsModeActive)
{
  if(isIntervalsModeActive)
  {
    ui->prevStepButton->setEnabled(false);
    ui->nextStepButton->setEnabled(false);
    SaveAsAction->setEnabled(false);
    SaveAction->setEnabled(false);
    RemarkupAction->setEnabled(false);
    ImportChromatogrammsAction->setEnabled(false);
  }
  else
  {
    ui->prevStepButton->setEnabled(true);
    ui->nextStepButton->setEnabled(true);
    auto handleAction = [this](StepGuiInteractions flag, QAction* a)
    {
      if(mCurrentController->getCurrentStepInfo().guiInteractions.testFlag(flag))
      {
        if(!a->isEnabled())
          a->setEnabled(true);
      }
      else
      {
        if(a->isEnabled())
          a->setEnabled(false);
      }
    };
    handleAction(SIGRemarkupSlaves, RemarkupAction);
    handleAction(SIGSaveMarkup, SaveAction);
    handleAction(SIGSaveMarkupAs, SaveAsAction);
    handleAction(SIGImportChromatogramms, ImportChromatogrammsAction);
  }
}

void ViewPageProjectMarkup::clearView()
{
  auto currentModule = (Names::ModulesContextTags::Module)Core::GenesisContextRoot::Get()->GetContextModules()->GetData(Names::ModulesContextTags::kModule).toInt();
  if(mMainController)
  {
    disconnect(mMainController, nullptr, nullptr, nullptr);
    mMainController->deleteLater();
    mMainController = nullptr;
  }
  if(mSubController)
  {
    disconnect(mSubController, nullptr, nullptr, nullptr);
    mSubController->deleteLater();
    mSubController = nullptr;
  }
  MarkupWidget->clear();
  ui->steppersStackedWidget->setCurrentIndex(0);
  switch(currentModule)
  {
  case ModulesContextTags::MNoModule:
  case ModulesContextTags::MReservoir:
  case ModulesContextTags::MPlots:
  case ModulesContextTags::MLast:
  default:
  {
    mMainController = new MarkupStepController(this);
    break;
  }
  case ModulesContextTags::MBiomarkers:
  {
    mMainController = new BiomarkersMarkupStepController(this);
    break;
  }
  }
  mMainController->initialize();
  mCurrentController = mMainController;
  setupControllerConnections(mCurrentController);
  connect(mCurrentController, &MarkupStepController::submarkupLoadingRequired, this, &ViewPageProjectMarkup::onSubmarkupLoadingRequired, Qt::QueuedConnection);
  setStepperToController(ui->mainStepper, mCurrentController);
  connectUndoStack(mCurrentController);
}

void ViewPageProjectMarkup::setViewHeader(QString newHeader)
{
  ui->markupCaptionLabel->setToolTip(newHeader);
  newHeader = QFontMetrics(ui->markupCaptionLabel->font()).elidedText(newHeader, Qt::ElideRight, 800);
  ui->markupCaptionLabel->setText(newHeader);
}

void ViewPageProjectMarkup::onSubmarkupLoadingRequired(QJsonObject submarkupData)
{
  auto subStepper = new StepperWidget();
  auto ind = ui->steppersStackedWidget->addWidget(subStepper);
  ui->steppersStackedWidget->setCurrentIndex(ind);
  Steps targetStepInd = (Steps)submarkupData["target_step_ind"].toInt();
  int currentSubStepInd = submarkupData["step_ind"].toInt();


  auto oldModel = mCurrentController->getCurrentModel();
  auto subController = new SubMarkupStepController(targetStepInd);
  subController->initialize();
  setStepperToController(subStepper, subController);
  auto oldController = mCurrentController;
  setNewController(subController);

  connect(subController, &QObject::destroyed, this, [this]()
          {
            ui->steppersStackedWidget->setCurrentIndex(0);
          });

  connect(subController, &SubMarkupStepController::MarkupComplete, this, [oldController, subController, this]
      {
        oldController->merge(*subController);
        oldController->MergePassportDataWith(*subController);
        delete subController;
      }, Qt::QueuedConnection);
  auto markupData = submarkupData["markup"].toObject();
  connect(mMainController, &MarkupStepController::markupLoadingFinished, this,
          [this, subController, markupData, currentSubStepInd]()
          {
            subController->setExternalPassportsModel(mMainController->passportsModelPtr());
            subController->load(markupData);
            auto passports = subController->passportsModelPtr();
            auto models = subController->stepModels();
            auto baseModel = mMainController->getCurrentModel();
            for(auto model : models)
            {
              auto idList = model->getChromatogrammsIdList();
              for(auto& id : idList)
              {
                auto curve = MarkupDataModel::getCurve(id);
                auto chroma = model->getChromatogramm(id);
                if(chroma)
                {
                  auto curveUid = chroma->addNewEntity(TypeCurve);
                  chroma->setEntityData(curveUid, CurveData, QVariant::fromValue(curve));
                }
              }
              if(model->getData(MarkupMasterIdRole).toInt() != -1)
              {
                //should we compare master ids here?
                model->setData(MarkupExternalMasterPtr, QVariant::fromValue(baseModel->getMaster()));
              }
            }
            subController->setCurrentStep(subController->stepsInfo()[currentSubStepInd].step);
      }, Qt::QueuedConnection);
}

void ViewPageProjectMarkup::onDialogRised()
{
  MarkupWidget->onActionsStateChanged(false);
}

void ViewPageProjectMarkup::onDialogDestroyed()
{
  MarkupWidget->onActionsStateChanged(true);
}

void ViewPageProjectMarkup::keyPressEvent(QKeyEvent *event)
{
  View::keyPressEvent(event);
}

void ViewPageProjectMarkup::setNewController(MarkupStepController *newController)
{
  if(!newController)
    return;
  if(mSubController)
  {
    mSubController->deleteLater();
    mSubController = nullptr;
  }

  setupControllerConnections(newController);
  mSubController = newController;
  disconnectUndoStack(mCurrentController);
  mCurrentController = mSubController;
  connectUndoStack(mCurrentController);
}

void ViewPageProjectMarkup::setupControllerConnections(MarkupStepController *newController)
{
  bool isMain = false;
  if(newController == mMainController)
    isMain = true;
  if(isMain)
    connect(newController, &MarkupStepController::ModelChanged, this, &ViewPageProjectMarkup::onMainModelChanged);
  else
    connect(newController, &MarkupStepController::ModelChanged, this, &ViewPageProjectMarkup::onSubModelChanged);

  connect(newController, &MarkupStepController::markupSavingFinished, this, [this](bool error)
          {
            if(error)
              return;
            if(auto stack = getCurrentUndoStack())
              stack->setClean();
          });
  connect(newController, &QObject::destroyed, this, [this, isMain]()
          {
            if(!isMain)
            {
              mSubController = nullptr;
              MarkupWidget->setSubMarkupModel(nullptr, StepInfo());
              if(mMainController)
              {
                mCurrentController = mMainController;
                connectUndoStack(mCurrentController);
                onMainModelChanged(mCurrentController->getCurrentModel(), mCurrentController->getCurrentStepInfo());
              }
            }
            else
            {
              mMainController = nullptr;
              mCurrentController = nullptr;
              MarkupWidget->setMainMarkupModel(nullptr, StepInfo());
            }
          });
}

void ViewPageProjectMarkup::setStepperToController(StepperWidget *stepper, MarkupStepController *controller)
{
  {
    auto connections = stepper->property("connections");
    if(connections.isValid() && !connections.isNull())
    {
      auto conList = connections.value<QList<QMetaObject::Connection>>();
      for(auto& connection : conList)
      {
        if(connection)
          disconnect(connection);
      }
      stepper->setProperty("connections", QVariant());
    }
  }
  QList<std::tuple<int, QString, GenesisMarkup::Steps, QString>> stepsList;
  for(auto& step : controller->stepsInfo())
  {
    if(step.step == Step0DataLoad)
      continue;
    stepsList << std::make_tuple(step.stepNumber, step.stepName, step.step, step.toolTip);
  }
  stepper->setStep(-1);//Step0DataLoad
  stepper->setSteps(stepsList);
  QList<QMetaObject::Connection> connectionsList;
  connectionsList.append(connect(controller, &MarkupStepController::ModelChanged, stepper, [stepper, stepsList](MarkupModelPtr, const StepInfo& modelStepInteractions)
                                 {
                                   QSignalBlocker sb(stepper);
                                   for(int i = 0; i < stepsList.size(); i++)
                                   {
                                     if(std::get<2>(stepsList[i]) == modelStepInteractions.step)
                                     {
                                       stepper->setStep(i);
                                       return;
                                     }
                                   }
                                   stepper->setStep(-1);
                                 }));
  connectionsList.append(connect(stepper, &StepperWidget::stepChanged, this, [this, stepper, stepsList, controller](Steps step)
                                 {
                                   if(controller->stepModels().constFind(step) == controller->stepModels().constEnd())
                                   {
                                     QSignalBlocker sb(stepper);
                                     for(int i = 0; i < stepsList.size(); i++)
                                     {
                                       if(std::get<2>(stepsList[i]) == controller->getCurrentStepInfo().step)
                                       {
                                         stepper->setStep(i);
                                         return;
                                       }
                                     }
                                     stepper->setStep(-1);
                                     return;
                                   }
                                   auto cmd = new SetStepCommand(controller,
                                                                 controller->getCurrentModel(),
                                                                 controller->getCurrentStepInfo().step,
                                                                 step,
                                                                 controller->stepModels().constFind(step).value());
                                   controller->pushCommand(cmd);
                                 }));
  connectionsList.append(connect(controller, &MarkupStepController::stepsChanged, stepper, [stepper, controller]()
                                 {
                                   QList<std::tuple<int, QString, GenesisMarkup::Steps, QString>> stepsList;
                                   for(auto& step : controller->stepsInfo())
                                   {
                                     if(step.step == Step0DataLoad)
                                       continue;
                                     stepsList << std::make_tuple(step.stepNumber, step.stepName, step.step, step.toolTip);
                                   }
                                   stepper->setStep(-1);//Step0DataLoad
                                   stepper->setSteps(stepsList);
                                 }));

  stepper->setProperty("connections", QVariant::fromValue(connectionsList));
}

void ViewPageProjectMarkup::onStepForward()
{
  auto throwNextStepCommand = [this](const QVariant& details = QVariant())
  {
    auto cmd = new NextStepCommand(mCurrentController,
                                   mCurrentController->getCurrentModel(),
                                   mCurrentController->getCurrentStepInfo().step,
                                   details);
    mCurrentController->pushCommand(cmd);
  };
  auto steps = mCurrentController->stepsInfo();
  auto currInd = mCurrentController->currentStepIndex();
  auto currentModel = mCurrentController->getCurrentModel();
  Steps nextStep;
  if(steps.size() >= (currInd + 1) + 1)
  {
    nextStep = steps[currInd + 1].step;
  }
  else
  {
    //trying to step forward but there no next step, controller should finish markup
    mCurrentController->StepForward();
    return;
  }

  switch(nextStep)
  {
  case Step0DataLoad:
  case Step1PeakDetection:
  case Step2MasterMarkersMarkup:
  case Step3MasterInterMarkersMarkup:
  {
    throwNextStepCommand();
    break;
  }
  case Step4SlavePeaksEditing:
  {
    auto currentModule = (Names::ModulesContextTags::Module)GenesisContextRoot::Get()->GetContextModules()->GetData(Names::ModulesContextTags::kModule).toInt();
    if(currentModule == Names::ModulesContextTags::MBiomarkers)
    {
      throwNextStepCommand();
      break;
    }
    //calculate valuable peaks list for markup model:
    if(currentModel)
      currentModel->updateValuablePeaksSortedIdList();
    else
      break;
    if(!currentModel->getMaster())
      break;

    Dialogs::Templates::SaveEdit::Settings s;
    s.titleHeader = tr("Reference saving");
    s.titlePlaceholder = tr("Reference markup name");
    s.titleTooltipsSet.empty = tr("Reference name can't be empty");
    s.titleTooltipsSet.forbidden = tr("There is other reference with that name");
    s.titleTooltipsSet.notChecked = tr("Reference name is not checked yet");
    s.titleTooltipsSet.valid = tr("Reference name is valid");
    s.header = tr("Reference saving");
    s.commentHeader = tr("Comment");
    s.commentPlaceholder = tr("Leave comment about reference");

    auto dial = new Dialogs::Templates::SaveEdit(this, s);
    connect(dial, &WebDialog::Accepted, this, [throwNextStepCommand, dial, model = currentModel]()
            {
              QString title = dial->getTitle();
              const auto currentModel = model->getMaster();
              API::REST::SaveReference(Core::GenesisContextRoot::Get()->GetContextMarkup()->GetData(Names::MarkupContextTags::kMarkupId).toInt(),
                title,
                dial->getComment(),
                currentModel->id(),
                currentModel->save(),
                [throwNextStepCommand, title, currentModel](QNetworkReply* r, QJsonDocument doc)
                {
                  auto obj = doc.object();

                  const QString referenceId("etalon_id");
                  if (obj.contains(referenceId))
                  {
                    currentModel->setChromatogramValue(ChromatogrammReferenceId, obj.value(referenceId));
                  }
                  if(obj["error"].toBool())
                  {
                    Notification::NotifyError(obj["msg"].toString(), tr("Reference saving error"));
                  }
                  else
                  {
                    Notification::NotifySuccess(tr("Reference \"%1\" saved").arg(title));
                    throwNextStepCommand();
                    TreeModel::ResetInstances("TreeModelDynamicProjectMarkupEtalonList");
                  }
                },
                [](QNetworkReply* r, QNetworkReply::NetworkError e)
                {
                  Notification::NotifyError(tr("Reference saving network error"), e);

                });
            });
    dial->Open();
    break;
  }
  case Step5MarkersTransferToSlave:
  {
    auto dial = new WebDialogMarkersTransfer(this);
    connect(dial, &WebDialogMarkersTransfer::Accepted, this, [dial, throwNextStepCommand]()
            {
              throwNextStepCommand(dial->getParameter());
            });
    dial->Open();
    break;
  }
  case Step6InterMarkersTransferToSlave:
  {
    Q_ASSERT(currentModel->hasMaster());
    if(!currentModel->hasMaster())
      return;

    auto dial = new IntermarkersTransferDialog(currentModel->getMaster().data(), this);
    connect(dial, &IntermarkersTransferDialog::Accepted, this, [dial, throwNextStepCommand]()
            {
              GenesisMarkup::IntermakrerTransferDialogDataPair pair{dial->getParameter(), dial->getIntervals()};
              throwNextStepCommand(QVariant::fromValue(pair));
            });
    dial->Open();
    break;
  }
  case Step7Complete:
  {
    API::REST::GetDataTables(Core::GenesisContextRoot::Get()->ProjectId(),
      [this, throwNextStepCommand](QNetworkReply*, QJsonDocument doc)
      {
        auto root = doc.object();
        if(root["error"].toBool())
        {

          Notification::NotifyError(tr("Cant save model on step 7, try again!: %1").arg(root["msg"].toString()));
        }
        auto data = root["data"].toArray();
        QStringList occupiedNames;

        for(int i = 0; i < data.size(); i++)
          occupiedNames << data[i].toArray()[1].toString();
        auto currentModule = (Names::ModulesContextTags::Module)GenesisContextRoot::Get()->GetContextModules()->GetData(Names::ModulesContextTags::kModule).toInt();
        if(currentModule == ModulesContextTags::MReservoir)
        {
          auto dial = new MarkupSaveTableDialog(this, occupiedNames);
          connect(dial, &WebDialog::Accepted, this, [this, throwNextStepCommand, dial]
                  {
                    if(dial->generateMatrix())
                      connect(mCurrentController, &MarkupStepController::markupTableIdReceived,
                              this, &ViewPageProjectMarkup::openRatioMatrixSettingsDialog,
                              Qt::SingleShotConnection);

                    throwNextStepCommand(QVariant::fromValue(QPair<QString, QString>(dial->name(), dial->comment())));
                  });
          connect(dial, &WebDialog::Rejected, this, [this]
                  {
                    Notification::NotifyError(tr("You should save data table if you want to finish markup"));
                  });
          dial->Open();
        }
        else if(currentModule == ModulesContextTags::MBiomarkers)
        {
          auto dial = new BiomarkersMarkupSaveTableDialog(this, occupiedNames);
          connect(dial, &WebDialog::Accepted, this, [dial, throwNextStepCommand]()
                  {
                    QVariantMap saveData;
                    saveData["title"] = dial->tableTitle();
                    saveData["comment"] = dial->tableComment();
                    if(dial->isCommonGroup())
                    {
                      saveData["into_common"] = true;
                    }
                    else
                    {
                      saveData["into_common"] = false;
                      if(dial->isExistedTableGroup())
                      {
                        saveData["existed"] = true;
                        saveData["group_id"] = dial->tableGroupId();
                        saveData["group_title"] = dial->tableGroupTitle();
                      }
                      else
                      {
                        saveData["existed"] = false;
                        saveData["group_title"] = dial->tableGroupTitle();
                      }
                    }
                    throwNextStepCommand(saveData);
                  });
          dial->Open();
        }
      },
      [](QNetworkReply*, QNetworkReply::NetworkError e)
      {
        Notification::NotifyError(tr("Cant save model on step 7, try again!:"), e);
      });
    break;
  }
  case Step6Identification:
    throwNextStepCommand();
    break;
  case StepEnumLast:
  default:
    break;
  }
}

QUndoStack* ViewPageProjectMarkup::getCurrentUndoStack()
{
  return mCurrentController->getUndoStack();
}

void ViewPageProjectMarkup::connectUndoStack(MarkupStepController *controller)
{
  if(!controller)
    return;
  auto stack = controller->getUndoStack();
  connect(stack, &QUndoStack::canRedoChanged, this, &ViewPageProjectMarkup::onCurrentUndoStackCanRedoChanged);
  connect(stack, &QUndoStack::canUndoChanged, this, &ViewPageProjectMarkup::onCurrentUndoStackCanUndoChanged);
  connect(stack, &QUndoStack::cleanChanged, this, &ViewPageProjectMarkup::onCurrentUndoStackCleanChanged);
}

void ViewPageProjectMarkup::disconnectUndoStack(MarkupStepController *controller)
{
  if(!controller)
    return;
  auto stack = controller->getUndoStack();
  disconnect(stack, &QUndoStack::canRedoChanged, this, &ViewPageProjectMarkup::onCurrentUndoStackCanRedoChanged);
  disconnect(stack, &QUndoStack::canUndoChanged, this, &ViewPageProjectMarkup::onCurrentUndoStackCanUndoChanged);
  disconnect(stack, &QUndoStack::cleanChanged, this, &ViewPageProjectMarkup::onCurrentUndoStackCleanChanged);

}

void ViewPageProjectMarkup::onCurrentUndoStackCanRedoChanged(bool canRedo)
{
  MarkupWidget->onRedoActionStateChanged(canRedo);
}

void ViewPageProjectMarkup::onCurrentUndoStackCanUndoChanged(bool canUndo)
{
  MarkupWidget->onUndoActionStateChanged(canUndo);
}

void ViewPageProjectMarkup::onCurrentUndoStackCleanChanged(bool clean)
{
  if(clean)
    emit markupStateClean();
}

//for future
// void ViewPageProjectMarkup::onCurrentUndoStackIndexChanged(int idx)
// {
// }

// void ViewPageProjectMarkup::onCurrentUndoStackUndoTextChanged(const QString &undoText)
// {
// }

// void ViewPageProjectMarkup::onCurrentUndoStackRedoTextChanged(const QString &redoText)
// {
// }

void GenesisMarkup::ViewPageProjectMarkup::SwitchToPage(const QString &pageId)
{
  if(mCurrentController)
  {
    auto stepsInfo = mCurrentController->stepsInfo();
    auto currStepInd = mCurrentController->currentStepIndex();
    auto currStep = stepsInfo[currStepInd].step;
    if(currStep == Step7Complete)
    {
      View::SwitchToPage(pageId);
      return;
    }
  }

  if(isCommandStackClean())
  {
    View::SwitchToPage(pageId);
    return;
  }

  auto dial = Dialogs::Templates::Confirm::confirm(GenesisWindow::Get(),
    tr("You have unsaved changes, and it may be lost. Do you want to save your changes, before continue?"),
    [this, pageId]()
    {
      connect(mCurrentController, &MarkupStepController::markupSavingFinished, this,
        [this, pageId](bool error)
        {
          if(!error)
            View::SwitchToPage(pageId);
          else
            View::RejectPageSwitching();
        }, Qt::SingleShotConnection);
      SaveMarkupStateOnBack();
    },
    [this, pageId]()
    {
      View::SwitchToPage(pageId);
    },
    [this]()
    {
      View::RejectPageSwitching();
    });
  dial->Open();
}

void ViewPageProjectMarkup::SwitchToModule(int module)
{
  if(mCurrentController)
  {
    auto stepsInfo = mCurrentController->stepsInfo();
    auto currStepInd = mCurrentController->currentStepIndex();
    auto currStep = stepsInfo[currStepInd].step;
    if(currStep == Step7Complete)
    {
      View::SwitchToModule(module);
      return;
    }
  }

  if(isCommandStackClean())
  {
    View::SwitchToModule(module);
    return;
  }

  auto dial = Dialogs::Templates::Confirm::confirm(GenesisWindow::Get(),
      tr("You have unsaved changes, and it may be lost. Do you want to save your changes, before continue?"),
      [this, module]()
      {
        connect(mCurrentController, &MarkupStepController::markupSavingFinished, this,
            [this, module](bool error)
            {
              if(!error)
                View::SwitchToModule(module);
              else
                View::RejectModuleSwitching();
            }, Qt::SingleShotConnection);
        SaveMarkupStateOnBack();
      },
      [this, module]()
      {
        View::SwitchToModule(module);
      },
      [this]()
      {
        View::RejectModuleSwitching();
      });
  dial->Open();
}

void GenesisMarkup::ViewPageProjectMarkup::LogOut()
{
  if(mCurrentController)
  {
    auto stepsInfo = mCurrentController->stepsInfo();
    auto currStepInd = mCurrentController->currentStepIndex();
    auto currStep = stepsInfo[currStepInd].step;
    if(currStep == Step7Complete)
      View::LogOut();
  }

  if(isCommandStackClean())
    View::LogOut();

  auto dial = Dialogs::Templates::Confirm::confirm(GenesisWindow::Get(),
    tr("You have unsaved changes, and it may be lost. Do you want to save your changes, before continue?"),
    [this]()
    {
      connect(mCurrentController, &MarkupStepController::markupSavingFinished, this,
        [this](bool error)
        {
          if(!error)
            View::LogOut();
          else
            View::RejectLoggingOut();
        }, Qt::SingleShotConnection);
      SaveMarkupStateOnBack();
    },
    [this]()
    {
      View::LogOut();
    },
    [this]()
    {
      View::RejectLoggingOut();
    });
  dial->Open();
}


void GenesisMarkup::ViewPageProjectMarkup::ApplyContextModules(const QString &dataId, const QVariant &data)
{
  bool isReset = isDataReset(dataId, data);

  using namespace Names::ModulesContextTags;

  if(!isReset && dataId != kModule)
    return;

  Module module = MNoModule;

  if(isReset)
  {
    auto context = Core::GenesisContextRoot::Get()->GetContextModules();
    module = Module(context->GetData(kModule).toInt());
  }
  else if(dataId == kModule)
  {
    if(module == data.toInt())
      return;
    module = Module(data.toInt());
  }
  //@todo: CLEAR ALL AND DROP TO MARKUPS TABLE WITHOUT NOTIFICATION?
  clearView();


  switch(module)
  {
  case Names::ModulesContextTags::MReservoir:
  case Names::ModulesContextTags::MPlots:
  case Names::ModulesContextTags::MNoModule:
  case Names::ModulesContextTags::MLast:
  default:
    if(IdentificationTabWidget)
    {
      MarkupWidget->removeTab(IdentificationTabWidget);
      IdentificationTabWidget = nullptr;
    }
    TitleModeCompoundAction->setVisible(false);
    TitleModeIndexAction->setVisible(true);
    TitleModeOrderAction->setChecked(true);
    ChromatogramDataModel::DefaultMarkerColor = QColor(0, 32, 51, 150);
    ChromatogramDataModel::DefaultMarkerPrefix = "M";
    break;
  case Names::ModulesContextTags::MBiomarkers:
  {
    if(!IdentificationTabWidget)
    {
      IdentificationTabWidget = new IdentificationWidget(nullptr);
      MarkupWidget->addTab(IdentificationTabWidget, tr("Identification"));
      IdentificationTabWidget->setChromaHeight(MarkupWidget->chromaHeight());
    }
    TitleModeCompoundAction->setVisible(true);
    TitleModeCompoundAction->setChecked(true);
    TitleModeIndexAction->setVisible(false);
    ChromatogramDataModel::DefaultMarkerColor = QColor(133, 221, 179, 255);
    ChromatogramDataModel::DefaultMarkerPrefix = "";
    break;
  }
  }
  MarkupWidget->setModule(module);
}

void ViewPageProjectMarkup::loadMarkup(int markupId)
{
  loadingIsPending = true;
  mFirstShow = true;
  connect(mMainController, &MarkupStepController::markupLoadingFinished, this,
          [this]()
          {
            loadingIsPending = false;
          });
  mMainController->LoadMarkup(markupId);
}

void ViewPageProjectMarkup::onUserChangedTitlesMode(bool checked)
{
  if(!checked)
    return;

  if(auto action = qobject_cast<QAction*>(sender()))
  {
    auto mode = action->property("mode").value<TChromatogrammTitlesDisplayMode>();
    GenesisContextRoot::Get()->GetContextMarkup()->SetData(MarkupContextTags::kPeakTitleDisplayMode, mode);
  }
}

void ViewPageProjectMarkup::updateTitleMode(ChromatogrammTitlesDisplayModes mode)
{
  bool skipUpdate = false;
  switch(mode)
  {
  case TitleModeIndex:
    TitleModeIndexAction->setChecked(true);
    break;
  case TitleModeRetentionTime:
    TitleModeRetTimeAction->setChecked(true);
    break;
  case TitleModeCompoundTitle:
    TitleModeCompoundAction->setChecked(true);
    break;
  case TitleModeOrder:
    TitleModeOrderAction->setChecked(true);
    break;
  case TitleModeDefault:
  default:
  {
    using namespace Names::ModulesContextTags;
    using namespace Names::MarkupContextTags;
    auto module = GenesisContextRoot::Get()->GetContextModules()->GetData(kModule).value<Module>();
    skipUpdate = true;
    if(module == MBiomarkers)
      mode = TitleModeCompoundTitle;
    else if(module == MReservoir)
      mode = TitleModeOrder;
    if(mode != TitleModeDefault)
      GenesisContextRoot::Get()->GetContextMarkup()->SetData(kPeakTitleDisplayMode, mode);
    break;
  }
  }

  if(!skipUpdate)
  {
    qDebug() << "switch to" << enumToStr(mode);
    if(mMainController)
      mMainController->updateTitlesDisplayMode();
    if(mSubController)
      mSubController->updateTitlesDisplayMode();
  }
}

void ViewPageProjectMarkup::openRatioMatrixSettingsDialog(int tableId)
{
  auto hrmDial = new Dialogs::WebDialogRatioMatrixSettings(this);
  connect(hrmDial, &WebDialog::Accepted, this, [tableId, hrmDial]()
          {
            QJsonObject json;
            json.insert(JsonTagNames::Title, hrmDial->GetTitle());
            json.insert(JsonTagNames::Comment, hrmDial->GetComment());
            json.insert(JsonTagNames::TableId, tableId);
            if (const auto contextRoot = Core::GenesisContextRoot::Get())
            {
              if (const auto projectContext = contextRoot->GetContextProject())
                json.insert(JsonTagNames::project_id, projectContext->GetData(Names::ContextTagNames::ProjectId).toInt());
            }
            QJsonObject params{
                               { JsonTagNames::WindowSize, hrmDial->GetWindowSize() },
                               { JsonTagNames::MinValue, hrmDial->GetMinimum() },
                               { JsonTagNames::MaxValue, hrmDial->GetMaximum() },
                               { JsonTagNames::IncludeMarkers, hrmDial->GetIsIncludeMarkers() },
                               };
            json.insert(JsonTagNames::Parameters, params);
            auto overlayId = GenesisWindow::Get()->ShowOverlay(tr("Loading data..."));
            API::REST::GetHeigthRatioMatrix(json,
                [&, overlayId, title = hrmDial->GetTitle()](QNetworkReply*, QJsonDocument doc)
                {
                  const auto object = doc.object();
                  if (object.value(JsonTagNames::Error).toBool())
                  {
                    Notification::NotifyError(object.value(JsonTagNames::Msg).toString(), tr("Error receive ratio matrix data"));
                  }
                  else
                  {
                    TreeModel::ResetInstances("TreeModelDynamicNumericTables");
                    Notification::NotifySuccess(tr("Matrix %1 generated successfuly").arg(title), tr("Ratio matrix generated succes"));
                  }
                  GenesisWindow::Get()->RemoveOverlay(overlayId);
                },
                [overlayId](QNetworkReply*, QNetworkReply::NetworkError err)
                {
                  Notification::NotifyError(tr("Error receive ratio matrix data."), err);
                  GenesisWindow::Get()->RemoveOverlay(overlayId);
                });
          });
  hrmDial->Open();
}
