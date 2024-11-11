#include "pca_count_plot_widget.h"

#include "api/api_rest.h"
#include "genesis_style/style.h"
#include "logic/context_root.h"
#include "logic/known_context_tag_names.h"
#include "logic/known_json_tag_names.h"
#include "logic/service/ijson_serializer.h"
#include "logic/service/service_locator.h"
#include "logic/models/hotteling_response_model.h"
#include "logic/models/hotteling_request_model.h"
#include "ui/dialogs/hotelling_criterion_parameters_dialog.h"
#include "ui/dialogs/web_dialog_pick_axis_components.h"
#include "ui/dialogs/templates/info.h"
#include "ui/dialogs/templates/confirm.h"
#include "ui/itemviews/datatable_item_delegate.h"
#include "ui/item_models/hotteling_data_model.h"
#include "ui/plots/counts_plot_manual_hotteling.h"
#include <ui/plots/analysis_plot_refactor.h>
#include <ui/plots/gp_items/gpshape_with_label_item.h>
#include <logic/notification.h>
#include <ui/dialogs/web_dialog_create_group_manual_hotteling.h>
#include <ui/widgets/pca_plot_legend_widget.h>
#include "ui/widgets/legend/legend_editor.h"
#include <ui/widgets/legend/legend_group_editor.h>
#include <ui/widgets/legend/legend_group_creator.h>
#include <extern/common_gui/standard_widgets/tumbler.h>
#include "ui/contexts/plot_setup_view_context.h"

#include <QRegExp>
// #include <iostream>

using namespace Model;
using namespace Models;
using namespace Structures;
using namespace Service;
using namespace Core;
using namespace Names;

using namespace AnalysisEntity;

#define LOG_PREFIX(LOG_MESSAGE) QDateTime::currentDateTime().toString() + " : "+ __FILE__ +" [ " + QString::number(__LINE__) + " ] - "+ LOG_MESSAGE

namespace
{
  const std::string_view SampleId = "sample_id";
  QString PreviousCaptionText;
  float WidgetPart = .75;
  QFile Logging("hotteling-log.txt");

  void SaveToLog(const QStringList& log)
  {
    if (Logging.open(QIODevice::Append))
    {
      QScopeGuard guard([]() { Logging.close(); });
      QTextStream stream(&Logging);
      for (const auto& msg : log)
        stream << msg << "\n";
    }
  }
  const QColor kGrey(0, 32, 51, 154);
}

namespace Widgets
{
  PcaCountPlotWidget::PcaCountPlotWidget(QWidget* parent)
    : PcaPlotWidget(parent)
  {
    setupUi();
    connectSignals();
  }

  void PcaCountPlotWidget::setModel(QPointer<AnalysisEntityModel> model)
  {
    QScopeGuard modelReset([&]() { onEntityModelReset(); });
    if(model && model == mEntityModel)
    {
      return;
    }
    if(mEntityModel)
    {
      disconnect(mEntityModel, nullptr, this, nullptr);
    }
    PcaPlotWidget::setModel(model);
    model->setCurrentXYComponent(1,2);
    mPlot->setModel(model);
    mPlotLegend->setModel(model);
    if(mEntityModel)
    {
      connect(mEntityModel, &AnalysisEntityModel::modelAboutToReset, this, &PcaCountPlotWidget::onEntityModelAboutToReset);
      connect(mEntityModel, &AnalysisEntityModel::modelReset, this, &PcaCountPlotWidget::onEntityModelReset);
      connect(mEntityModel, &AnalysisEntityModel::modelDataChanged, this, &PcaCountPlotWidget::onEntityModelDataChanged);
    }
    mPlotToolBar->SwitchScale->setCheckState(Qt::Checked);
  }

  void PcaCountPlotWidget::setupUi()
  {
    auto parentLayout = new QVBoxLayout(this);
    setLayout(parentLayout);

    mPlotToolBar = QSharedPointer<PlotToolbarWidget>::create(this);
    mPlotToolBar->Caption->setVisible(false);
    mPlotToolBar->CancelExcludingChoise->setVisible(false);
    mRecalcHottelingManual = new QDialogButtonBox(QDialogButtonBox::StandardButton::Ok | QDialogButtonBox::StandardButton::Cancel);
    mRecalcHottelingManual->button(QDialogButtonBox::StandardButton::Ok)->setText(tr("Calc Hotelling criterion"));
    mRecalcHottelingManual->setVisible(false);

    mPlotToolBar->TabBarLayout->addWidget(mRecalcHottelingManual);
    mToolbarMenu = new PlotSetupViewContext(mPlotToolBar.data());
    mPlotToolBar->SetMenu(mToolbarMenu);
    parentLayout->addWidget(mPlotToolBar.get());

    mScrollArea = new QScrollArea(this);
    mScrollArea->setWidgetResizable(true);
    mScrollArea->setStyleSheet("QWidget { background-color:white; }");
    mScrollArea->setFrameShape(QFrame::NoFrame);
    mScrollArea->setAutoFillBackground(false);
    mScrollArea->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);
    mScrollArea->setContentsMargins(0, 0, 0, 0);
    parentLayout->addWidget(mScrollArea);

    auto scrollWidget = new QWidget(mScrollArea);
    mMainLayout = new QVBoxLayout(scrollWidget);
    mMainLayout->setContentsMargins(0, 0, 0, 0);
    scrollWidget->setLayout(mMainLayout);
    scrollWidget->setContentsMargins(0, 0, 0, 0);
    mScrollArea->setWidget(scrollWidget);

    mPlot = new AnalysisPlotRefactor(scrollWidget, {TypeSample});
    mPlot->setAttribute(Qt::WA_NoMousePropagation, false);
    mPlot->setZoomModifiers(Qt::ControlModifier);
    // mPlot->installEventFilter(this);
    mPlot->xAxis->setTickLength(0, 4);
    mPlot->xAxis->setRange(0, 11);
    mPlot->xAxis->grid()->setVisible(true);

    mPlot->xAxis->setLabelColor(QColor(0, 32, 51, 154));
    mPlot->yAxis->setLabelColor(QColor(0, 32, 51, 154));

    mPlot->yAxis->setRange(0, 11);
    mPlot->yAxis->setPadding(5);
    mPlot->yAxis->grid()->setSubGridVisible(false);
    mPlot->xAxis->grid()->setSubGridVisible(false);
    mPlot->xAxis->setBasePen(kGrey);
    mPlot->yAxis->setBasePen(kGrey);
    mPlot->xAxis->setTickPen(kGrey);
    mPlot->yAxis->setTickPen(kGrey);
    mPlot->xAxis->setSubTickPen(kGrey);
    mPlot->yAxis->setSubTickPen(kGrey);
    mPlot->xAxis->setTickLabelColor(kGrey);
    mPlot->yAxis->setTickLabelColor(kGrey);
    mPlot->xAxis->setLabelColor(kGrey);
    mPlot->yAxis->setLabelColor(kGrey);
    mMainLayout->addWidget(mPlot);
    mMainLayout->setStretchFactor(mPlot, 7);
    mPlotLegend = new PcaPlotLegendWidget(this);
    connect(mPlotLegend, &PlotLegendWidgetRefactor::itemEditingRequested, this, &PcaCountPlotWidget::onGroupEditRequested);
    mMainLayout->addWidget(mPlotLegend);
    mMainLayout->setStretchFactor(mPlotLegend, 2);
    auto hLine = new QFrame();
    hLine->setFrameShape(QFrame::HLine);
    hLine->setObjectName("hline");
    hLine->setStyleSheet("border-top: 1px solid #f2f4f5");
    mMainLayout->addSpacerItem(new QSpacerItem(0, 40 - mMainLayout->spacing(), QSizePolicy::Fixed, QSizePolicy::Fixed));
    mMainLayout->addWidget(hLine);
    mMainLayout->addSpacerItem(new QSpacerItem(0, 40 - mMainLayout->spacing(), QSizePolicy::Fixed, QSizePolicy::Fixed));
    auto hottPbLayout = new QHBoxLayout;
    hottPbLayout->setContentsMargins(0, 0, 0, 0);
    mMainLayout->addLayout(hottPbLayout);
    auto hottelingLabel = new QLabel(tr("Hotelling criterion"));
    hottelingLabel->setStyleSheet(Style::Genesis::GetH2());
    hottPbLayout->addWidget(hottelingLabel);

    mHottelingCalcButton = new QPushButton(tr("Calculate hotelling criterion"));
    mHottelingCalcButton->setProperty("secondary", true);
    mHottelingCalcButton->setStyleSheet(Style::GetStyleSheet(":/resource/styles/genesis_ui_qpushbutton.qss"));

    connect(mHottelingCalcButton, &QPushButton::clicked, this, &PcaCountPlotWidget::calcHottelingCriterion);

    hottPbLayout->addWidget(mHottelingCalcButton);
    hottPbLayout->addStretch();

    auto hottelingTable = new QTableView(scrollWidget);
    hottelingTable->setStyleSheet(Style::Genesis::GetTableViewStyle());
    hottelingTable->setProperty("dataDrivenColors", true);
    hottelingTable->setItemDelegate(new DataTableItemDelegate(hottelingTable));

