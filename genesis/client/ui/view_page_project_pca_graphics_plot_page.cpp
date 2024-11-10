#include "view_page_project_pca_graphics_plot_page.h"

#include "known_view_names.h"
#include "api/api_rest.h"
#include "genesis_style/style.h"
#include "logic/tree_model.h"
#include "genesis_window.h"
#include "contexts/new_model_settings_context.h"
#include "contexts/exist_model_settings_context.h"
#include "dialogs/web_dialog_analysis_model_settings.h"
#include "logic/service/ijson_serializer.h"
#include "logic/service/service_locator.h"
#include "logic/context_root.h"
#include "logic/enums.h"
#include "logic/known_json_tag_names.h"
#include "logic/known_context_tag_names.h"
#include "logic/models/pca_analysis_request_model.h"
#include "logic/models/pca_analysis_response_model.h"
#include "logic/models/pca_analysis_identity_model.h"
#include "logic/notification.h"
#include "ui/widgets/pca_load_plot_widget.h"
#include "ui/widgets/pca_count_plot_widget.h"
#include "widgets/model_settings_widget.h"
#include <ui/plots/analysis_plot_refactor.h>
#include "widgets/pca_bars_plot_widget.h"
#include "widgets/tab_widget.h"
#include "ui/dialogs/templates/save_edit.h"
#include "ui/dialogs/templates/confirm.h"
#include "ui/dialogs/export_tables_and_plots_dialog.h"

#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QRegExp>

using namespace Model;
using namespace Structures;
using namespace Names;
using namespace Service;
using namespace Widgets;
using namespace Core;
using namespace Constants;
using namespace AnalysisEntity;

namespace
{
const static int MinimumCount = 3;
}