    hottelingTable->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);
    hottelingTable->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    hottelingTable->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    hottelingTable->setMinimumHeight(500);

    mMainLayout->addWidget(hottelingTable);
  }

  void PcaCountPlotWidget::connectSignals()
  {
    if (mRecalcHottelingManual)
    {
      connect(mRecalcHottelingManual, &QDialogButtonBox::accepted, this, &PcaCountPlotWidget::recalcManualHotteling);
      connect(mRecalcHottelingManual, &QDialogButtonBox::rejected, this, &PcaCountPlotWidget::restoreUiFromManualHotteling);
    }

    if (mPlotToolBar)
    {
      connect(mPlotToolBar->PCButton, &QPushButton::clicked, this,
              [&]()
              {
                if(!mEntityModel)
                  return;
                mEntityModel->setCurrentXYComponent(1, 2);
                mPlot->resize();
              });

      connect(mPlotToolBar->AlterPCButton, &QPushButton::clicked, this,
              [&]()
              {
                if(!mEntityModel)
                  return;
                mEntityModel->setCurrentXYComponent(3, 4);
                mPlot->resize();
              });
      connect(mToolbarMenu->mSetParamGroup, &QPushButton::clicked, this, &PcaCountPlotWidget::setParamGroup);
      connect(mToolbarMenu->mSetParamGroup, &QPushButton::clicked, mToolbarMenu, &QMenu::hide);
      connect(mPlotToolBar->PickOtherAxisButton, &QPushButton::clicked, this, &PcaCountPlotWidget::showPickAxis);
      connect(mPlotToolBar->PickOtherAxisButton, &QPushButton::clicked, mToolbarMenu, &QMenu::hide);
      connect(mPlotToolBar->CancelExcludingChoise, &QPushButton::clicked, this, [&]()
        {
          mToolbarMenu->hide();
          if (!mEntityModel)
            return;
          mEntityModel->cancelExclude(TypeSample);
          mPlotToolBar->CancelExcludingChoise->setVisible(false);
        });

      connect(mToolbarMenu->mShowNamesCheckBox, &QCheckBox::toggled, this, [&](bool checked)
        {
          if (mPlot)
            mPlot->showNames(checked);
        });

      connect(mToolbarMenu->mShowPassportInfoCheckBox, &QCheckBox::toggled, this, [&](bool checked)
        {
          if (mPlot)
            mPlot->showPassport(checked);
        });

      connect(mToolbarMenu->mRemoveAllUserGroups, &QPushButton::clicked, this, [&]()
        {
          mToolbarMenu->hide();
          auto settings = Dialogs::Templates::Confirm::Settings();
          settings.dialogHeader = tr("Warning");
          settings.buttonsProperties =
          {
            { QDialogButtonBox::Ok, {{"red", true}}}
          };
          settings.buttonsNames = { { QDialogButtonBox::Ok , tr("Continue")} };
          auto label = new QLabel(tr("If you delete all custom groups, the samples will go into groups that match the initial grouping settings."));
          auto confirm = new Dialogs::Templates::Confirm(this, settings, QDialogButtonBox::Ok | QDialogButtonBox::Cancel, label);
          connect(confirm, &Dialogs::Templates::Confirm::Accepted, this, &PcaCountPlotWidget::regroupItems);
          confirm->Open();
        });
    }
    connect(mPlot, &AnalysisPlotRefactor::contextMenuRequested, this, &PcaCountPlotWidget::onPlotContextMenuRequested);
    connect(mPlot->yAxis, qOverload<const GPRange&>(&GPAxis::rangeChanged), this, &PcaCountPlotWidget::rescaleXAxisToYAxis);
    connect(mPlot->xAxis, qOverload<const GPRange&>(&GPAxis::rangeChanged), this, &PcaCountPlotWidget::rescaleYAxisToXAxis);
    connect(mPlotToolBar->SwitchScale, &QCheckBox::stateChanged, this, [&]()
      {
        mPlot->xAxis->setScaleRatio(mPlot->yAxis);
        mPlot->xAxis->rescale();
        mPlot->replot(GraphicsPlot::RefreshPriority::rpQueuedReplot);
      });
  }

  void PcaCountPlotWidget::regroupItems()
  {
    auto groups = mEntityModel->getEntitiesUidsSet([](AnalysisEntityModel::ConstDataPtr ent)->bool
                                            {
                                              return ent->getType() == TypeGroup && ent->getData(RoleGroupUserCreated).toBool();
                                            });
    removeGroupsAndRegroup(groups);
  }

  void PcaCountPlotWidget::removeGroupAndRegroup(const TEntityUid& groupUid)
  {
    auto ent = mEntityModel->getEntity(groupUid);
    auto groups = mEntityModel->getEntities(ent->getType());
    auto affectedEntities = mEntityModel->getEntities([groupUid](AbstractEntityDataModel::ConstDataPtr ent)->bool
                                                {
                                                  auto gset = ent->getData(RoleGroupsUidSet).value<TGroupsUidSet>();
                                                  return gset.contains(groupUid);
                                                });
    if(ent->getType() == TypeGroup)
    {
      for(auto& ent : affectedEntities)
      {
        mEntityModel->resetEntityGroup(ent->getUid(), groupUid);
        for(auto& g : groups)
        {
          if(!g->hasDataAndItsValid(RolePassportFilter))
            continue;
          auto filter = g->getData(RolePassportFilter).value<TPassportFilter>();
          auto passport = ent->getData(RolePassport).value<TPassport>();
          if(filter.match(passport))
          {
            mEntityModel->setEntityGroup(ent->getUid(), g->getUid());
            break;
          }
        }
      }
    }
    else if(ent->getType() == TypeHottelingGroup)
    {
      auto defGrps = mEntityModel->getEntitiesUidsSet([](AbstractEntityDataModel::ConstDataPtr ent)
                                                {
                                                  return ent->getType() == TypeHottelingGroup && !ent->getData(RoleGroupUserCreated).toBool();
                                                });
      for(auto& ent : affectedEntities)
      {
        mEntityModel->resetEntityGroup(ent->getUid(), groupUid);
        if(!defGrps.isEmpty())
          mEntityModel->setEntityGroup(ent->getUid(), *defGrps.begin());
      }
    }
    mEntityModel->removeEntity(groupUid);
  }

  void PcaCountPlotWidget::removeGroupsAndRegroup(const QSet<TEntityUid> &groupsUids)
  {
    QMap<EntityType, QList<AnalysisEntityModel::ConstDataPtr>> groups;
    for(auto& guid : groupsUids)
    {
      auto ent = mEntityModel->getEntity(guid);
      if(!groups.contains(ent->getType()))
        groups[ent->getType()] = mEntityModel->getEntities(ent->getType());
      auto groupsList = groups[ent->getType()];
      auto affectedEntities = mEntityModel->getEntities([guid](AbstractEntityDataModel::ConstDataPtr ent)->bool
                                                        {
                                                          auto gset = ent->getData(RoleGroupsUidSet).value<TGroupsUidSet>();
                                                          return gset.contains(guid);
                                                        });
      if(ent->getType() == TypeGroup)
      {
        for(auto& ent : affectedEntities)
        {
          mEntityModel->resetEntityGroup(ent->getUid(), guid);
          for(auto& g : groupsList)
          {
            if(!g->hasDataAndItsValid(RolePassportFilter))
              continue;
            auto filter = g->getData(RolePassportFilter).value<TPassportFilter>();
            auto passport = ent->getData(RolePassport).value<TPassport>();
            if(filter.match(passport))
            {
              mEntityModel->setEntityGroup(ent->getUid(), g->getUid());
              break;
            }
          }
        }
      }
      else if(ent->getType() == TypeHottelingGroup)
      {
        auto defGrps = mEntityModel->getEntitiesUidsSet([](AbstractEntityDataModel::ConstDataPtr ent)
                                                        {
                                                          return ent->getType() == TypeHottelingGroup && !ent->getData(RoleGroupUserCreated).toBool();
                                                        });
        for(auto& ent : affectedEntities)
        {
          mEntityModel->resetEntityGroup(ent->getUid(), guid);
          if(!defGrps.isEmpty())
            mEntityModel->setEntityGroup(ent->getUid(), *defGrps.begin());
        }
      }
    }
    mEntityModel->removeEntitiesIf([groupsUids](AnalysisEntityModel::ConstDataPtr ent)->bool
                                   {
                                     return groupsUids.contains(ent->getUid());
                                   });
  }

  void PcaCountPlotWidget::showPickAxis()
  {
    if(!mEntityModel)
      return;

    QList<double> compList;
    auto varianceMap = mEntityModel->modelData(ModelRoleComponentsExplVarianceMap)
                           .value<TComponentsExplVarianceMap>();
    compList.resize(varianceMap.size(), -1);
    for(auto iter = varianceMap.constKeyValueBegin(); iter != varianceMap.constKeyValueEnd(); iter++)
      compList[iter->first - 1] = iter->second;

    auto dial = new WebDialogPickAxisComponents(this, compList);
    dial->setCurrentXComponent(mEntityModel->modelData(ModelRoleCurrentXComponentNum).toInt() - 1);
    dial->setCurrentYComponent(mEntityModel->modelData(ModelRoleCurrentYComponentNum).toInt() - 1);
    connect(dial, &WebDialog::Accepted, this, [&, dial]()
      {
        const auto& [xcomp, ycomp]= dial->GetComponents();
        mEntityModel->setCurrentXYComponent(xcomp + 1, ycomp + 1);
      });

    dial->Open();
  }

  void PcaCountPlotWidget::setParamGroup()
  {
    std::optional<TPassportFilter> colorFilter;
    if(mEntityModel->modelData().contains(ModelRoleColorFilter))
      colorFilter = mEntityModel->modelData(ModelRoleColorFilter).value<TPassportFilter>();
    std::optional<TPassportFilter> shapeFilter;
    if(mEntityModel->modelData().contains(ModelRoleShapeFilter))
      shapeFilter = mEntityModel->modelData(ModelRoleShapeFilter).value<TPassportFilter>();

    auto dial = new Dialogs::LegendEditor(mEntityModel, this, colorFilter, shapeFilter);
    connect(dial, &Dialogs::LegendEditor::Accepted, this, [this, dial]()
            {
              auto colorFilter = dial->colorFilter();
              auto shapeFilter = dial->shapeFilter();
              auto groupInfo = dial->getGroups();
              {
                QSignalBlocker block(mEntityModel);
                mEntityModel->regroup(colorFilter.value_or(TPassportFilter()),
                  shapeFilter.value_or(TPassportFilter()), groupInfo);
              }
              mEntityModel->setData(mEntityModel->getData());
            });
    dial->Open();
  }

  void PcaCountPlotWidget::calcHottelingCriterion()
  {
    QStringList logs = { LOG_PREFIX(" Calc hotteling criterion. ") };
    QScopeGuard guard([&]() { SaveToLog(logs); });

    logs << LOG_PREFIX(" init key list. ");
    auto dialog = new HotellingCriterionParametersDialog(mEntityModel, 0, this);
    connect(dialog, &Dialogs::Templates::Dialog::Accepted, this, [this, dialog]()
      {
        QStringList logs = { LOG_PREFIX(" Accept hotteling criterion dialog. " )};
        QScopeGuard guard([&]() { SaveToLog(logs); });
        if(dialog->step() == 0)
        {//manual hotteling
          logs << LOG_PREFIX(" selected manual hotteling. ");
          setupUiForManualHotteling();
          return;
        }
        //nonManual hotteling
        logs << LOG_PREFIX(" selected grouping by criteria. ");
        auto& groups = dialog->groups();
        if(groups.isEmpty())
        {
          logs << LOG_PREFIX(" ERROR hotteling groups is empty");
          Notification::NotifyError(tr("Hotteling groups error"));
          return;
        }
        auto request = createHottelingRequestModel(dialog->groups(), dialog->getCountOfPC());
        auto serializer = ServiceLocator::Instance().Resolve<HottelingRequestModel, IJsonSerializer>();
        auto json = serializer->ToJson(request);
        json.insert(JsonTagNames::project_id, GenesisContextRoot::Get()->ProjectId());
        requestHotteling(json);
      }, Qt::DirectConnection);
    logs << LOG_PREFIX(" open dialog. ");
    dialog->Open();
  }

  void PcaCountPlotWidget::exportHotteling()
  {
    if (!mHottelingRequestBody.isEmpty())
    {
      auto request = mHottelingRequestBody.value(JsonTagNames::ExportRequestBody).toObject();
      request.insert(JsonTagNames::project_id, GenesisContextRoot::Get()->ProjectId());
      //@TODO: check json tags correctness:
      API::REST::ExportTables(request,
        [](QNetworkReply* reply, QByteArray data)
        {
          auto fileName = QString::fromUtf8(reply->rawHeader("file-title"));
          QRegExp rx("(\\\\u[0-9a-fA-F]{4})");
          int pos = 0;
          while ((pos = rx.indexIn(fileName, pos)) != -1) {
            fileName.replace(pos++, 6, QChar(rx.cap(1).right(4).toUShort(0, 16)));
          }
          fileName.replace("\"", "");

          auto fileExtension = QString::fromUtf8(reply->rawHeader("file-extension"));
          fileExtension.replace("\"", "");
          fileName = QFileDialog::getSaveFileName(nullptr, tr("Choose export path"), fileName, QString("*.%1;;All files (*.*)").arg(fileExtension));

          if (!fileName.isEmpty())
          {
            QFile file(fileName);
            if (file.open(QFile::WriteOnly))
            {
              file.write(data);
              file.close();
            }
            else
            {
              Notification::NotifyError(tr("Can't open file: %1").arg(fileName));
            }
          }
        },
        [](QNetworkReply*, QNetworkReply::NetworkError error)
        {
          Notification::NotifyError(tr("File export error"), error);
        });
    }
  }

  void PcaCountPlotWidget::setupUiForManualHotteling()
  {
    QStringList logs = { LOG_PREFIX("Setup ui for manual hotteling.")};
    QScopeGuard guard([&]() { SaveToLog(logs); });
    try
    {
      // Plot->SetGroupContext(true);
      logs << LOG_PREFIX("set group context.");
      mHottelingCalcButton->setVisible(false);
      mRecalcHottelingManual->setVisible(true);
      logs << LOG_PREFIX("set visible recalc hotteling button.");
      mPlotToolBar->AlterPCButton->setVisible(false);
      mPlotToolBar->DisplayMenuButton->setVisible(false);
      mPlotToolBar->PCButton->setVisible(false);
      mPlotToolBar->PickOtherAxisButton->setVisible(false);
      mPlotToolBar->CancelExcludingChoise->setVisible(false);
      logs << LOG_PREFIX(" hide plot toolbar buttons. ");
      mPlotToolBar->SwitchScale->setVisible(false);
      logs << LOG_PREFIX(" hide open plot button. ");
      PreviousCaptionText = mPlotToolBar->Caption->text();
      mPlotToolBar->Caption->setText(tr(""));
      emit pageTitleCaptionOverride(tr("Setup group for Hotteling criterion"));
      logs << LOG_PREFIX(" emit page title override. ");
      mInHottelingManualMode = true;
      mNonParticipantsGroupUid = mEntityModel->addNewEntity(TypeHottelingGroup);
      mEntityModel->setEntityData(mNonParticipantsGroupUid, RoleColor, QVariant::fromValue(GPShapeItem::ToColor(GPShapeItem::PaleGray)));
      mEntityModel->setEntityData(mNonParticipantsGroupUid, RoleShape, QVariant::fromValue(GPShape::GetPath(GPShape::Circle)));
      mEntityModel->setEntityData(mNonParticipantsGroupUid, RoleShapePixelSize, 8);
      mEntityModel->setEntityData(mNonParticipantsGroupUid, RoleTitle, tr("Non-participants"));
      auto samples = mEntityModel->getUIdListOfEntities(TypeSample);
      mEntityModel->addEntitiesToGroup({samples.begin(), samples.end()}, mNonParticipantsGroupUid);
      mPlotLegend->setHottelingMode(mInHottelingManualMode);
      grayAll(true);
    }
    catch (std::exception ex)
    {
      logs << LOG_PREFIX(ex.what());
      Notification::NotifyError(QString("Hotteling fail!"));
    }
  }

  void PcaCountPlotWidget::restoreUiFromManualHotteling()
  {
    QStringList logs = { LOG_PREFIX(" Restore ui for manual hotteling. ") };
    QScopeGuard guard([&]() { SaveToLog(logs); });
    mRecalcHottelingManual->setVisible(false);
    mHottelingCalcButton->setVisible(true);
    logs << LOG_PREFIX(" set visible recalc hotteling button. ");
    mPlotToolBar->AlterPCButton->setVisible(true);
    mPlotToolBar->DisplayMenuButton->setVisible(true);
    mPlotToolBar->PCButton->setVisible(true);
    mPlotToolBar->PickOtherAxisButton->setVisible(true);
    mPlotToolBar->CancelExcludingChoise->setVisible(false);
    logs << LOG_PREFIX(" show plot toolbar buttons. ");
    mPlotToolBar->SwitchScale->setVisible(true);
    logs << LOG_PREFIX(" show open plot button. ");
    mPlotToolBar->Caption->setText(PreviousCaptionText);
    // Plot->SetGroupContext(false);
    // Plot->clearGroups();
    logs << LOG_PREFIX(" clear groups. ");
    emit restorePageTitle();
    logs << LOG_PREFIX(" emit restor page title. ");
    mInHottelingManualMode = false;
    mPlotLegend->setHottelingMode(mInHottelingManualMode);
    mEntityModel->removeEntities({{AbstractEntityDataModel::RoleEntityType, TypeHottelingGroup}});
    mNonParticipantsGroupUid = QUuid();
    grayAll(false);
  }

  void PcaCountPlotWidget::requestHotteling(const QJsonObject& json)
  {
    QStringList logs = { LOG_PREFIX(" Request hotteling. ") };
    QScopeGuard guard([&]() { SaveToLog(logs); });
    const auto projectId = GenesisContextRoot::Get()->ProjectId();
    API::REST::calcHotteling(projectId, json,
      [&](QNetworkReply*, QJsonDocument data)
      {
        QStringList logs = { LOG_PREFIX(" Response hotteling. ") };
        QScopeGuard guard([&]() { SaveToLog(logs); });

        auto responseJson = data.object();
        if (responseJson.value(JsonTagNames::Error).toBool())
        {
          logs << LOG_PREFIX(" Error in response. ");
          Notification::NotifyError(responseJson.value(JsonTagNames::Msg).toString(), tr("Hotteling error."));
        }
        else
        {
          // hotteling
          logs << LOG_PREFIX(" succesfull response. ");
          auto object = responseJson.value(JsonTagNames::Result).toObject();
          logs << LOG_PREFIX(" call update hotteling. ");
          updateHotteling(object);
          mHottelingRequestBody = object;
        }
      },
      [this](QNetworkReply*, QNetworkReply::NetworkError err)
      {
        mHottelingRequestBody = QJsonObject();
        updateHotteling(mHottelingRequestBody);
        QStringList logs = { LOG_PREFIX(" Error in hotteling response. ") };
        QScopeGuard guard([&]() { SaveToLog(logs); });
        logs << LOG_PREFIX(QString("Hotteling error : %1").arg(err));
        Notification::NotifyError(QString("Hotteling error : %1").arg(err));
      });
  }

  void PcaCountPlotWidget::recalcManualHotteling()
  {
    QStringList logs = { LOG_PREFIX(" Recalc manual hotteling. ") };
    QScopeGuard guard([&]() { SaveToLog(logs); });
    auto dialog = new HotellingCriterionParametersDialog(mEntityModel, 1, this);
    connect(dialog, &Dialogs::Templates::Dialog::Accepted, this, [this, dialog]()
            {
              QStringList logs = { LOG_PREFIX(" Accept hotteling criterion dialog. " )};
              QScopeGuard guard([&]() { SaveToLog(logs); });
              auto request = createHottelingRequestModel(dialog->groups(), dialog->getCountOfPC());
              auto serializer = ServiceLocator::Instance().Resolve<HottelingRequestModel, IJsonSerializer>();
              auto json = serializer->ToJson(request);
              json.insert(JsonTagNames::project_id, GenesisContextRoot::Get()->ProjectId());
              logs << QDateTime::currentDateTime().toString() + " Serialize manual hotteling complete. " + __FILE__ + QString::number(__LINE__);
              requestHotteling(json);
              restoreUiFromManualHotteling();
            });
    dialog->Open();
  }

  void PcaCountPlotWidget::clearHotteling()
  {
    QStringList logs = { LOG_PREFIX(" Clear hotteling. ") };
    QScopeGuard guard([&]() { SaveToLog(logs); });
    if (const auto table = findChild<QTableView*>())
    {
      if (auto model = table->model())
      {
        table->setModel(nullptr);
        delete model;
      }
      mHottelingRequestBody = QJsonObject();
      updateHotteling(mHottelingRequestBody);
      logs << LOG_PREFIX(" Clear groups. ");
    }
  }

  bool PcaCountPlotWidget::hottelingManualGroupingIsActive()
  {
    return mInHottelingManualMode;
  }

  bool PcaCountPlotWidget::hotellingLoaded() const
  {
    return mHotellingLoaded;
  }

  QJsonObject PcaCountPlotWidget::hottelingExportRequestBody() const
  {
    return mHottelingRequestBody;
  }

  bool PcaCountPlotWidget::eventFilter(QObject* watched, QEvent* event)
  {
    if (const auto wheelEvent = static_cast<QWheelEvent*>(event))
    {
      if (auto plot = qobject_cast<GraphicsPlot*>(watched))
      {
        if (event->type() == QEvent::Wheel)
        {
          if (!wheelEvent->modifiers().testFlag(Qt::ControlModifier))
          {
            wheelEvent->ignore();
            auto ne = new QWheelEvent(wheelEvent->position(),
              wheelEvent->globalPosition(), wheelEvent->pixelDelta(),
              wheelEvent->angleDelta(), wheelEvent->buttons(),
              wheelEvent->modifiers(), wheelEvent->phase(),
              wheelEvent->inverted(), wheelEvent->source(),
              wheelEvent->pointingDevice());
            QCoreApplication::sendEvent(mScrollArea->verticalScrollBar(), ne);
            return true;
          }
        }
      }
    }

    return false;
  }

  void PcaCountPlotWidget::resizeEvent(QResizeEvent* event)
  {
    PcaPlotWidget::resizeEvent(event);
    mPlot->setMinimumHeight(height() * WidgetPart);
    mPlot->setMaximumHeight(height() * WidgetPart);
  }

  void PcaCountPlotWidget::onPlotContextMenuRequested(QContextMenuEvent *event)
  {
    if(mContextMenu)
      mContextMenu->clear();
    else
      mContextMenu = new QMenu(this);
void PcaCountPlotWidget::setupContextMenu() {
    // Создаем контекстное меню, если его еще нет
    if (!mContextMenu) {
        mContextMenu = new QMenu(this);
    }

    // Создаем действия для копирования, вставки и вырезания
    QAction* copyAction = new QAction(tr("Copy"), this);
    QAction* pasteAction = new QAction(tr("Paste"), this);
    QAction* cutAction = new QAction(tr("Cut"), this);

    // Добавляем действия в контекстное меню
    mContextMenu->addAction(copyAction);
    mContextMenu->addAction(pasteAction);
    mContextMenu->addAction(cutAction);

    // Подключаем действия к функциям
    connect(copyAction, &QAction::triggered, this, &PcaCountPlotWidget::copySelectedItem);
    connect(pasteAction, &QAction::triggered, this, &PcaCountPlotWidget::pasteItem);
    connect(cutAction, &QAction::triggered, this, &PcaCountPlotWidget::cutSelectedItem);
}

// Функция для обработки запроса контекстного меню
void PcaCountPlotWidget::onPlotContextMenuRequested(const QPoint &pos) {
    setupContextMenu();
    mContextMenu->exec(mapToGlobal(pos));  // Показать контекстное меню в точке клика
}

    if(mInHottelingManualMode)
      createHottelingContextMenu(event);
    else
      createContextMenu(event);

    mContextMenu->popup(event->globalPos());
    event->accept();
  }

  void PcaCountPlotWidget::onGroupEditRequested(const QUuid &uid)
  {
    if(mEntityModel->getEntity(uid))
    {
      auto dial = new Dialogs::LegendGroupEditor(uid, mEntityModel, this);
      connect(dial, &Dialogs::LegendGroupEditor::removeGroupAndRegroup, this, &PcaCountPlotWidget::removeGroupAndRegroup);
      dial->Open();
    }
    else
    {

    }
  }

// Копирование выбранного объекта
void ClassName::copySelectedItem() {
    // Сохраняем ссылку на текущий объект и его стиль
    copiedItem = currentItem;
    copiedItemStyle = currentItem->style();
}

// Вставка скопированного объекта
void ClassName::pasteItem() {
    if (copiedItem) {
        // Создаем новый объект на основе скопированного и применяем стиль
        auto newItem = new ItemType(*copiedItem);
        newItem->setStyle(copiedItemStyle);
        addItemToScene(newItem); // Добавляем объект на сцену
    }
}

// Вырезание объекта
void ClassName::cutSelectedItem() {
    copySelectedItem();  // Сначала копируем объект
    deleteSelectedItem(); // Удаляем текущий объект
}
  
IdentityModelPtr PcaCountPlotWidget::createHottelingRequestModel(const QString& criterion, uint gk)
  {
    QStringList logs = { LOG_PREFIX(" Create manual hotteling request model. ") };
    QScopeGuard guard([&]() { SaveToLog(logs); });
    auto model = std::make_shared<HottelingRequestModel>();
    model->Set(HottelingRequestModel::Count, static_cast<size_t>(gk));
    HottelingData data;
    for(auto& sample : mEntityModel->getEntities(TypeSample))
    {
      auto passport = sample->getData(RolePassport).value<TPassport>();
      std::string groupName = "Unknown";
      if (criterion == tr("field"))
        groupName = passport.value(PassportTags::field).toString().toStdString();
      if (criterion == tr("layer"))
        groupName = passport.value(PassportTags::layer).toString().toStdString();
      if (criterion == tr("well"))
        groupName = passport.value(PassportTags::well).toString().toStdString();
      auto componentsData = sample->getData(RoleComponentsData)
                                .value<TComponentsData>();
      DoubleVector tValues(componentsData.size(), 0);
      for(auto iter = componentsData.constKeyValueBegin(); iter != componentsData.constKeyValueEnd(); iter++)
        tValues[iter->first] = iter->second;
      data[groupName].push_back(tValues);
    }
    model->Set(HottelingRequestModel::T, data);
    logs << LOG_PREFIX(" return model. ");
    return model;
  }

  IdentityModelPtr PcaCountPlotWidget::createHottelingRequestModel(const QHash<QString, QList<QUuid> > &groups, uint gk)
  {
    QStringList logs = { LOG_PREFIX(" Create criteria hotteling request model. ") };
    QScopeGuard guard([&]() { SaveToLog(logs); });
    auto model = std::make_shared<HottelingRequestModel>();
    model->Set(HottelingRequestModel::Count, static_cast<size_t>(gk));
    HottelingData data;
    for(auto giter : groups.asKeyValueRange())
    {
      for(auto& sampleId : giter.second)
      {
        auto sample = mEntityModel->getEntity(sampleId);
        auto componentsData = sample->getData(RoleComponentsData)
                                  .value<TComponentsData>();
        DoubleVector tValues(componentsData.size(), 0);
        for(const auto key : componentsData.keys())
        {
          tValues[key - 1] = componentsData.value(key);
        }
        data[giter.first.toStdString()].push_back(tValues);
      }
    }
    model->Set(HottelingRequestModel::T, data);
    logs << LOG_PREFIX(" return model. ");
    return model;
  }

  IdentityModelPtr PcaCountPlotWidget::createHottelingRequestModel(uint gk)
  {
    QStringList logs = { LOG_PREFIX(" Create hotteling request model. ") };
    QScopeGuard guard([&]() { SaveToLog(logs); });
    auto model = std::make_shared<HottelingRequestModel>();
    model->Set(HottelingRequestModel::Count, static_cast<size_t>(gk));
    HottelingData data;
    logs << LOG_PREFIX(" init hotteling data. ");
    for(auto& sample : mEntityModel->getEntities(TypeSample))
    {
      auto groupsUids = sample->getData(RoleGroupsUidSet).value<TGroupsUidSet>();
      for(auto& uid : groupsUids)
      {
        auto groupEnt = mEntityModel->getEntity(uid);
        if(groupEnt->getType() == TypeHottelingGroup && groupEnt->getUid() != mNonParticipantsGroupUid)
        {
          auto groupName = groupEnt->getData(RoleTitle).toString().toStdString();
          auto compData = sample->getData(RoleComponentsData).value<TComponentsData>();
          DoubleVector compValues(compData.size(), 0);
          for(auto iter = compData.constKeyValueBegin(); iter != compData.constKeyValueEnd(); iter++)
            compValues[iter->first] = iter->second;
          data[groupName].push_back(compValues);
        }
      }
    }
    model->Set(HottelingRequestModel::T, data);
    logs << LOG_PREFIX(" return model. ");
    return model;
  }

  void PcaCountPlotWidget::onEntityModelEntityDataChanged(EntityType type, TEntityUid eId, DataRoleType role, const QVariant &value)
  {
    if(mInHottelingManualMode)
    {
      mPlotToolBar->CancelExcludingChoise->setVisible(false);
      return;
    }
    if(type != TypeSample || role != RoleExcluded)
      return;
    int excludedCount = mEntityModel->countEntities([](AbstractEntityDataModel::ConstDataPtr ent)->bool
      {
        return ent->getData(RoleExcluded).toBool() && ent->getType() == TypeSample;
      });
    mPlotToolBar->CancelExcludingChoise->setVisible(excludedCount > 0);
  }

  void PcaCountPlotWidget::onEntityAboutToRemove(EntityType type, TEntityUid eId)
  {
    if(mInHottelingManualMode)
    {

    }
  }

  void PcaCountPlotWidget::onEntityModelAboutToReset()
  {
    if(!mEntityModel)
      return;
    disconnect(mEntityModel, &AnalysisEntityModel::modelDataChanged, this, &PcaCountPlotWidget::onEntityModelDataChanged);
    disconnect(mEntityModel, &AnalysisEntityModel::entityChanged, this, &PcaCountPlotWidget::onEntityModelEntityDataChanged);
    disconnect(mEntityModel, &AnalysisEntityModel::entityAboutToRemove, this, &PcaCountPlotWidget::onEntityAboutToRemove);
  }

  void PcaCountPlotWidget::onEntityModelReset()
  {
    if(!mEntityModel)
      return;
    connect(mEntityModel, &AnalysisEntityModel::modelDataChanged, this, &PcaCountPlotWidget::onEntityModelDataChanged);
    connect(mEntityModel, &AnalysisEntityModel::entityChanged, this, &PcaCountPlotWidget::onEntityModelEntityDataChanged);
    connect(mEntityModel, &AnalysisEntityModel::entityAboutToRemove, this, &PcaCountPlotWidget::onEntityAboutToRemove);
    onEntityModelDataChanged(mEntityModel->modelData());
    onEntityModelEntityDataChanged(TypeSample, QUuid(), RoleExcluded, QVariant());
  }

  void PcaCountPlotWidget::onEntityModelDataChanged(const QHash<int, QVariant> &)
  {
    if(!mEntityModel)
      return;
    auto varianceMap = mEntityModel->modelData(ModelRoleComponentsExplVarianceMap).value<TComponentsExplVarianceMap>();
    const auto xPC = mEntityModel->currentXComponent();
    mPlot->setXAxisLabel(generateAxisLabel(xPC, varianceMap[xPC]));
    const auto yPC = mEntityModel->currentYComponent();
    mPlot->setYAxisLabel(generateAxisLabel(yPC, varianceMap[yPC]));
    mPlotToolBar->SetNameForPCButtons(varianceMap.size());
  }

  void PcaCountPlotWidget::updateHotteling(const QJsonObject& obj)
  {
    auto responseSerializer = ServiceLocator::Instance().Resolve<HottelingResponseModel, IJsonSerializer>();
    IdentityModelPtr responseModel;

    if(!obj.isEmpty())
      responseModel = responseSerializer->ToModel(obj);

    if (auto table = findChild<QTableView*>())
    {
      auto tableModel = table->model();
      if(!tableModel)
      {
        if(!responseModel)
          return;
        tableModel = new HottelingDataModel(this);
      }
      if(auto hottModel = qobject_cast<HottelingDataModel*>(tableModel))
      {
        if(!responseModel)
        {
          hottModel->SetHeaderData(StringList());
          hottModel->SetColors(StringList2D());
          hottModel->SetTableData(StringList2D());
        }
        else
        {
          hottModel->SetHeaderData(responseModel->Get<StringList>(HottelingResponseModel::Headers));
          hottModel->SetColors(responseModel->Get<StringList2D>(HottelingResponseModel::Colors));
          hottModel->SetTableData(responseModel->Get<StringList2D>(HottelingResponseModel::Values));
        }
      }
      else
      {
        tableModel->deleteLater();
        tableModel = new HottelingDataModel(this);
      }
      mHotellingLoaded = tableModel->rowCount() > 0;
      table->setModel(tableModel);
      table->setMinimumHeight(table->sizeHint().height());
      table->setMaximumHeight(table->sizeHint().height());
    }
  }

  void PcaCountPlotWidget::createHottelingContextMenu(QContextMenuEvent *event)
  {
    auto selected = mPlot->selectedItems();

    QWidgetAction* tumblersAction = nullptr;
    Tumbler* showNamesTumbler = nullptr;
    Tumbler* showPassportTumbler = nullptr;

    // if(mPlot->namesHidden() || mPlot->passportsHidden())
    {
      tumblersAction = new QWidgetAction(mContextMenu);
      auto tumblersWidget = new QWidget;
      auto tumblersLayout = new QVBoxLayout;
      tumblersWidget->setLayout(tumblersLayout);
      // if(mPlot->namesHidden())
      {
        showNamesTumbler = new Tumbler(tr("Show names"));
        tumblersLayout->addWidget(showNamesTumbler);
      }
      // if(mPlot->passportsHidden())
      {
        showPassportTumbler = new Tumbler(tr("Show passport"));
        tumblersLayout->addWidget(showPassportTumbler);
      }
      tumblersAction->setDefaultWidget(tumblersWidget);
    }

    QAction* cMenuTitleAction = mContextMenu->addAction("");
    cMenuTitleAction->setEnabled(false);
    mContextMenu->addSeparator();

    if (selected.size() == 1)
    {
      auto item = qobject_cast<GPShapeWithLabelItem*>(selected.first());
      if (!item)
      {
        return;
      }
      auto entity = mEntityModel->getEntity(item->property(AnalysisPlotRefactor::kEntityUid).toUuid());

      if(showNamesTumbler)
      {
        if(entity->getData(RoleForceShowName).toBool())
          showNamesTumbler->setChecked(true);

        connect(showNamesTumbler, &Tumbler::toggled, this, [this, entity](bool checked)
                {
                  mEntityModel->setEntityData(entity->getUid(), RoleForceShowName, checked);
                });
      }

      if(showPassportTumbler)
      {
        if(entity->getData(RoleForceShowPassport).toBool())
          showPassportTumbler->setChecked(true);
        connect(showPassportTumbler, &Tumbler::toggled, this, [this, entity](bool checked)
                {
                  mEntityModel->setEntityData(entity->getUid(), RoleForceShowPassport, checked);
                });
      }
      if(tumblersAction)
      {
        mContextMenu->addAction(tumblersAction);
      }


      auto insertIntoGroup = mContextMenu->addAction(tr("Insert into group"));
      connect(insertIntoGroup, &QAction::triggered, this,
              [this, entity]()
              {
                auto dial = new Dialogs::LegendGroupCreator({entity->getUid()}, mEntityModel, true, this);
                dial->Open();
              });
      insertIntoGroup->setEnabled(true);

      cMenuTitleAction->setText(item->getLabelText().toUpper());
      // cMenuTitleAction->setText(tr("Add in group"));
      // auto groups = mEntityModel->getEntities(TypeHottelingGroup);
      // for (auto& group : groups)
      // {
      //   QIcon groupIcon = QIcon::fromTheme("folder");
      //   QAction* addToGroupAction = mContextMenu->addAction(groupIcon, group->getData(RoleGroupName).toString());
      //   connect(addToGroupAction, &QAction::triggered, this,
      //           [this, group, entity]()
      //           {
      //             for(auto& groupUid : entity->getData(RoleGroupsUidSet).value<TGroupsUidSet>())
      //             {
      //               auto gent = mEntityModel->getEntity(groupUid);
      //               if(gent->getType() == TypeHottelingGroup)
      //                 mEntityModel->resetEntityGroup(entity->getUid(), groupUid);
      //             }
      //             mEntityModel->setEntityGroup(entity->getUid(), group->getUid());
      //             mEntityModel->setEntityData(entity->getUid(), RoleExcluded, false);
      //           });
      // }

      // QAction* CreateGroupAction = mContextMenu->addAction(tr("Create group"));
      // connect(CreateGroupAction, &QAction::triggered, this,
      //         [this, entity]()
      //         {
      //           auto addGroupLambda = [this, entity](WebDialogCreateGroupManualHotteling* dialog) {
      //             auto groupUid = mEntityModel->addNewEntity(TypeHottelingGroup);
      //             mEntityModel->setEntityData(groupUid, RoleGroupName, dialog->getNameGroup());
      //             mEntityModel->setEntityData(groupUid, RoleColor, randomColor());
      //             mEntityModel->setEntityData(groupUid, RoleShape, QVariant::fromValue(randomShape()));
      //             mEntityModel->setEntityData(groupUid, RoleShapePixelSize, 10);
      //             for(auto& groupUid : entity->getData(RoleGroupsUidSet).value<TGroupsUidSet>())
      //             {
      //               auto gent = mEntityModel->getEntity(groupUid);
      //               if(gent->getType() == TypeHottelingGroup)
      //                 mEntityModel->resetEntityGroup(entity->getUid(), groupUid);
      //             }
      //             mEntityModel->setEntityGroup(entity->getUid(), groupUid);
      //             mEntityModel->setEntityData(entity->getUid(), RoleExcluded, false);
      //             dialog->Done(QDialog::Accepted);
      //           };

      //           QSet<QString> occupiedNames;
      //           for(auto& group : mEntityModel->getEntities(TypeHottelingGroup))
      //             occupiedNames << group->getData(RoleGroupName).toString();

      //           auto dialog = new WebDialogCreateGroupManualHotteling(this, occupiedNames);
      //           dialog->setGroupAddingFunc(addGroupLambda);
      //           dialog->Open();
      //         });

      // QAction* DeleteGroupAction = mContextMenu->addAction(tr("Remove group"));
      // TEntityUid hottGroupUid;
      // for(auto& groupUid : entity->getData(RoleGroupsUidSet).value<TGroupsUidSet>())
      // {
      //   auto group = mEntityModel->getEntity(groupUid);
      //   if(group->getType() == TypeHottelingGroup)
      //   {
      //     hottGroupUid = group->getUid();
      //     break;
      //   }
      // }
      // DeleteGroupAction->setEnabled(!hottGroupUid.isNull());
      // connect(DeleteGroupAction, &QAction::triggered, this,
      //         [this, entity, hottGroupUid]()
      //         {
      //           mEntityModel->resetEntityGroup(entity->getUid(), hottGroupUid);
      //           mEntityModel->setEntityData(entity->getUid(), RoleExcluded, true);
      //         });
      auto removeFromGroupAction = mContextMenu->addAction(tr("Remove from group"));
      removeFromGroupAction->setEnabled(false);
      connect(removeFromGroupAction, &QAction::triggered, this,
              [this, entity]()
              {
                excludeFromGroup({entity->getUid()});
              });
      for(auto& groupUid : entity->getData(RoleGroupsUidSet).value<TGroupsUidSet>())
      {
        if(groupUid != mNonParticipantsGroupUid)
        {
          removeFromGroupAction->setEnabled(true);
          break;
        }
      }

    }
    if (!selected.isEmpty() && selected.size() != 1)
    { //multiple selection


      QList<AbstractEntityDataModel::ConstDataPtr> entList;
      for(auto& item : selected)
        entList << mEntityModel->getEntity(item->property(AnalysisPlotRefactor::kEntityUid).toUuid());

      cMenuTitleAction->setText(tr("SELECTED %n SAMPLES", "", selected.size()));

      if(showNamesTumbler)
      {
        for (const auto& ent : entList)
        {
          if(ent->getData(RoleForceShowName).toBool())
          {
            showNamesTumbler->setChecked(true);
            break;
          }
        }
        connect(showNamesTumbler, &Tumbler::toggled, this, [this, entList](bool checked)
                {
                  for (const auto& ent : entList)
                    mEntityModel->setEntityData(ent->getUid(), RoleForceShowName, checked);
                });
      }

      if(showPassportTumbler)
      {
        for (const auto& ent : entList)
        {
          if(ent->getData(RoleForceShowPassport).toBool())
          {
            showPassportTumbler->setChecked(true);
            break;
          }
        }
        connect(showPassportTumbler, &Tumbler::toggled, this, [this, entList](bool checked)
                {
                  for (const auto& ent : entList)
                    mEntityModel->setEntityData(ent->getUid(), RoleForceShowPassport, checked);
                });
      }
      if(tumblersAction)
      {
        mContextMenu->addAction(tumblersAction);
      }

     // auto groups = mEntityModel->getEntities(TypeHottelingGroup);
     // for (auto& group : groups)
     // {
     //   QIcon groupIcon = QIcon::fromTheme("folder");
     //   QAction* addToGroupAction = mContextMenu->addAction(groupIcon, group->getData(RoleGroupName).toString());
     //   connect(addToGroupAction, &QAction::triggered, this,
     //           [this, group, entList]()
     //           {
     //             for(auto& entity : entList)
     //             {
     //               for(auto& groupUid : entity->getData(RoleGroupsUidSet).value<TGroupsUidSet>())
     //               {
     //                 auto gent = mEntityModel->getEntity(groupUid);
     //                 if(gent->getType() == TypeHottelingGroup)
     //                   mEntityModel->resetEntityGroup(entity->getUid(), groupUid);
     //               }
     //               mEntityModel->setEntityGroup(entity->getUid(), group->getUid());
     //               mEntityModel->setEntityData(entity->getUid(), RoleExcluded, false);
     //             }
     //           });
     // }

     // QAction* CreateGroupAction = mContextMenu->addAction(tr("Create group"));
     // connect(CreateGroupAction, &QAction::triggered, this,
     //         [this, entList]()
     //         {
     //           auto addGroupLambda = [this, entList](WebDialogCreateGroupManualHotteling* dialog) {
     //             auto groupUid = mEntityModel->addNewEntity(TypeHottelingGroup);
     //             mEntityModel->setEntityData(groupUid, RoleGroupName, dialog->getNameGroup());
     //             mEntityModel->setEntityData(groupUid, RoleColor, randomColor());
     //             mEntityModel->setEntityData(groupUid, RoleShape, QVariant::fromValue(randomShape()));
     //             mEntityModel->setEntityData(groupUid, RoleShapePixelSize, 10);
     //             for(auto& entity : entList)
     //             {
     //               for(auto& groupUid : entity->getData(RoleGroupsUidSet).value<TGroupsUidSet>())
     //               {
     //                 auto gent = mEntityModel->getEntity(groupUid);
     //                 if(gent->getType() == TypeHottelingGroup)
     //                   mEntityModel->resetEntityGroup(entity->getUid(), groupUid);
     //               }
     //               mEntityModel->setEntityGroup(entity->getUid(), groupUid);
     //               mEntityModel->setEntityData(entity->getUid(), RoleExcluded, false);
     //             }
     //             dialog->Done(QDialog::Accepted);
     //           };

     //           QSet<QString> occupiedNames;
     //           for(auto& group : mEntityModel->getEntities(TypeHottelingGroup))
     //             occupiedNames << group->getData(RoleGroupName).toString();

     //           auto dialog = new WebDialogCreateGroupManualHotteling(this, occupiedNames);
     //           dialog->setGroupAddingFunc(addGroupLambda);
     //           dialog->Open();
     //         });
      auto insertIntoGroup = mContextMenu->addAction(tr("Insert into group"));
      connect(insertIntoGroup, &QAction::triggered, this,
              [this, entList]()
              {
                QList<TEntityUid> uids;
                for(auto& ent : entList)
                  uids << ent->getUid();
                auto dial = new Dialogs::LegendGroupCreator(uids, mEntityModel, true, this);
                dial->Open();
              });
      insertIntoGroup->setEnabled(true);

      // QAction* DeleteGroupAction = mContextMenu->addAction(tr("Remove group"));
      // DeleteGroupAction->setEnabled(false);

      // bool hasGroup = false;
      // for(auto& entity : entList)
      // {
      //   for(auto& groupUid : entity->getData(RoleGroupsUidSet).value<TGroupsUidSet>())
      //   {
      //     auto group = mEntityModel->getEntity(groupUid);
      //     if(group->getType() == TypeHottelingGroup)
      //     {
      //       hasGroup = true;
      //       break;
      //     }
      //   }
      //   if(hasGroup)
      //     break;
      // }
      // DeleteGroupAction->setEnabled(hasGroup);
      // connect(DeleteGroupAction, &QAction::triggered, this,
      //         [this, entList]()
      //         {
      //           for(auto& entity : entList)
      //           {
      //             auto groupsUids = entity->getData(RoleGroupsUidSet).value<TGroupsUidSet>();
      //             groupsUids.removeIf([this](const TEntityUid& uid)
      //                                 {
      //                                   return mEntityModel->getEntity(uid)->getType() != TypeHottelingGroup;
      //                                 });
      //             for(auto& groupUid : groupsUids)
      //               mEntityModel->resetEntityGroup(entity->getUid(), groupUid);
      //             mEntityModel->setEntityData(entity->getUid(), RoleExcluded, true);
      //           }
      //         });
      auto removeFromGroupAction = mContextMenu->addAction(tr("Remove from group"));
      removeFromGroupAction->setEnabled(false);
      connect(removeFromGroupAction, &QAction::triggered, this,
              [this, entList]()
              {
                QList<TEntityUid> uids;
                for(auto& ent : entList)
                  uids << ent->getUid();
                excludeFromGroup(uids);
              });
      for(auto& ent : entList)
      {
        for(auto& groupUid : ent->getData(RoleGroupsUidSet).value<TGroupsUidSet>())
        {
          if(groupUid != mNonParticipantsGroupUid)
          {
            removeFromGroupAction->setEnabled(true);
            break;
          }
        }
        if(removeFromGroupAction->isEnabled())
          break;
      }
    }
  }

  void PcaCountPlotWidget::createContextMenu(QContextMenuEvent *event)
  {
    auto selected = mPlot->selectedItems();
    auto excludedItemsCount = mEntityModel->countEntities({{AbstractEntityDataModel::RoleEntityType, TypeSample},
                                                           {RoleExcluded, true}});
    QWidgetAction* tumblersAction = nullptr;
    Tumbler* showNamesTumbler = nullptr;
    Tumbler* showPassportTumbler = nullptr;

    // if(mPlot->namesHidden() || mPlot->passportsHidden())
    {
      tumblersAction = new QWidgetAction(mContextMenu);
      auto tumblersWidget = new QWidget;
      auto tumblersLayout = new QVBoxLayout;
      tumblersWidget->setLayout(tumblersLayout);
      tumblersLayout->setSpacing(10);
      // if(mPlot->namesHidden())
      {
        showNamesTumbler = new Tumbler(tr("Show names"));
        tumblersLayout->addWidget(showNamesTumbler);
      }
      // if(mPlot->passportsHidden())
      {
        showPassportTumbler = new Tumbler(tr("Show passport"));
        tumblersLayout->addWidget(showPassportTumbler);
      }
      tumblersAction->setDefaultWidget(tumblersWidget);
    }
    auto pickGroupsAction = new QAction(tr("Select all samples from it's group(s)"), mContextMenu);

    auto item = qobject_cast<GPShapeWithLabelItem*>(mPlot->itemAt(event->pos(), true));
    if (!item && selected.empty())
    {
      //default plot context menu
      mContextMenu->setHidden(true);
      auto nameAct = mContextMenu->addAction(tr("Count plot"));
      nameAct->setEnabled(false);

    }
    else
    {
      mContextMenu->setHidden(false);
      if ((item && !selected.contains(item)) ||
          (!item && selected.size() == 1) ||
          (selected.contains(item) && selected.size() == 1)
          )
      {
        if (!item && selected.size() == 1)
          item = qobject_cast<GPShapeWithLabelItem*>(selected.first());
        //item menu
        auto entity = mEntityModel->getEntity(item->property(AnalysisPlotRefactor::kEntityUid).toUuid());
        auto nameAct = mContextMenu->addAction(item->getLabelText().toUpper());
        nameAct->setEnabled(false);
        mContextMenu->addSeparator();

        if(showNamesTumbler)
        {
          if(entity->getData(RoleForceShowName).toBool())
            showNamesTumbler->setChecked(true);

          connect(showNamesTumbler, &Tumbler::toggled, this, [this, entity](bool checked)
                  {
                    mEntityModel->setEntityData(entity->getUid(), RoleForceShowName, checked);
                  });
        }

        if(showPassportTumbler)
        {
          if(entity->getData(RoleForceShowPassport).toBool())
            showPassportTumbler->setChecked(true);
          connect(showPassportTumbler, &Tumbler::toggled, this, [this, entity](bool checked)
                  {
                    mEntityModel->setEntityData(entity->getUid(), RoleForceShowPassport, checked);
                  });
        }
        if(tumblersAction)
        {
          mContextMenu->addAction(tumblersAction);
        }

        connect(pickGroupsAction, &QAction::triggered, this, [this, entity]()
                {
                  auto groups = entity->getData(RoleGroupsUidSet).value<TGroupsUidSet>();
                  auto affectedUids = mEntityModel->getEntitiesUidsSet([groups](AbstractEntityDataModel::ConstDataPtr ent)->bool
                                                                       {
                                                                         return ent->getData(RoleGroupsUidSet).value<TGroupsUidSet>().intersects(groups);
                                                                       });
                  mPlot->select(affectedUids);
                });
        mContextMenu->addAction(pickGroupsAction);

        if (excludedItemsCount == 0)
        {
          QAction* pickForModel = mContextMenu->addAction(tr("Pick for new model"));
          connect(pickForModel, &QAction::triggered, this,
                  [this, entity]()
                  {
                    for(auto& sample : mEntityModel->getEntities(TypeSample))
                    {
                      if(sample->getUid() != entity->getUid())
                        mEntityModel->setEntityData(sample->getUid(), RoleExcluded, true);
                    }
                  });
        }
        if (!entity->getData(RoleExcluded).toBool())
        {
          QAction* excludeFromModel = mContextMenu->addAction(tr("Pick for excluding from model"));
          connect(excludeFromModel, &QAction::triggered, this,
                  [this, entity]()
                  {
                    mEntityModel->setEntityData(entity->getUid(), RoleExcluded, true);
                  });
          excludeFromModel->setEnabled(true);
        }
        else
        {
          QAction* reincludeToModel = mContextMenu->addAction(tr("Reset excluding from model"));
          connect(reincludeToModel, &QAction::triggered, this,
                  [this, entity]()
                  {
                    mEntityModel->setEntityData(entity->getUid(), RoleExcluded, false);
                  });
        }
        auto insertIntoGroup = mContextMenu->addAction(tr("Insert into group"));
        connect(insertIntoGroup, &QAction::triggered, this,
                [this, entity]()
                {
                  auto dial = new Dialogs::LegendGroupCreator({entity->getUid()}, mEntityModel, false, this);
                  dial->Open();
                });
        insertIntoGroup->setEnabled(true);
        auto removeFromGroupAction = mContextMenu->addAction(tr("Remove from group"));
        removeFromGroupAction->setEnabled(false);
        connect(removeFromGroupAction, &QAction::triggered, this, &PcaCountPlotWidget::confirmRemoveFromGroup);

        for(auto& groupUid : entity->getData(RoleGroupsUidSet).value<TGroupsUidSet>())
        {
          if(mEntityModel->getEntity(groupUid)->getData(RoleGroupUserCreated).toBool())
          {
            removeFromGroupAction->setEnabled(true);
            break;
          }
        }

      }
      else
      {
        //whole selected items context menu
        QList<AbstractEntityDataModel::ConstDataPtr> entList;
        for(auto& item : selected)
          entList << mEntityModel->getEntity(item->property(AnalysisPlotRefactor::kEntityUid).toUuid());

        auto nameAct = mContextMenu->addAction(tr("PICKED %n SAMPLES", "", selected.size()));
        nameAct->setEnabled(false);
        mContextMenu->addSeparator();

        if(showNamesTumbler)
        {
          for (const auto& ent : entList)
          {
            if(ent->getData(RoleForceShowName).toBool())
            {
              showNamesTumbler->setChecked(true);
              break;
            }
          }
          connect(showNamesTumbler, &Tumbler::toggled, this, [this, entList](bool checked)
                  {
                    for (const auto& ent : entList)
                      mEntityModel->setEntityData(ent->getUid(), RoleForceShowName, checked);
                  });
        }

        if(showPassportTumbler)
        {
          for (const auto& ent : entList)
          {
            if(ent->getData(RoleForceShowPassport).toBool())
            {
              showPassportTumbler->setChecked(true);
              break;
            }
          }
          connect(showPassportTumbler, &Tumbler::toggled, this, [this, entList](bool checked)
                  {
                    for (const auto& ent : entList)
                      mEntityModel->setEntityData(ent->getUid(), RoleForceShowPassport, checked);
                  });
        }
        if(tumblersAction)
        {
          mContextMenu->addAction(tumblersAction);
        }

        mContextMenu->addSeparator();

        connect(pickGroupsAction, &QAction::triggered, this, [this, entList]()
                {
                  TGroupsUidSet groups;
                  for(auto& ent : entList)
                    groups.unite(ent->getData(RoleGroupsUidSet).value<TGroupsUidSet>());

                  auto affectedUids = mEntityModel->getEntitiesUidsSet([groups](AbstractEntityDataModel::ConstDataPtr ent)->bool
                                                                       {
                                                                         return ent->getData(RoleGroupsUidSet).value<TGroupsUidSet>().intersects(groups);
                                                                       });
                  mPlot->select(affectedUids);
                });
        mContextMenu->addAction(pickGroupsAction);

        if (excludedItemsCount == 0)
        {
          QAction* pickForModel = mContextMenu->addAction(tr("Pick for new model"));
          connect(pickForModel, &QAction::triggered, this,
                  [this, entList]()
                  {
                    for(auto& sample : mEntityModel->getEntities(TypeSample))
                    {
                      if(!entList.contains(sample))
                        mEntityModel->setEntityData(sample->getUid(), RoleExcluded, true);
                    }
                  });
          pickForModel->setEnabled(true);
        }
        bool bHasNonExcludedItem = false;
        {
          for (auto& ent : entList)
          {
            if(ent->getData(RoleExcluded).toBool() == false)
            {
              bHasNonExcludedItem = true;
              break;
            }
          }
        }
        if (bHasNonExcludedItem)
        {
          QAction* excludeFromModel = mContextMenu->addAction(tr("Pick for excluding from model"));
          connect(excludeFromModel, &QAction::triggered, this,
                  [this, entList]()
                  {
                    for(auto& ent : entList)
                      mEntityModel->setEntityData(ent->getUid(), RoleExcluded, true);
                  });
          excludeFromModel->setEnabled(true);
        }

        bool bHasExcludedItems = false;
        {
          for (auto& ent : entList)
          {
            if(ent->getData(RoleExcluded).toBool() == true)
            {
              bHasExcludedItems = true;
              break;
            }
          }
        }
        if (bHasExcludedItems)
        {
          QAction* reincludeToModel = mContextMenu->addAction(tr("Reset excluding from model"));
          connect(reincludeToModel, &QAction::triggered, this,
                  [this, entList]()
                  {
                    for(auto& ent : entList)
                      mEntityModel->setEntityData(ent->getUid(), RoleExcluded, false);
                  });
          reincludeToModel->setEnabled(true);
        }
        auto insertIntoGroup = mContextMenu->addAction(tr("Insert into group"));
        connect(insertIntoGroup, &QAction::triggered, this,
                [this, entList]()
                {
                  QList<TEntityUid> uids;
                  for(auto& ent : entList)
                    uids << ent->getUid();
                  auto dial = new Dialogs::LegendGroupCreator(uids, mEntityModel, false, this);
                  dial->Open();
                });
        insertIntoGroup->setEnabled(true);
        auto removeFromGroupAction = mContextMenu->addAction(tr("Remove from group"));
        removeFromGroupAction->setEnabled(false);
        connect(removeFromGroupAction, &QAction::triggered, this, &PcaCountPlotWidget::confirmRemoveFromGroup);
        for(auto& ent : entList)
        {
          for(auto& groupUid : ent->getData(RoleGroupsUidSet).value<TGroupsUidSet>())
          {
            if(mEntityModel->getEntity(groupUid)->getData(RoleGroupUserCreated).toBool())
            {
              removeFromGroupAction->setEnabled(true);
              break;
            }
          }
          if(removeFromGroupAction->isEnabled())
            break;
        }
      }
    }
  }

  void PcaCountPlotWidget::confirmRemoveFromGroup()
  {
    auto settings = Dialogs::Templates::Confirm::Settings();
    settings.dialogHeader = tr("Warning");
    settings.buttonsProperties = { { QDialogButtonBox::Ok, {{"red", true}} } };
    settings.buttonsNames = { { QDialogButtonBox::Ok , tr("Continue") } };
    auto label = new QLabel(tr("When you remove samples from custom groups, the selected samples will go into groups that match the initial grouping settings."));
    auto confirm = new Dialogs::Templates::Confirm(this, settings, QDialogButtonBox::Ok | QDialogButtonBox::Cancel, label);
    connect(confirm, &Dialogs::Templates::Confirm::Accepted, this, [&]()
      {
        auto selected = mPlot->selectedItems();
        QList<TEntityUid> uids;
        for (auto& item : selected)
          uids << mEntityModel->getEntity(item->property(AnalysisPlotRefactor::kEntityUid).toUuid())->getUid();
        excludeFromGroup(uids);
      });
    confirm->Open();

  }

  QColor PcaCountPlotWidget::randomColor()
  {
    QVector<GPShapeItem::DefaultColor> exceptColors =
        { GPShapeItem::Gray,
            GPShapeItem::DarkGray,
        GPShapeItem::PaleGray
        };

    auto grandom = QRandomGenerator::global();
    GPShapeItem::DefaultColor color;
    do
    {
      color = static_cast<GPShapeItem::DefaultColor>(grandom->bounded(GPShapeItem::LastColor - 1));
    } while (exceptColors.contains(color));
    return GPShapeItem::ToColor(color);
  }

  QPainterPath PcaCountPlotWidget::randomShape()
  {
    QVector<GPShape::ShapeType> exceptShapes =
        { GPShape::Circle
        };

    auto grandom = QRandomGenerator::global();
    GPShape::ShapeType shape;
    do
    {
      shape = static_cast<GPShape::ShapeType>(grandom->bounded(GPShape::LastShape - 1));
    } while (exceptShapes.contains(shape));
    return GPShape::GetPath(shape);
  }

  void PcaCountPlotWidget::grayAll(bool gray)
  {
    // for(auto& sample : mEntityModel->getEntities(TypeSample))
    //   mEntityModel->setEntityData(sample->getUid(), RoleExcluded, gray);
  }

  void Widgets::PcaCountPlotWidget::excludeFromGroup(QList<TEntityUid> uids)
  {
    auto groupType = mInHottelingManualMode ? TypeHottelingGroup : TypeGroup;
    if(mInHottelingManualMode)
    {
      for(auto& eid : uids)
      {
        mEntityModel->resetEntityGroup(eid, TypeHottelingGroup);
        mEntityModel->setEntityGroup(eid, mNonParticipantsGroupUid);
      }
    }
    else
    {//remove ent-s from user groups, and sort them into right system groups
      auto groups = mEntityModel->getEntities(TypeGroup);//take all existed groups
      for(auto& eid : uids)//for all of input entities
      {
        auto ent = mEntityModel->getEntity(eid);
        auto groupsUidSet = ent->getData(RoleGroupsUidSet).value<TGroupsUidSet>();
        for(auto& gid : groupsUidSet)//walk through all entity groups
        {
          auto gent = mEntityModel->getEntity(gid);
          if(!gent->getData(RoleGroupUserCreated).toBool())
            continue;//if group is system, skip
          //if group is user created
          mEntityModel->resetEntityGroup(eid, gid);//take ent-s from this group
          for(auto& group : groups)//walk through all groups
          {
            if(group->getData(RoleGroupUserCreated).toBool()
                || !group->hasDataAndItsValid(RolePassportFilter))
              continue;//if it's not system group - skip
            //if group is system, check it's filter
            auto filter = group->getData(RolePassportFilter).value<TPassportFilter>();
            auto passport = ent->getData(RolePassport).value<TPassport>();
            if(filter.match(passport))
            {//if filter match, assign this entity into this group, exit cycle
              mEntityModel->setEntityGroup(ent->getUid(), group->getUid());
              break;
            }
          }
        }
      }
    }
  }

  void PcaCountPlotWidget::rescaleXAxisToYAxis()
  {
    if(mPlotToolBar->SwitchScale->checkState() == Qt::Checked)
      mPlot->xAxis->setScaleRatio(mPlot->yAxis);
  }

  void PcaCountPlotWidget::rescaleYAxisToXAxis()
  {
    if (mPlotToolBar->SwitchScale->checkState() == Qt::Checked)
      mPlot->yAxis->setScaleRatio(mPlot->xAxis);
  }
}//namespace Widgets