namespace Views
{
ViewPageProjectPcaGraphicsPlotPage::ViewPageProjectPcaGraphicsPlotPage(QWidget* parent)
  : View(parent)
{
  setupUi();
}

void ViewPageProjectPcaGraphicsPlotPage::ApplyContextMarkupVersionAnalysisPCA(const QString& dataId, const QVariant& data)
{
  //TODO: make update partial
  if(isDataReset(dataId, data))
  {
    auto context = GenesisContextRoot::Get()->GetContextMarkupVersionAnalisysPCA();
    auto id = context->GetData(JsonTagNames::analysis_id).toInt();
    if (id > 0)
    {
      loadResponseModelFromServer(id);
    }
    else
    {
      const auto object = context->GetData(ContextTagNames::PCAData).toJsonObject();
      const auto jsonResponseSerializer = ServiceLocator::Instance().Resolve<PcaAnalysisResponseModel, IJsonSerializer>();
      mPcaResponseModel = jsonResponseSerializer->ToModel(object);
      createPcaModel();
      setEntityModel(extractEntitiesFromPcaModel(mPcaModel));
      loadGraphicsFromEntityModel();
    }
  }
}

IdentityModelPtr ViewPageProjectPcaGraphicsPlotPage::getIdentityModel() const
{
  return mPcaModel;
}

void ViewPageProjectPcaGraphicsPlotPage::setupUi()
{
  //// Self
  setStyleSheet(Style::Genesis::GetUiStyle());

  //// Layout
  const auto layout = new QVBoxLayout(this);

  layout->setContentsMargins(Style::Scale(40), Style::Scale(32), Style::Scale(40), Style::Scale(32));
  layout->setSpacing(0);

  auto toolBarLayout = new QHBoxLayout;
  toolBarLayout->setContentsMargins(0, 0, 0, 0);
  auto caption = new QLabel(tr("Current PCA"));
  caption->setStyleSheet(Style::Genesis::GetH1());
  toolBarLayout->addWidget(caption);

  toolBarLayout->addStretch();

  mRecalcButton = new QPushButton(tr("Recalculate model"));
  mRecalcButton->setVisible(false);
  mRecalcButton->setProperty("secondary", true);
  connect(mRecalcButton, &QPushButton::clicked, this, &ViewPageProjectPcaGraphicsPlotPage::openModelParameters);

  toolBarLayout->addWidget(mRecalcButton);
  toolBarLayout->addWidget(new ModelSettingsWidget(this));

  layout->addLayout(toolBarLayout);

  auto tabContainer = new QWidget(this);
  layout->addWidget(tabContainer);
  tabContainer->setProperty("style", "white_base");
  tabContainer->setLayout(new QHBoxLayout());
  tabContainer->layout()->setContentsMargins(26, 26, 26, 26);

  const auto plotTabWidget = new TabWidget(tabContainer);
  tabContainer->layout()->addWidget(plotTabWidget);
  mCountPlot = new PcaCountPlotWidget(this);
  caption->setProperty("stored_original_text", caption->text());
  connect(mCountPlot.data(), &PcaCountPlotWidget::pageTitleCaptionOverride, caption, &QLabel::setText);
  connect(mCountPlot.data(), &PcaCountPlotWidget::restorePageTitle, this, [caption]()
          {
            caption->setText(caption->property("stored_original_text").toString());
          });

  QPointer<PcaLoadPlotWidget> loadPlot = new PcaLoadPlotWidget(this);

  connect(this, &ViewPageProjectPcaGraphicsPlotPage::pcaModelSavingFinished, this, []() {TreeModel::ResetInstances("TreeModelDynamicProjectModelsList"); });

  mPlotWidgets = {
                 { tr("Count plot"), mCountPlot },
                 { tr("Load plot"), loadPlot },
                 { tr("Bars plot"), new PcaBarsPlotWidget(this) },
                };

  for (const auto& [name, widget] : mPlotWidgets)
    plotTabWidget->AddTabWidget(widget.get(), name);

  for (const auto& [name, widget] : mPlotWidgets)
  {
    auto plot = widget->findChild<AnalysisPlotRefactor*>();
    if (plot)
      mPlots[name] = plot;
  }
}

void ViewPageProjectPcaGraphicsPlotPage::loadResponseModelFromServer(int analysis_id)
{
  API::REST::GetAnalysis(analysis_id,
    [&, analysis_id](QNetworkReply*, QJsonDocument doc)
    {
      if (const auto json = doc.object(); json.value(JsonTagNames::Error).toBool())
      {
        Notification::NotifyError(tr("Error : %1 while loading analysis.").arg(json.value(JsonTagNames::Status).toString()));
      }
      else
      {
        // qDebug().noquote() << doc.toJson();
        const auto jsonObject = json.value(JsonTagNames::Data).toObject();
        const auto modelSerializer = ServiceLocator::Instance().Resolve<PcaAnalysisIdentityModel, IJsonSerializer>();
        assert(modelSerializer);
        mPcaModel = modelSerializer->ToModel(jsonObject);
        mPcaModel->Set<size_t>(PcaAnalysisIdentityModel::AnalysisId, analysis_id);
        mPcaResponseModel = convertPcaModelToResponse(mPcaModel);
        mPcaModel->Exec<QJsonObject>(PcaAnalysisIdentityModel::Hotteling, [&](const QJsonObject& object)
          {
            mCountPlot->updateHotteling(object);
          });
        setEntityModel(extractEntitiesFromPcaModel(mPcaModel));
        loadGraphicsFromEntityModel();
      }
    },
    [](QNetworkReply*, QNetworkReply::NetworkError)
    {
      Notification::NotifyError(tr("Error : while loading analysis."));
    });
}

void ViewPageProjectPcaGraphicsPlotPage::loadGraphicsFromEntityModel()
{
  for(auto& [name, plot] : mPlotWidgets)
    plot->setModel(mEntityModel);

  updateModelMenu();
}

void ViewPageProjectPcaGraphicsPlotPage::updateModelMenu()
{
  if (const auto modelMenuButton = findChild<ModelSettingsWidget*>())
  {
    if (mMenuContext)
      disconnectMenuSignals();

    const auto id = mPcaModel->Get<size_t>(PcaAnalysisIdentityModel::AnalysisId);
    if (id > 0)
    {
      const auto existModelContext = new ExistModelSettingsContext(modelMenuButton);
      connect(existModelContext, &ExistModelSettingsContext::SaveAsNew, this, &ViewPageProjectPcaGraphicsPlotPage::savePcaModel);
      connect(existModelContext, &ExistModelSettingsContext::SaveChanges, this, &ViewPageProjectPcaGraphicsPlotPage::saveChangesPcaModel);
      mMenuContext = existModelContext;
    }
    else
    {
      const auto newModelContext = new NewModelSettingsContext(modelMenuButton);
      connect(newModelContext, &NewModelSettingsContext::SaveNewModel, this, &ViewPageProjectPcaGraphicsPlotPage::savePcaModel);
      mMenuContext = newModelContext;
    }

    connectMenuSignals();
    modelMenuButton->SetMenu(mMenuContext);
  }
}

IdentityModelPtr ViewPageProjectPcaGraphicsPlotPage::copyModelExceptId()
{
  auto model = std::make_shared<PcaAnalysisIdentityModel>();

  model->Set<size_t>(PcaAnalysisIdentityModel::AnalysisId, 0);
  mPcaModel->Exec<AnalysisData>(PcaAnalysisIdentityModel::AnalysisData,
                               [&](const AnalysisData& data)
                               {
                                 model->Set<AnalysisData>(PcaAnalysisIdentityModel::AnalysisData, data);
                               });

  mPcaModel->Exec<size_t>(PcaAnalysisIdentityModel::AnalysisTypeId,
                         [&](const size_t& id)
                         {
                           model->Set<size_t>(PcaAnalysisIdentityModel::AnalysisTypeId, id);
                         });

  mPcaModel->Exec<size_t>(PcaAnalysisIdentityModel::AnalysisTableId,
                         [&](const size_t& id)
                         {
                           model->Set<size_t>(PcaAnalysisIdentityModel::AnalysisTableId, id);
                         });

  mPcaModel->Exec<Parameters>(PcaAnalysisIdentityModel::AnalysisParameters,
                             [&](const Parameters& params)
                             {
                               model->Set<Parameters>(PcaAnalysisIdentityModel::AnalysisParameters, params);
                             });

  mPcaModel->Exec<AnalysisTable>(PcaAnalysisIdentityModel::AnalysisTable,
                                [&](const AnalysisTable& params)
                                {
                                  model->Set<AnalysisTable>(PcaAnalysisIdentityModel::AnalysisTable, params);
                                });

  mPcaModel->Exec<QByteArray>(PcaAnalysisIdentityModel::ExportRequestBody,
                             [&](const QByteArray& requestBody)
                             {
                               model->Set<QByteArray>(PcaAnalysisIdentityModel::ExportRequestBody, requestBody);
                             });

  mPcaModel->Exec<bool>(PcaAnalysisIdentityModel::Normalization,
                       [&](const bool& norm)
                       {
                         model->Set<bool>(PcaAnalysisIdentityModel::Normalization, norm);
                       });

  mPcaModel->Exec<bool>(PcaAnalysisIdentityModel::Autoscale,
                       [&](const bool& autoscale)
                       {
                         model->Set<bool>(PcaAnalysisIdentityModel::Autoscale, autoscale);
                       });

  mPcaModel->Exec<bool>(PcaAnalysisIdentityModel::PC9599,
                       [&](const bool& pc9599)
                       {
                         model->Set<bool>(PcaAnalysisIdentityModel::PC9599, pc9599);
                       });
  mPcaModel->Exec<QStringList>(PcaAnalysisIdentityModel::PassportKeys,
                        [&](const QStringList& list)
                        {
                          model->Set<QStringList>(PcaAnalysisIdentityModel::PassportKeys, list);
                        });
  mPcaModel->Exec<QStringList>(PcaAnalysisIdentityModel::PassportHeaders,
                        [&](const QStringList& list)
                        {
                          model->Set<QStringList>(PcaAnalysisIdentityModel::PassportHeaders, list);
                        });

  return model;
}

void ViewPageProjectPcaGraphicsPlotPage::updateModelByResponse(Model::IdentityModelPtr model)
{
  model->Set(PcaAnalysisIdentityModel::Hotteling, mCountPlot->hottelingExportRequestBody());
  if(mEntityModel)
    model->Set(PcaAnalysisIdentityModel::EntityModel, mEntityModel);
}

void ViewPageProjectPcaGraphicsPlotPage::getAnalysis(const QJsonObject& jsonRequest)
{
  auto overlayId = GenesisWindow::Get()->ShowOverlay(tr("Loading data"));
  API::REST::GetTableAnalysisPCA(jsonRequest,
    [&, overlayId](QNetworkReply*, QJsonDocument doc)
    {
      // qDebug().noquote() << doc.toJson();
      const auto obj = doc.object();
      if (obj.value(JsonTagNames::Error).toBool())
      {
        GenesisWindow::Get()->RemoveOverlay(overlayId);
        Notification::NotifyError(
          tr("PCA error: %1")
            .arg(obj.value(JsonTagNames::Msg).toString()));
      }
      else
      {
        const auto jsonResponseSerializer = ServiceLocator::Instance().Resolve<PcaAnalysisResponseModel, IJsonSerializer>();
        mPcaResponseModel = jsonResponseSerializer->ToModel(obj.value(JsonTagNames::Result).toObject());
        createPcaModel();
        setEntityModel(extractEntitiesFromPcaModel(mPcaModel));
        loadGraphicsFromEntityModel();
        GenesisWindow::Get()->RemoveOverlay(overlayId);
      }
      mIsRebuildModel = false;
    },
    [overlayId](QNetworkReply*, QNetworkReply::NetworkError)
    {
      Notification::NotifyError("Netowrk error while receiving data.");
      GenesisWindow::Get()->RemoveOverlay(overlayId);
    });
}

void ViewPageProjectPcaGraphicsPlotPage::onEntityChanged(EntityType type, TEntityUid eId, DataRoleType role, const QVariant &value)
{
  if((type == TypeSample || type == TypePeak) && role == RoleExcluded)
  {
    onModelReset();
  }
}

void ViewPageProjectPcaGraphicsPlotPage::connectMenuSignals()
{
  connect(mMenuContext, &AnalysisModelSettingsContext::OpenDataTable, this, &ViewPageProjectPcaGraphicsPlotPage::openDataTable);
  connect(mMenuContext, &AnalysisModelSettingsContext::OpenModelParameters, this, &ViewPageProjectPcaGraphicsPlotPage::openModelParameters);
  connect(mMenuContext, &AnalysisModelSettingsContext::Export, this, &ViewPageProjectPcaGraphicsPlotPage::exportPage);
}

void ViewPageProjectPcaGraphicsPlotPage::disconnectMenuSignals()
{
  disconnect(mMenuContext, &AnalysisModelSettingsContext::OpenDataTable, this, &ViewPageProjectPcaGraphicsPlotPage::openDataTable);
  disconnect(mMenuContext, &AnalysisModelSettingsContext::OpenModelParameters, this, &ViewPageProjectPcaGraphicsPlotPage::openModelParameters);
  disconnect(mMenuContext, &AnalysisModelSettingsContext::Export, this, &ViewPageProjectPcaGraphicsPlotPage::exportPage);
}

void ViewPageProjectPcaGraphicsPlotPage::openDataTable()
{
  removeExcludedSamples();
  removeExcludedPeaks();
  if (auto contextRoot = GenesisContextRoot::Get())
  {
    if (const auto markupContext = contextRoot->GetChildContextStepBasedMarkup())
    {
      const auto tableId = mPcaModel->Get<size_t>(PcaAnalysisIdentityModel::AnalysisTableId);
      markupContext->SetData(ContextTagNames::ReferenceId, static_cast<int>(tableId));
      markupContext->SetData(ContextTagNames::analysis_builded_model, static_cast<int>(AnalysisType::PCA));
      const auto& analysisTable = mPcaModel->Get<AnalysisTable>(PcaAnalysisIdentityModel::AnalysisTable);
      markupContext->SetData(ContextTagNames::checked_peaks, QVariant::fromValue<IntVector>(analysisTable.CheckedValues.MarkersIds));
      markupContext->SetData(ContextTagNames::checked_samples, QVariant::fromValue<IntVector>(analysisTable.CheckedValues.SamplesIds));
    }
    GenesisWindow::Get()->ShowPage(ViewPageNames::ViewPageAnalysisDataTablePageName);
  }
}

void ViewPageProjectPcaGraphicsPlotPage::openModelParameters()
{
  if(!mEntityModel)
    return;
  int excludedSamples = 0;
  int excludedPeaks = 0;

  excludedSamples = mEntityModel->countEntities({{AbstractEntityDataModel::RoleEntityType, TypeSample},
                                                    {RoleExcluded, true}});

  excludedPeaks = mEntityModel->countEntities({{AbstractEntityDataModel::RoleEntityType, TypePeak},
                                                  {RoleExcluded, true}});

  const auto table = mPcaModel->Get<AnalysisTable>(PcaAnalysisIdentityModel::AnalysisTable);
  if (table.CheckedValues.SamplesIds.size() - excludedSamples < MinimumCount
      || table.CheckedValues.MarkersIds.size() - excludedPeaks < MinimumCount)
  {
    Notification::NotifyError(tr("To rebuild a model, at least 3 samples should be selected on the counts plot and at least 3 peaks on the loads plot"), tr("Analysis launch error"));
    return;
  }

  auto dial = new WebDialogAnalysisModelSettings(this);
  dial->AddEnabledOptions(
    {
     { WebDialogAnalysisModelSettings::FilterPKEnabled, mPcaModel->Get<bool>(PcaAnalysisIdentityModel::PC9599) },
     { WebDialogAnalysisModelSettings::Autoscale, mPcaModel->Get<bool>(PcaAnalysisIdentityModel::Autoscale) },
     { WebDialogAnalysisModelSettings::Normalization, mPcaModel->Get<bool>(PcaAnalysisIdentityModel::Normalization) },
     });
  connect(dial, &WebDialogAnalysisModelSettings::OptionsAccepted, this,
          [&](WebDialogAnalysisModelSettings::Options opt)
          {
            const auto pc95_99 = opt.testFlag(WebDialogAnalysisModelSettings::FilterPKEnabled);
            mPcaModel->Set(PcaAnalysisIdentityModel::PC9599, pc95_99);
            mPcaModel->Set(PcaAnalysisIdentityModel::Autoscale, opt.testFlag(WebDialogAnalysisModelSettings::Autoscale));
            mPcaModel->Set(PcaAnalysisIdentityModel::Normalization, opt.testFlag(WebDialogAnalysisModelSettings::Normalization));
            mPcaResponseModel->Set(PcaAnalysisResponseModel::PC9599, pc95_99);
            // if (auto barPlot = findChild<PcaBarsPlotWidget*>())
            // {
            //   barPlot->InvalidateBars();
            // }
            rebuildModel();
          });
  dial->Open();
}

void ViewPageProjectPcaGraphicsPlotPage::exportPage()
{
  ItemSelected tables;
  auto modelDataLabel = tr("Model Data");
  tables.push_back({ modelDataLabel, Qt::Checked });
  auto hotellingLabel = tr("Hotelling criterion table");
  if (mCountPlot->hotellingLoaded())
    tables.push_back({ hotellingLabel, Qt::Checked });

  ItemSelected plots;
  for (const auto& name : mPlots.keys())
    plots.push_back({ name, Qt::Checked });

  auto dialog = new ExportTablesAndPlotsDialog(tables, plots, this);
  connect(dialog, &WebDialog::Accepted, [&, dialog, modelDataLabel, hotellingLabel]
          {
            auto path = QFileDialog::getExistingDirectory(this, tr("Open folder"), QDir::currentPath(),
                                                          QFileDialog::ReadOnly | QFileDialog::ShowDirsOnly);
            path = QDir::toNativeSeparators(path);
            if (path.isEmpty())
              return;

            auto saveToFile = [path = path](QNetworkReply* reply, const QByteArray& data)
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

              if (fileName.isEmpty())
              {
                Notification::NotifyError(tr("File-title header is empty"));
              }
              else
              {
                fileName = path + "/" + fileName + "." + fileExtension;
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
            };

            for (const auto& [name, checked] : dialog->Tables())
            {
              if (checked == Qt::Checked)
              {
                if (name == modelDataLabel)
                {
                  const auto requestData = mPcaResponseModel->Get<QByteArray>(PcaAnalysisIdentityModel::ExportRequestBody);
                  const auto doc = QJsonDocument::fromJson(requestData);
                  auto request = doc.object();
                  request.insert(JsonTagNames::project_id, GenesisContextRoot::Get()->ProjectId());
                  API::REST::ExportTables(request, [saveToFile = saveToFile](QNetworkReply* reply, QByteArray data)
                    {
                      saveToFile(reply, data);
                    },
                    [](QNetworkReply*, QNetworkReply::NetworkError error)
                    {
                      Notification::NotifyError(tr("File export error"), error);
                    });
                }
                else if (name == hotellingLabel)
                {
                  auto requestBody = mCountPlot->hottelingExportRequestBody();
                  if (!requestBody.empty())
                  {
                    requestBody.insert(JsonTagNames::project_id, GenesisContextRoot::Get()->ProjectId());
                    API::REST::ExportTables(requestBody, [saveToFile = saveToFile](QNetworkReply* reply, QByteArray data)
                      {
                        saveToFile(reply, data);
                      },
                      [](QNetworkReply*, QNetworkReply::NetworkError error)
                      {
                        Notification::NotifyError(tr("File export error"), error);
                      });
                  }
                }
              }
            }
            for (const auto& [name, checked] : dialog->Plots())
            {
              if (checked == Qt::Checked)
              {
                auto plot = mPlots[name];
                auto pixmap = plot->toPixmap(1920, 1080);

#ifdef Q_OS_WASM
                QBuffer buffer;
                plot->savePng(&buffer);
                QFileDialog::saveFileContent(buffer.data(), name + ".png");
#else
          pixmap.save(path + "/" + name + ".png");
#endif
              }
            }
          });
  dialog->Open();
}

void ViewPageProjectPcaGraphicsPlotPage::savePcaModel()
{
  Dialogs::Templates::SaveEdit::Settings s;
  s.header = tr("PCA model saving");
  s.titleHeader = tr("Model name");
  s.titlePlaceholder = tr("Enter the name of the new model");
  s.titleTooltipsSet.empty = tr("Model name can't be empty");
  s.titleTooltipsSet.forbidden = tr("There is other model with that name");
  s.titleTooltipsSet.notChecked = tr("Model name is not checked yet");
  s.titleTooltipsSet.valid = tr("Model name is valid");
  s.commentHeader = tr("Comment");
  s.commentPlaceholder = tr("Leave a comment about the model");
  s.okBtnText = tr("Save model");

  auto dialog = new Dialogs::Templates::SaveEdit(this, s);
  connect(dialog, &WebDialog::Accepted, this,
          [&, dialog]()
          {
            auto model = copyModelExceptId();
            updateModelByResponse(model);
            model->Set<std::string>(PcaAnalysisIdentityModel::AnalysisTitle, dialog->getTitle().toStdString());
            model->Set<std::string>(PcaAnalysisIdentityModel::AnalysisComment, dialog->getComment().toStdString());
            saveNewModelToServer(model);
          });
  dialog->Open();
}

void ViewPageProjectPcaGraphicsPlotPage::saveChangesPcaModel()
{
  auto overlayId = GenesisWindow::Get()->ShowOverlay(tr("Saving data..."));
  // QScopeGuard guard([overlayId]() { GenesisWindow::Get()->RemoveOverlay(overlayId); });

  const auto serializer = ServiceLocator::Instance().Resolve<PcaAnalysisIdentityModel, IJsonSerializer>();
  assert(serializer);

  updateModelByResponse(mPcaModel);

  const auto jsonModel = serializer->ToJson(mPcaModel);
  API::REST::SaveAnalysis(jsonModel, [&, this, overlayId](QNetworkReply*, QJsonDocument doc)
    {
      if (const auto jsonResponse = doc.object();
          jsonResponse.value(JsonTagNames::Error).toBool())
      {
        Notification::NotifyError(tr("Error : %1 while saving.").arg(jsonResponse.value(JsonTagNames::Status).toString()));
        emit pcaModelSavingFinished(true);
      }
      else
      {
        Notification::NotifySuccess(tr("Model PCA was updated."));
        emit pcaModelSavingFinished(false);
      }
      GenesisWindow::Get()->RemoveOverlay(overlayId);
    },
    [this, overlayId](QNetworkReply*, QNetworkReply::NetworkError error)
    {
      Notification::NotifyError(tr("Error : %1 while saving.").arg(error));
      emit pcaModelSavingFinished(true);
      GenesisWindow::Get()->RemoveOverlay(overlayId);
    });
}

void ViewPageProjectPcaGraphicsPlotPage::saveNewModelToServer(IdentityModelPtr model)
{
  auto overlayId = GenesisWindow::Get()->ShowOverlay(tr("Saving data..."));
  // QScopeGuard guard([overlayId]() { GenesisWindow::Get()->RemoveOverlay(overlayId); });

  const auto serializer = ServiceLocator::Instance().Resolve<PcaAnalysisIdentityModel, IJsonSerializer>();
  assert(serializer);
  const auto jsonModel = serializer->ToJson(model);
  API::REST::SaveAnalysis(jsonModel,
    [&, this, overlayId](QNetworkReply*, QJsonDocument doc)
    {
      if (const auto jsonResponse = doc.object();
          jsonResponse.value(JsonTagNames::Error).toBool())
      {
        Notification::NotifyError(tr("Error : %1 while saving.").arg(jsonResponse.value(JsonTagNames::Status).toString()));
        emit pcaModelSavingFinished(true);
      }
      else
      {
        const auto analysis_id = static_cast<size_t>(jsonResponse.value(JsonTagNames::analysis_id).toInt());
        mPcaModel->Set<size_t>(PcaAnalysisIdentityModel::AnalysisId, analysis_id);
        updateModelMenu();
        emit pcaModelSavingFinished(false);
        Notification::NotifySuccess(tr("Model PCA was saved."));
      }
      GenesisWindow::Get()->RemoveOverlay(overlayId);
    },
    [this, overlayId](QNetworkReply*, QNetworkReply::NetworkError error)
    {
      // qDebug() << error;
      Notification::NotifyError(tr("Error : %1 while saving.").arg(error));
      emit pcaModelSavingFinished(true);
      GenesisWindow::Get()->RemoveOverlay(overlayId);
    });
}

void ViewPageProjectPcaGraphicsPlotPage::LogOut()
{
  const auto model = getIdentityModel();
  const auto isExist = static_cast<int>(model->Get<size_t>(AnalysisIdentityModel::AnalysisId));
  auto dial = Dialogs::Templates::Confirm::confirm(GenesisWindow::Get(), !isExist
                            ? tr("Save current model?")
                            : tr("Save changes in current model?"),

    [this, isExist]()
    {
      connect(this, &ViewPageProjectPcaGraphicsPlotPage::pcaModelSavingFinished, this,
        [this](bool error)
        {
          if(!error)
            View::LogOut();
          else
            View::RejectLoggingOut();
        }, Qt::SingleShotConnection);
      if (isExist)
        saveChangesPcaModel();
      else
        savePcaModel();
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

void ViewPageProjectPcaGraphicsPlotPage::SwitchToPage(const QString &pageId)
{
  const auto model = getIdentityModel();
  const auto isExist = static_cast<int>(model->Get<size_t>(AnalysisIdentityModel::AnalysisId));
  auto dial = Dialogs::Templates::Confirm::confirm(GenesisWindow::Get(), !isExist
                            ? tr("Save current model?")
                            : tr("Save changes in current model?"),

    [this, isExist, pageId]()
    {
      connect(this, &ViewPageProjectPcaGraphicsPlotPage::pcaModelSavingFinished, this,
        [this, pageId](bool error)
        {
          if(!error)
            View::SwitchToPage(pageId);
          else
            View::RejectPageSwitching();
        }, Qt::SingleShotConnection);
      if (isExist)
        saveChangesPcaModel();
      else
        savePcaModel();
      View::SwitchToPage(pageId);
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

void ViewPageProjectPcaGraphicsPlotPage::SwitchToModule(int module)
{
  const auto model = getIdentityModel();
  const auto isExist = static_cast<int>(model->Get<size_t>(AnalysisIdentityModel::AnalysisId));
  auto dial = Dialogs::Templates::Confirm::confirm(GenesisWindow::Get(), !isExist
                                ? tr("Save current model?")
                                : tr("Save changes in current model?"),

      [this, isExist, module]()
      {
        connect(this, &ViewPageProjectPcaGraphicsPlotPage::pcaModelSavingFinished, this,
            [this, module](bool error)
            {
              if(!error)
                View::SwitchToModule(module);
              else
                View::RejectModuleSwitching();
            }, Qt::SingleShotConnection);
        if (isExist)
          saveChangesPcaModel();
        else
          savePcaModel();
        View::SwitchToModule(module);
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

void ViewPageProjectPcaGraphicsPlotPage::setEntityModel(QPointer<AnalysisEntityModel> model)
{
  if(mEntityModel)
  {
    disconnect(mEntityModel, nullptr, this, nullptr);
    mEntityModel->deleteLater();
  }
  mEntityModel = model;
  if(mEntityModel)
  {
    connect(mEntityModel, &AbstractEntityDataModel::entityChanged, this, &ViewPageProjectPcaGraphicsPlotPage::onEntityChanged);
    connect(mEntityModel, &AbstractEntityDataModel::modelReset, this, &ViewPageProjectPcaGraphicsPlotPage::onModelReset);
  }
}

void ViewPageProjectPcaGraphicsPlotPage::createPcaModel()
{
  const auto context = GenesisContextRoot::Get();
  if (!context)
    return;
  size_t tableId = 0;
  size_t typeId = 0;
  size_t analysisId = 0;
  std::string title;
  std::string comment;
  Parameters params;
  if (mIsRebuildModel && mPcaModel)
  {
    tableId = mPcaModel->Get<size_t>(PcaAnalysisIdentityModel::AnalysisTableId);
    typeId = mPcaModel->Get<size_t>(PcaAnalysisIdentityModel::AnalysisTypeId);
    analysisId = mPcaModel->Get<size_t>(PcaAnalysisIdentityModel::AnalysisId);
    params = mPcaModel->Get<Parameters>(PcaAnalysisIdentityModel::AnalysisParameters);
    title = mPcaModel->Get<std::string>(PcaAnalysisIdentityModel::AnalysisTitle);
    comment = mPcaModel->Get<std::string>(PcaAnalysisIdentityModel::AnalysisComment);
  }
  else
  {
    if (const auto pcaContext = context->GetContextMarkupVersionAnalisysPCA())
    {
      const auto analysis_id = pcaContext->GetData(JsonTagNames::analysis_id);
      if (!analysis_id.isNull() && analysis_id.isValid())
        analysisId = static_cast<size_t>(analysis_id.toInt());
      const auto type_id = pcaContext->GetData(JsonTagNames::AnalysisType);
      if (!type_id.isNull() && type_id.isValid())
        typeId = static_cast<size_t>(type_id.toInt());
      const auto table_id = pcaContext->GetData(JsonTagNames::TableId);
      if (!table_id.isNull() && table_id.isValid())
        tableId = static_cast<size_t>(table_id.toInt());
      const auto param = pcaContext->GetData(JsonTagNames::ValueType);
      if (!param.isNull() && param.isValid())
        params = { param.toString().toStdString() };
    }
  }
  mPcaModel = std::make_shared<PcaAnalysisIdentityModel>();
  mPcaModel->Set(PcaAnalysisIdentityModel::AnalysisTitle, title);
  mPcaModel->Set(PcaAnalysisIdentityModel::AnalysisComment, comment);
  mPcaModel->Set(PcaAnalysisIdentityModel::AnalysisTableId, tableId);
  mPcaModel->Set(PcaAnalysisIdentityModel::AnalysisTypeId, typeId);
  mPcaModel->Set(PcaAnalysisIdentityModel::AnalysisId, analysisId);
  mPcaModel->Set(PcaAnalysisIdentityModel::AnalysisParameters, params);
  mPcaModel->Set(PcaAnalysisIdentityModel::ExportRequestBody, mPcaResponseModel->Get<QByteArray>(PcaAnalysisIdentityModel::ExportRequestBody));
  mPcaModel->Set<bool>(PcaAnalysisIdentityModel::Normalization, mPcaResponseModel->Get<bool>(PcaAnalysisResponseModel::NormData));
  mPcaModel->Set<bool>(PcaAnalysisIdentityModel::Autoscale, mPcaResponseModel->Get<bool>(PcaAnalysisResponseModel::Autoscale));
  mPcaModel->Set<bool>(PcaAnalysisIdentityModel::PC9599, mPcaResponseModel->Get<bool>(PcaAnalysisResponseModel::PC9599));
  mPcaModel->Set<QStringList>(PcaAnalysisIdentityModel::PassportKeys, mPcaResponseModel->Get<QStringList>(PcaAnalysisResponseModel::PassportKeys));
  mPcaModel->Set<QStringList>(PcaAnalysisIdentityModel::PassportHeaders, mPcaResponseModel->Get<QStringList>(PcaAnalysisResponseModel::PassportHeaders));
  mPcaModel->Set<AnalysisTable>(PcaAnalysisIdentityModel::AnalysisTable, mPcaResponseModel->Get<AnalysisTable>(PcaAnalysisResponseModel::AnalysisTable));
  mPcaModel->Set<AnalysisData>(PcaAnalysisIdentityModel::AnalysisData, createAnalysisData());
}

IdentityModelPtr ViewPageProjectPcaGraphicsPlotPage::convertPcaModelToResponse(Model::IdentityModelPtr model)
{
  auto response = std::make_shared<PcaAnalysisResponseModel>();
  model->Exec<AnalysisData>(PcaAnalysisIdentityModel::AnalysisData,
                            [&](const AnalysisData& data)
                            {
                              response->Set<AnalysisMetaInfoList>(PcaAnalysisResponseModel::T_Title, data.Data.T_Title);
                              response->Set<AnalysisComponents>(PcaAnalysisResponseModel::Components, data.Data.Components);
                              response->Set<IntVector>(PcaAnalysisResponseModel::P_Ids, data.Data.P_Id);
                              response->Set<StringList>(PcaAnalysisResponseModel::P_Title, data.Data.P_Title);
                            });

  model->Exec<QJsonObject>(PcaAnalysisIdentityModel::Hotteling,
                           [&](const QJsonObject& obj) { response->Set(PcaAnalysisResponseModel::Hotteling, obj); });
  response->Set(PcaAnalysisResponseModel::PC9599, model->Get<bool>(PcaAnalysisIdentityModel::PC9599));
  response->Set(PcaAnalysisIdentityModel::ExportRequestBody, model->Get<QByteArray>(PcaAnalysisIdentityModel::ExportRequestBody));
  response->Set(PcaAnalysisResponseModel::Autoscale, model->Get<bool>(PcaAnalysisIdentityModel::Autoscale));
  response->Set(PcaAnalysisResponseModel::NormData, model->Get<bool>(PcaAnalysisIdentityModel::Normalization));
  return response;
}

QPointer<AnalysisEntityModel> ViewPageProjectPcaGraphicsPlotPage::extractEntitiesFromPcaModel(Model::IdentityModelPtr model)
{
  QPointer<AnalysisEntityModel> entityModel(nullptr);
  model->Exec<QPointer<AnalysisEntityModel>>(PcaAnalysisIdentityModel::EntityModel,
                           [&entityModel](QPointer<AnalysisEntityModel> model)
                           {
                             entityModel = model;
                           });
  if(entityModel)
  {
    //update passport keys and passport headers from actual source
    QStringList pk;
    QStringList ph;
    model->Exec<QStringList>(PcaAnalysisIdentityModel::PassportKeys,
                             [&pk](QStringList list)
                             {
                               pk = list;
                             });
    model->Exec<QStringList>(PcaAnalysisIdentityModel::PassportHeaders,
                             [&ph](QStringList list)
                             {
                               ph = list;
                             });
    entityModel->setModelData(ModelRolePassportKeys, pk);
    entityModel->setModelData(ModelRolePassportHeaders, ph);
    return entityModel;
  }
  entityModel = new AnalysisEntityModel;
  {
    QStringList pk;
    QStringList ph;
    model->Exec<QStringList>(PcaAnalysisIdentityModel::PassportKeys,
                             [&pk](QStringList list)
                             {
                               pk = list;
                             });
    model->Exec<QStringList>(PcaAnalysisIdentityModel::PassportHeaders,
                             [&ph](QStringList list)
                             {
                               ph = list;
                             });
    entityModel->setModelData(ModelRolePassportKeys, pk);
    entityModel->setModelData(ModelRolePassportHeaders, ph);
  }

  TPassportFilter defaultColorFilter;//by layer
  defaultColorFilter[PassportTags::layer] = "grouping by";//exact value can be any non-empty
  TPassportFilter defaultShapeFilter;//no filter

  QList<TLegendGroup> groupsInfo;
  int currentShapeInd = 0;
  int currentColorInd = 0;
  auto ensureGroupEnlisted = [&defaultColorFilter, &defaultShapeFilter, &groupsInfo, &currentShapeInd, &currentColorInd](const TPassport& passport)
  {
    auto title = AnalysisEntityModel::genericGroupName(defaultColorFilter, defaultShapeFilter, passport);
    auto iter = std::find_if(groupsInfo.begin(), groupsInfo.end(), [title](const TLegendGroup& groupInfo)->bool
                             {
                               return groupInfo.title == title;
                             });
    if(iter == groupsInfo.end())
    {
      TPassportFilter filter;
      filter[PassportTags::layer] = passport.value(PassportTags::layer);
      groupsInfo << TLegendGroup
      {
        title, title,
        GPShapeItem::GetCycledColor(currentColorInd++),
        { GPShape::GetPath(GPShapeItem::GetCycledShape(currentShapeInd++)), 10 },
        filter
      };
    }
  };

  QVector<QHash<int, double>> peaksComponentsArrays;
  QVector<QHash<int, double>> samplesComponentsArrays;
  QHash<int, double> explPCsVariance;
  AnalysisInnerData data = model->Get<AnalysisData>(PcaAnalysisIdentityModel::AnalysisData).Data;
  peaksComponentsArrays.resize(data.P_Title.size(), {});
  samplesComponentsArrays.resize(data.T_Title.size(), {});

  for(int i = 0; i < data.Components.size(); i++)
  {
    auto comp = data.Components[i];
    explPCsVariance[comp.Number] = comp.ExplPCsVariance;
    for(int j = 0; j < comp.P.size(); j++)
      peaksComponentsArrays[j][comp.Number] = comp.P[j];
    for(int j = 0; j < comp.T.size(); j++)
      samplesComponentsArrays[j][comp.Number] = comp.T[j];
  }
  for(int i = 0; i < data.P_Title.size(); i++)
  {
    auto peakName = data.P_Title[i];
    DataModel entityData;
    TPeakData peakData;
    peakData.title = QString::fromStdString(peakName);
    peakData.orderNum = i;
    entityData[RolePeakData] = QVariant::fromValue(peakData);
    entityData[RoleTitle] = peakData.title;
    entityData[RoleGroupTooltip] = peakData.title;
    entityData[RoleIntId] = data.P_Id[i];
    entityData[RoleExcluded] = false;
    entityData[RoleOrderNum] = i;
    TComponentsData components = peaksComponentsArrays[i];
    entityData[RoleComponentsData] = QVariant::fromValue(components);
    entityData[RoleShape] = QVariant::fromValue(GPShape::GetPath(GPShape::Circle));
    entityData[RoleShapePixelSize] = 8;
    entityData[RoleColor] = GPShapeItem::ToColor(GPShapeItem::Blue);
    entityModel->addNewEntity(TypePeak, entityData);
  }
  for(int i = 0; i < data.T_Title_expanded.size(); i++)
  {
    auto t_title  = data.T_Title_expanded[i];
    TPassport passport = t_title;
    auto title = AnalysisEntityModel::passportToSampleTitle(passport);
    DataModel entityData;
    entityData[RolePassport] = QVariant::fromValue(passport);
    entityData[RoleTitle] = title;
    entityData[RoleGroupTooltip] = title;
    entityData[RoleIntId] = t_title["sample_id"].toInt();
    entityData[RoleExcluded] = false;
    entityData[RoleOrderNum] = i;
    TComponentsData components = samplesComponentsArrays[i];
    entityData[RoleComponentsData] = QVariant::fromValue(components);
    entityData[RoleShape] = QVariant::fromValue(GPShape::GetPath(GPShape::Circle));
    entityData[RoleShapePixelSize] = 8;
    entityData[RoleColor] = GPShapeItem::ToColor(GPShapeItem::Blue);
    ensureGroupEnlisted(passport);
    entityModel->addNewEntity(TypeSample, entityData);
  }
  entityModel->setExplVarianceMap(explPCsVariance);
  entityModel->setCurrentXYComponent(0,1);
  entityModel->setParent(mCountPlot);
  entityModel->regroup(defaultColorFilter, defaultShapeFilter, groupsInfo);
  return entityModel;
}

AnalysisData ViewPageProjectPcaGraphicsPlotPage::createAnalysisData() const
{
  return
    {
      createAnalysisInnerData(),
    };
}

AnalysisInnerData ViewPageProjectPcaGraphicsPlotPage::createAnalysisInnerData() const
{
  return
    {
     mPcaResponseModel->Get<AnalysisMetaInfoList>(PcaAnalysisResponseModel::T_Title),
     {}, // no test data
     mPcaResponseModel->Get<AnalysisMetaInfoMapsList>(PcaAnalysisResponseModel::T_Title),
     mPcaResponseModel->Get<AnalysisComponents>(PcaAnalysisResponseModel::Components),
     mPcaResponseModel->Get<IntVector>(PcaAnalysisResponseModel::P_Ids),
     mPcaResponseModel->Get<StringList>(PcaAnalysisResponseModel::P_Title),
     };
}

IntVector ViewPageProjectPcaGraphicsPlotPage::getCheckedSamples() const
{
  if(!mEntityModel)
    return Structures::IntVector();
  auto samples = mEntityModel->getEntities({{AbstractEntityDataModel::RoleEntityType, TypeSample},
                                               {RoleExcluded, false}});
  //just for some case, probably not necessary
  std::sort(samples.begin(), samples.end(),
            [](AbstractEntityDataModel::ConstDataPtr op1, AbstractEntityDataModel::ConstDataPtr op2)->bool
            {
              return op1->getData(RoleOrderNum).toInt()
                     < op2->getData(RoleOrderNum).toInt();
            });
  IntVector checkedSamplesIds;
  for(auto& sample : samples)
    checkedSamplesIds << sample->getData(RoleIntId).toInt();

  return checkedSamplesIds;
}

IntVector ViewPageProjectPcaGraphicsPlotPage::getCheckedPeaks() const
{
  if(!mEntityModel)
    return Structures::IntVector();
  auto peaks = mEntityModel->getEntities({{AbstractEntityDataModel::RoleEntityType, TypePeak},
                                               {RoleExcluded, false}});
  //just for some case, probably not necessary
  std::sort(peaks.begin(), peaks.end(),
            [](AbstractEntityDataModel::ConstDataPtr op1, AbstractEntityDataModel::ConstDataPtr op2)->bool
            {
              return op1->getData(RoleOrderNum).toInt()
                     < op2->getData(RoleOrderNum).toInt();
            });

  IntVector checkedPeaksIds;
  for(auto& peak : peaks)
    checkedPeaksIds << peak->getData(RoleIntId).toInt();

  return checkedPeaksIds;
}

void ViewPageProjectPcaGraphicsPlotPage::removeExcludedSamples()
{
  mPcaModel->Exec<AnalysisTable>(PcaAnalysisIdentityModel::AnalysisTable,
                                [&](AnalysisTable& table) { table.CheckedValues.SamplesIds = getCheckedSamples(); });
}

void ViewPageProjectPcaGraphicsPlotPage::removeExcludedPeaks()
{
  mPcaModel->Exec<AnalysisTable>(PcaAnalysisIdentityModel::AnalysisTable,
                                [&](AnalysisTable& table) { table.CheckedValues.MarkersIds = getCheckedPeaks(); });
}

void ViewPageProjectPcaGraphicsPlotPage::onModelReset()
{
  if(!mEntityModel || mCountPlot->hottelingManualGroupingIsActive())
  {
    mRecalcButton->setVisible(false);
    return;
  }
  auto excludedEntitiesCount = mEntityModel->countEntities({{RoleExcluded, true}});
  mRecalcButton->setVisible(excludedEntitiesCount > 0);
}

void ViewPageProjectPcaGraphicsPlotPage::rebuildModel()
{
  const auto model = std::make_shared<PcaAnalysisRequestModel>();

  mPcaModel->Exec<bool>(PcaAnalysisIdentityModel::Autoscale,
                       [&](bool autoscale) { model->Set<bool>(PcaAnalysisRequestModel::Autoscale, autoscale); });

  mPcaModel->Exec<bool>(PcaAnalysisIdentityModel::Normalization,
                       [&](bool normdata) { model->Set<bool>(PcaAnalysisRequestModel::Normdata, normdata); });

  mPcaModel->Exec<Parameters>(PcaAnalysisIdentityModel::AnalysisParameters,
                             [&](const Parameters& params) { model->Set<std::string>(PcaAnalysisRequestModel::Key, params.ValueType); });
  mPcaModel->Exec<size_t>(PcaAnalysisIdentityModel::AnalysisTableId,
                         [&](size_t tableId)
                         {
                           model->Set(PcaAnalysisRequestModel::TableId, static_cast<int>(tableId));
                         });
  model->Set(PcaAnalysisRequestModel::SampleIds, getCheckedSamples());
  model->Set<IntVector>(PcaAnalysisRequestModel::MarkerIds, getCheckedPeaks());
  const auto jsonRequestSerializer = ServiceLocator::Instance().Resolve<PcaAnalysisRequestModel, IJsonSerializer>();
  mIsRebuildModel = true;
  getAnalysis(jsonRequestSerializer->ToJson(model));
}
}
