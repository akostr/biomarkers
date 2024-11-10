#include "view_page_project_mcr_graphics_plot_page.h"

#include <qlabel.h>
#include <QRegExp>

#include "genesis_style/style.h"
#include "logic/tree_model.h"
#include "logic/known_context_names.h"
#include "logic/known_context_tag_names.h"
#include "logic/known_json_tag_names.h"
#include "logic/enums.h"
#include "ui/known_view_names.h"
#include "logic/service/ijson_serializer.h"
#include "logic/service/service_locator.h"
#include "widgets/tab_widget.h"
#include "logic/notification.h"
#include "widgets/model_settings_widget.h"
#include "api/api_rest.h"
#include "logic/context_root.h"
#include "logic/models/mcr_analysis_request_model.h"
#include "logic/models/mcr_analysis_response_model.h"
#include "logic/models/mcr_analysis_identity_model.h"
#include "widgets/mcr_plot_widget.h"
#include "widgets/table_view_widget.h"
#include "item_models/mcr_table_model.h"
#include "genesis_window.h"
#include "ui/dialogs/templates/save_edit.h"
#include "ui/dialogs/export_tables_and_plots_dialog.h"
#include "ui/dialogs/web_dialog_mcr_build_settings.h"
#include "ui/widgets/mcr_origin_restored_plot.h"
#include "ui/dialogs/templates/confirm.h"
#include <graphicsplot/graphicsplot_extended.h>

using namespace Dialogs;
using namespace Widgets;
using namespace Names;
using namespace Service;
using namespace Widgets;
using namespace Core;
using namespace Models;
using namespace Constants;

namespace Views
{
  ViewPageProjectMCRGraphicsPlotPage::ViewPageProjectMCRGraphicsPlotPage(QWidget* parent)
    : View(parent)
  {
    SetupUi();
    ConnectSignals();
    ConnectMenuSignals();
  }

  IdentityModelPtr ViewPageProjectMCRGraphicsPlotPage::GetMCRModel() const
  {
    return McrIdentityModel;
  }

  void ViewPageProjectMCRGraphicsPlotPage::ApplyContextMarkupVersionAnalysisMCR(const QString& dataId, const QVariant& data)
  {
    //TODO make this partial updatable
    bool isReset = true;//isDataReset(dataId, data);
    auto context = GenesisContextRoot::Get()->GetContextMarkupVersionAnalisysMCR();
    if (isReset)
    {
      if (!context)
        return;

      const auto analysisId = static_cast<size_t>(context->GetData(ContextTagNames::AnalysisId).toInt());
      TableId = context->GetData(JsonTagNames::TableId).toInt();
      if (analysisId == 0)
      {
        const auto response = context->GetData(ContextTagNames::McrResponseJson).toJsonObject();
        ParseResponseJson(response);
      }
      else
      {
        API::REST::GetAnalysis(static_cast<int>(analysisId),
          [&](QNetworkReply*, QJsonDocument doc)
          {
            if (const auto object = doc.object();
              object.value(JsonTagNames::Error).toBool())
            {
              Notification::NotifyError(tr("Failed to receive analysis by id."));
            }
            else
            {
              const auto modelSerializer = ServiceLocator::Instance().Resolve<McrAnalysisIdentityModel, IJsonSerializer>();
              McrIdentityModel = modelSerializer->ToModel(object["data"].toObject());
              emit MCRResponseModelLoaded();
            }
          },
          [](QNetworkReply*, QNetworkReply::NetworkError error)
          {
            Notification::NotifyError(tr("Failed to receive analysis by id : %1").arg(error));
          });
      }
    }
  }

  void ViewPageProjectMCRGraphicsPlotPage::SetupUi()
  {
    //// Self
    setStyleSheet(Style::Genesis::GetUiStyle());

    //// Layout
    const auto layout = new QVBoxLayout(this);

    layout->setContentsMargins(Style::Scale(40), Style::Scale(32), Style::Scale(40), Style::Scale(32));
    layout->setSpacing(0);

    auto toolBarLayout = new QHBoxLayout;
    toolBarLayout->setContentsMargins(0, 0, 0, 0);
    auto caption = new QLabel(tr("Current MCR"));
    caption->setStyleSheet(Style::Genesis::GetH1());
    toolBarLayout->addWidget(caption);

    toolBarLayout->addStretch();

    RecalcButton = new QPushButton(tr("Recalculate model"));
    RecalcButton->setVisible(false);
    connect(RecalcButton, &QPushButton::clicked, this, &ViewPageProjectMCRGraphicsPlotPage::OpenModelParameters);
    RecalcButton->setProperty("secondary", true);
    toolBarLayout->addWidget(RecalcButton);
    toolBarLayout->addWidget(new ModelSettingsWidget(this));

    layout->addLayout(toolBarLayout);

    auto tabContainer = new QWidget(this);
    tabContainer->setProperty("style", "white_base");
    tabContainer->setLayout(new QHBoxLayout());
    tabContainer->layout()->setContentsMargins(26, 26, 26, 26);
    layout->addWidget(tabContainer);

    mPlotTabWidget = new TabWidget(tabContainer);
    auto plotWidget = new McrPlotWidget(this);
    connect(plotWidget, &McrPlotWidget::UpdateRecalcButtonVisible, this,
      &ViewPageProjectMCRGraphicsPlotPage::UpdateRecalcButton);

    mPlotTabWidget->AddTabWidget(plotWidget, tr("Plots"));
    tabContainer->layout()->addWidget(mPlotTabWidget);
    const auto mcrTable = new TableViewWidget(this);
    mPlotTabWidget->AddTabWidget(mcrTable, tr("Relation of samples to references"));
  }

  void ViewPageProjectMCRGraphicsPlotPage::ConnectSignals()
  {
    connect(this, &ViewPageProjectMCRGraphicsPlotPage::MCRResponseModelLoaded,
      this, &ViewPageProjectMCRGraphicsPlotPage::LoadGraphicsFromResponseModel);
    connect(this, &ViewPageProjectMCRGraphicsPlotPage::MCRModelSavingFinished, this, []() {TreeModel::ResetInstances("TreeModelDynamicProjectModelsList"); });
  }

  void ViewPageProjectMCRGraphicsPlotPage::ConnectMenuSignals()
  {
    connect(MenuContext, &AnalysisModelSettingsContext::OpenDataTable, this, &ViewPageProjectMCRGraphicsPlotPage::OpenDataTable);
    connect(MenuContext, &AnalysisModelSettingsContext::OpenModelParameters, this, &ViewPageProjectMCRGraphicsPlotPage::OpenModelParameters);
    connect(MenuContext, &AnalysisModelSettingsContext::OpenMarkup, this, &ViewPageProjectMCRGraphicsPlotPage::OpenMarkup);
    connect(MenuContext, &AnalysisModelSettingsContext::Export, this, &ViewPageProjectMCRGraphicsPlotPage::Export);
  }

  void ViewPageProjectMCRGraphicsPlotPage::DisconnectSignals()
  {
    disconnect(MenuContext, &AnalysisModelSettingsContext::OpenDataTable, this, &ViewPageProjectMCRGraphicsPlotPage::OpenDataTable);
    disconnect(MenuContext, &AnalysisModelSettingsContext::OpenModelParameters, this, &ViewPageProjectMCRGraphicsPlotPage::OpenModelParameters);
    disconnect(MenuContext, &AnalysisModelSettingsContext::OpenMarkup, this, &ViewPageProjectMCRGraphicsPlotPage::OpenMarkup);
    disconnect(MenuContext, &AnalysisModelSettingsContext::Export, this, &ViewPageProjectMCRGraphicsPlotPage::Export);
  }

  void ViewPageProjectMCRGraphicsPlotPage::UpdateModelMenu()
  {
    if (const auto modelMenuButton = findChild<ModelSettingsWidget*>())
    {
      if (MenuContext)
        DisconnectSignals();

      if (McrIdentityModel)
      {
        const auto id = McrIdentityModel->Get<size_t>(McrAnalysisIdentityModel::AnalysisId);
        if (id > 0)
        {
          const auto existModelContext = new ExistModelSettingsContext(modelMenuButton);
          connect(existModelContext, &ExistModelSettingsContext::SaveAsNew, this, &ViewPageProjectMCRGraphicsPlotPage::SaveMcrModel);
          connect(existModelContext, &ExistModelSettingsContext::SaveChanges, this, &ViewPageProjectMCRGraphicsPlotPage::SaveChangesMcrModel);
          MenuContext = existModelContext;
        }
        else
        {
          const auto newModelContext = new NewModelSettingsContext(modelMenuButton);
          connect(newModelContext, &NewModelSettingsContext::SaveNewModel, this, &ViewPageProjectMCRGraphicsPlotPage::SaveMcrModel);
          MenuContext = newModelContext;
        }
      }
      ConnectMenuSignals();
      modelMenuButton->SetMenu(MenuContext);
    }
  }

  void ViewPageProjectMCRGraphicsPlotPage::UpdateRecalcButton()
  {
    if (auto plotWidget = findChild<McrPlotWidget*>())
    {
      RecalcButton->setVisible(plotWidget->IsPeaksExcluded() || plotWidget->IsSamplesExcluded());
    }
  }

  void ViewPageProjectMCRGraphicsPlotPage::OpenDataTable()
  {
    if (auto contextRoot = GenesisContextRoot::Get())
    {
      if (const auto markupContext = contextRoot->GetChildContextStepBasedMarkup())
      {
        const auto tableId = McrIdentityModel->Get<size_t>(AnalysisIdentityModel::AnalysisTableId);
        markupContext->SetData(ContextTagNames::ReferenceId, static_cast<int>(tableId));
        markupContext->SetData(ContextTagNames::analysis_builded_model, static_cast<int>(AnalysisType::MCR));
        const auto& analysisTable = McrIdentityModel->Get<AnalysisTable>(AnalysisIdentityModel::AnalysisTable);
        markupContext->SetData(ContextTagNames::checked_peaks, QVariant::fromValue<IntVector>(analysisTable.CheckedValues.MarkersIds));
        markupContext->SetData(ContextTagNames::checked_samples, QVariant::fromValue<IntVector>(analysisTable.CheckedValues.SamplesIds));
      }
      GenesisWindow::Get()->ShowPage(ViewPageNames::ViewPageAnalysisDataTablePageName);
    }
  }

  void ViewPageProjectMCRGraphicsPlotPage::OpenModelParameters()
  {
    if (auto plotWidget = findChild<McrPlotWidget*>())
    {
      if (!plotWidget->IsReadyForBuildModel())
      {
        Notification::NotifyError(tr("To rebuild a model, at least 3 samples should be selected on the difference plot and at least 3 variables on the the original and restored plot"), tr("Analysis launch error"));
        return;
      }
    }
    auto dialog = new WebDialogMCRBuildSettings(this);

    IsRebuildModel = true;

    McrIdentityModel->Exec<size_t>(McrAnalysisIdentityModel::AnalysisTableId,
      [&](size_t table_id) {
        TableId = static_cast<int>(table_id);
        dialog->SetTableId(static_cast<int>(table_id));
      });

    McrIdentityModel->Exec<AnalysisTable>(McrAnalysisIdentityModel::AnalysisTable,
      [&](const AnalysisTable& table)
      {
        dialog->SetSampleIds(table.CheckedValues.SamplesIds);
        dialog->SetMarkersIds(table.CheckedValues.MarkersIds);
      });

    McrIdentityModel->Exec<AnalysisData>(McrAnalysisIdentityModel::AnalysisData,
      [&](const AnalysisData& data)
      {
        QStringList names;
        names.reserve(data.Data.T_Title.size());
        std::transform(data.Data.T_Title.begin(), data.Data.T_Title.end(),
          std::back_inserter(names),
          [&](const AnalysisMetaInfo& info) -> QString { return QString::fromStdString(info.FileName); });
        dialog->SetSampleNames(names);
      });
    // set reference name after set sample names
    McrIdentityModel->Exec<MCRParameters>(McrAnalysisIdentityModel::InputParameters,
      [&](const MCRParameters& params)
      {
        dialog->SetComponetCurrentValue(params.NumberComponents);
        dialog->SetEvaluatedValue(params.TolErrChange);
        dialog->SetMethod(params.Method);
        dialog->SetMaxIterations(params.MaxIter);
        dialog->SetReferenceSettings(params.Settings);
        dialog->SetClosedSystem(params.ClosedSystem);
        dialog->SetNormData(params.NormData);
        dialog->SetParameterType(QString::fromStdString(params.Key));
        ParameterKey = params.Key;
      });
    connect(dialog, &WebDialog::Accepted, this,
      [&, dialog]()
      {
        MCRParameters params;
        params.ClosedSystem = dialog->IsClosedSystem();
        params.Key = ParameterKey;
        params.MaxIter = dialog->GetMaxIterations();
        params.Method = dialog->GetMethod();
        params.NormData = dialog->IsNormData();
        params.NumberComponents = dialog->GetComponentCurrentValue();
        params.Settings = dialog->GetReferenceSettings();
        params.TolErrChange = dialog->GetEvaluatedValue();
        auto model = CreateRequestModel(params,
          dialog->GetTableId(),
          dialog->GetSampleIds(),
          dialog->GetMarkerIds()
        );

        ParseResponseFromServer(model);
      });
    dialog->Open();
  }

  void ViewPageProjectMCRGraphicsPlotPage::OpenMarkup()
  {
  }

  void ViewPageProjectMCRGraphicsPlotPage::Export()
  {
    ItemSelected tables;
    auto modelDataLabel = tr("Model Data");
    tables.push_back({ modelDataLabel, Qt::Checked });

    ItemSelected plots;
    QMap<QString, GraphicsPlotExtended*> plotNames;
    for (const auto& plotWidget : mPlotTabWidget->findChildren<PlotWidget*>())
    {
      if (plotWidget->objectName() == "ConcentrationPlot")
      {
        auto name = tr("Concentration profilies");
        if (auto plot = plotWidget->findChild<GraphicsPlotExtended*>())
        {
          plots.push_back({ name, Qt::Checked });
          plotNames[name] = plot;
        }
      }
      else if (plotWidget->objectName() == "DifferencePlot")
      {
        auto name = tr("Difference plot");
        if (auto plot = plotWidget->findChild<GraphicsPlotExtended*>())
        {
          plots.push_back({ name, Qt::Checked });
          plotNames[name] = plot;
        }
      }
      else if (plotWidget->objectName() == "SpectralPlot")
      {
        auto name = tr("Spectral profiles");
        if (auto plot = plotWidget->findChild<GraphicsPlotExtended*>())
        {
          plots.push_back({ name, Qt::Checked });
          plotNames[name] = plot;
        }
      }
    }
    for (const auto& plotWidget : mPlotTabWidget->findChildren<McrOriginRestoredPlot*>())
    {
      if (plotWidget->objectName() == "OriginalRestoredPlot")
      {
        auto name = tr("Original restored");
        if (auto plot = plotWidget->findChild<GraphicsPlotExtended*>())
        {
          plots.push_back({ name, Qt::Checked });
          plotNames[name] = plot;
        }
      }
    }

    auto dialog = new ExportTablesAndPlotsDialog(tables, plots, this);
    connect(dialog, &WebDialog::Accepted, [&, dialog, modelDataLabel, plotNames]
      {
        auto path = QFileDialog::getExistingDirectory(this, tr("Open folder"), QDir::currentPath(),
        QFileDialog::ReadOnly | QFileDialog::ShowDirsOnly);
    path = QDir::toNativeSeparators(path);
    if (path.isEmpty())
      return;

    for (const auto& [name, checked] : dialog->Tables())
    {
      if (checked == Qt::Checked)
      {
        if (name == modelDataLabel)
        {
          const auto requestData = McrIdentityModel->Get<QByteArray>(McrAnalysisIdentityModel::ExportRequestBody);
          const auto doc = QJsonDocument::fromJson(requestData);
          auto request = doc.object();
          request.insert(JsonTagNames::project_id, GenesisContextRoot::Get()->ProjectId());
          API::REST::ExportTables(request,
            [path = path](QNetworkReply* reply, QByteArray data)
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
            },
            [](QNetworkReply*, QNetworkReply::NetworkError error)
            {
              Notification::NotifyError(tr("File export error"), error);
            });
        }
      }
    }
    for (const auto& [name, checked] : dialog->Plots())
    {
      if (checked && plotNames.contains(name))
      {
        auto plot = plotNames[name];
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

  void ViewPageProjectMCRGraphicsPlotPage::SaveMcrModel()
  {
    Dialogs::Templates::SaveEdit::Settings s;
    s.header = tr("MCR model saving");
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
        auto model = CopyModelExceptId();
        model->Set<std::string>(McrAnalysisResponseModel::AnalysisTitle, _S(dialog->getTitle()));
        model->Set<std::string>(McrAnalysisResponseModel::AnalysisComment, _S(dialog->getComment()));
        SaveNewModelToServer(model);
      });
    dialog->Open();
  }

  void ViewPageProjectMCRGraphicsPlotPage::SaveChangesMcrModel()
  {
    auto overlayId = GenesisWindow::Get()->ShowOverlay(tr("Saving data..."));
    QScopeGuard guard([overlayId]() { GenesisWindow::Get()->RemoveOverlay(overlayId); });
    const auto serializer = ServiceLocator::Instance().Resolve<McrAnalysisIdentityModel, IJsonSerializer>();
    assert(serializer);

    const auto json = serializer->ToJson(McrIdentityModel);
    API::REST::SaveAnalysis(json, [&, this](QNetworkReply*, QJsonDocument doc)
      {
        if (const auto jsonResponse = doc.object();
          jsonResponse.value(JsonTagNames::Error).toBool())
        {
          Notification::NotifyError(tr("Error : %1 while saving.").arg(jsonResponse.value(JsonTagNames::Status).toString()));
          emit MCRModelSavingFinished(true);
        }
        else
        {
          UpdateModelMenu();
          Notification::NotifySuccess(tr("Model MCR was saved."));
          emit MCRModelSavingFinished(false);
        }
      },
      [this](QNetworkReply*, QNetworkReply::NetworkError error)
      {
        // qDebug() << error;
        Notification::NotifyError(tr("Error : %1 while saving.").arg(error));
        emit MCRModelSavingFinished(true);
      });
  }

  void ViewPageProjectMCRGraphicsPlotPage::SaveNewModelToServer(IdentityModelPtr mcrModel)
  {
    auto overlayId = GenesisWindow::Get()->ShowOverlay(tr("Saving data..."));
    QScopeGuard guard([overlayId]() { GenesisWindow::Get()->RemoveOverlay(overlayId); });

    const auto serializer = ServiceLocator::Instance().Resolve<McrAnalysisIdentityModel, IJsonSerializer>();
    assert(serializer);

    const auto json = serializer->ToJson(mcrModel);
    API::REST::SaveAnalysis(json, [&, this](QNetworkReply*, QJsonDocument doc)
      {
        if (const auto jsonResponse = doc.object();
          jsonResponse.value(JsonTagNames::Error).toBool())
        {
          Notification::NotifyError(tr("Error : %1 while saving.").arg(jsonResponse.value(JsonTagNames::Status).toString()));
          emit MCRModelSavingFinished(true);
        }
        else
        {
          auto analysis_id = static_cast<size_t>(jsonResponse.value(JsonTagNames::analysis_id).toInt());
          McrIdentityModel->Set<size_t>(McrAnalysisIdentityModel::AnalysisId, analysis_id);
          UpdateModelMenu();
          Notification::NotifySuccess(tr("Model MCR was saved."));
          emit MCRModelSavingFinished(false);
        }
      },
      [this](QNetworkReply*, QNetworkReply::NetworkError error)
      {
        // qDebug() << error;
        Notification::NotifyError(tr("Error : %1 while saving.").arg(error));
        emit MCRModelSavingFinished(true);
      });
  }

  void ViewPageProjectMCRGraphicsPlotPage::LoadGraphicsFromResponseModel()
  {
    const auto widget = findChild<McrPlotWidget*>();
    widget->SetModel(McrIdentityModel);
    const auto table = findChild<TableViewWidget*>();
    auto tableModel = new MCRTableModel();
    auto concentrationTable = McrIdentityModel->Get<MCRConcentrationTable>(McrAnalysisResponseModel::ConcentrationsTable);
    const auto info = McrIdentityModel->Get<AnalysisData>(McrAnalysisResponseModel::AnalysisData);
    tableModel->SetData(concentrationTable, info.Data.T_Title);
    table->SetModel(tableModel);
    table->SetResizeSections(QHeaderView::ResizeToContents);
    table->SetStretchLastSection(true);
    if (const auto orPlot = findChild<McrOriginRestoredPlot*>())
      orPlot->CancelExclude();
    UpdateModelMenu();
  }

  void ViewPageProjectMCRGraphicsPlotPage::ConvertModelFromResponse(IdentityModelPtr mcrModel)
  {
    McrIdentityModel = std::make_shared<McrAnalysisIdentityModel>();
    McrIdentityModel->Set(McrAnalysisIdentityModel::AnalysisTable,
      mcrModel->Get<AnalysisTable>(McrAnalysisIdentityModel::AnalysisTable));
    McrIdentityModel->Set(McrAnalysisIdentityModel::AnalysisTypeId,
      mcrModel->Get<size_t>(McrAnalysisIdentityModel::AnalysisTypeId));
    McrIdentityModel->Set(McrAnalysisIdentityModel::CalculatedParameters,
      mcrModel->Get<MCRCalculatedParameters>(McrAnalysisResponseModel::CalculatedParameters));
    McrIdentityModel->Set(McrAnalysisIdentityModel::AnalysisData, CreateAnalysisData(mcrModel));
    McrIdentityModel->Set(McrAnalysisIdentityModel::Concentrations,
      mcrModel->Get<MCRCalculationList>(McrAnalysisResponseModel::Concentrations));
    McrIdentityModel->Set(McrAnalysisIdentityModel::ConcentrationsTable,
      mcrModel->Get<MCRConcentrationTable>(McrAnalysisResponseModel::ConcentrationsTable));
    McrIdentityModel->Set(McrAnalysisIdentityModel::Spectra,
      mcrModel->Get<MCRCalculationList>(McrAnalysisResponseModel::Spectra));
    McrIdentityModel->Set(McrAnalysisIdentityModel::Difference,
      mcrModel->Get<MCRCalculationList>(McrAnalysisResponseModel::Difference));
    McrIdentityModel->Set(McrAnalysisIdentityModel::Reconstructions,
      mcrModel->Get<MCRCalculationList>(McrAnalysisResponseModel::Reconstructions));
    McrIdentityModel->Set(McrAnalysisIdentityModel::Originals,
      mcrModel->Get<MCRCalculationList>(McrAnalysisResponseModel::Originals));
    McrIdentityModel->Set(McrAnalysisIdentityModel::AnalysisParameters,
      mcrModel->Get<Parameters>(McrAnalysisIdentityModel::AnalysisParameters));
    McrIdentityModel->Set(McrAnalysisIdentityModel::AnalysisTableId, static_cast<size_t>(TableId));
    McrIdentityModel->Set(McrAnalysisIdentityModel::ExportRequestBody,
      mcrModel->Get<QByteArray>(McrAnalysisResponseModel::ExportRequestBody));
    McrIdentityModel->Set(McrAnalysisIdentityModel::Pids,
      mcrModel->Get<IntVector>(McrAnalysisResponseModel::Pids));
    McrIdentityModel->Set(McrAnalysisIdentityModel::InputParameters, mcrModel->Get<MCRParameters>(McrAnalysisResponseModel::InputParameters));
  }

  AnalysisData ViewPageProjectMCRGraphicsPlotPage::CreateAnalysisData(IdentityModelPtr mcrModel)
  {
    return
    {
     {
       {
         mcrModel->Get<AnalysisMetaInfoList>(McrAnalysisResponseModel::TTitle),
       }, // TTitle
     },
     {},
     {},
    };
  }

  IdentityModelPtr ViewPageProjectMCRGraphicsPlotPage::CopyModelExceptId()
  {
    auto model = std::make_shared<McrAnalysisResponseModel>();

    model->Set<size_t>(McrAnalysisResponseModel::AnalysisId, 0);
    McrIdentityModel->Exec<AnalysisData>(McrAnalysisResponseModel::AnalysisData,
      [&](const AnalysisData& data)
      {
        model->Set<AnalysisData>(McrAnalysisResponseModel::AnalysisData, data);
      });

    McrIdentityModel->Exec<size_t>(McrAnalysisResponseModel::AnalysisTypeId,
      [&](const size_t& id)
      {
        model->Set<size_t>(McrAnalysisResponseModel::AnalysisTypeId, id);
      });

    McrIdentityModel->Exec<size_t>(McrAnalysisResponseModel::AnalysisTableId,
      [&](const size_t& id)
      {
        model->Set<size_t>(McrAnalysisResponseModel::AnalysisTableId, id);
      });

    McrIdentityModel->Exec<Parameters>(McrAnalysisResponseModel::AnalysisParameters,
      [&](const Parameters& params)
      {
        model->Set<Parameters>(McrAnalysisResponseModel::AnalysisParameters, params);
      });

    McrIdentityModel->Exec<AnalysisTable>(McrAnalysisResponseModel::AnalysisTable,
      [&](const AnalysisTable& params)
      {
        model->Set<AnalysisTable>(McrAnalysisResponseModel::AnalysisTable, params);
      });

    McrIdentityModel->Exec<QByteArray>(McrAnalysisResponseModel::ExportRequestBody,
      [&](const QByteArray& requestBody)
      {
        model->Set<QByteArray>(McrAnalysisResponseModel::ExportRequestBody, requestBody);
      });

    McrIdentityModel->Exec<MCRConcentrationTable>(McrAnalysisResponseModel::ConcentrationsTable,
      [&](const MCRConcentrationTable& table)
      {
        model->Set<MCRConcentrationTable>(McrAnalysisResponseModel::ConcentrationsTable, table);
      });

    McrIdentityModel->Exec<MCRCalculationList>(McrAnalysisResponseModel::Concentrations,
      [&](const MCRCalculationList& table)
      {
        model->Set<MCRCalculationList>(McrAnalysisResponseModel::Concentrations, table);
      });

    McrIdentityModel->Exec<MCRCalculationList>(McrAnalysisResponseModel::Spectra,
      [&](const MCRCalculationList& table)
      {
        model->Set<MCRCalculationList>(McrAnalysisResponseModel::Spectra, table);
      });

    McrIdentityModel->Exec<MCRCalculationList>(McrAnalysisResponseModel::Difference,
      [&](const MCRCalculationList& table)
      {
        model->Set<MCRCalculationList>(McrAnalysisResponseModel::Difference, table);
      });

    McrIdentityModel->Exec<MCRCalculationList>(McrAnalysisResponseModel::Reconstructions,
      [&](const MCRCalculationList& table)
      {
        model->Set<MCRCalculationList>(McrAnalysisResponseModel::Reconstructions, table);
      });

    McrIdentityModel->Exec<MCRCalculationList>(McrAnalysisResponseModel::Originals,
      [&](const MCRCalculationList& table)
      {
        model->Set<MCRCalculationList>(McrAnalysisResponseModel::Originals, table);
      });

    McrIdentityModel->Exec<MCRCalculatedParameters>(McrAnalysisResponseModel::CalculatedParameters,
      [&](const MCRCalculatedParameters& table)
      {
        model->Set<MCRCalculatedParameters>(McrAnalysisResponseModel::CalculatedParameters, table);
      });

    McrIdentityModel->Exec<IntVector>(McrAnalysisIdentityModel::Pids,
      [&](const IntVector& pids)
      {
        model->Set<IntVector>(McrAnalysisResponseModel::Pids, pids);
      });

    McrIdentityModel->Exec<MCRParameters>(McrAnalysisIdentityModel::InputParameters,
      [&](const MCRParameters& params)
      {
        model->Set(McrAnalysisIdentityModel::InputParameters, params);
      });
    return model;
  }

  IdentityModelPtr ViewPageProjectMCRGraphicsPlotPage::CreateRequestModel(GenesisContextPtr context) const
  {
    MCRParameters params;
    params.Key = ParameterKey;
    params.NumberComponents = static_cast<size_t>(context->GetData(ContextTagNames::ComponentCount).toInt());
    params.TolErrChange = context->GetData(ContextTagNames::ToleranceErrorChange).toDouble();
    params.Method = _S(context->GetData(ContextTagNames::Method).toString());
    params.MaxIter = static_cast<size_t>(context->GetData(ContextTagNames::MaximumIterations).toInt());
    params.Settings = context->GetData(ContextTagNames::ReferenceSettings).value<ReferenceSettingsList>();
    params.NormData = context->GetData(ContextTagNames::IsNomdata).toBool();
    auto tableId = context->GetData(JsonTagNames::TableId).toInt();
    auto samples = context->GetData(JsonTagNames::SamplesIds).value<IntVector>();
    auto markers = context->GetData(JsonTagNames::MarkersIds).value<IntVector>();
    return CreateRequestModel(params, tableId, samples, markers);
  }

  IdentityModelPtr ViewPageProjectMCRGraphicsPlotPage::CreateRequestModel(const MCRParameters& parameters,
    int tableId, const IntVector& sampleIds, const IntVector& markersIds) const
  {
    auto model = std::make_shared<McrAnalysisRequestModel>();
    model->Set(McrAnalysisRequestModel::NumberComponents, parameters.NumberComponents);
    model->Set(McrAnalysisRequestModel::TolErrorChange, parameters.TolErrChange);
    model->Set(McrAnalysisRequestModel::Method, parameters.Method);
    model->Set(McrAnalysisRequestModel::MaxIter, parameters.MaxIter);
    model->Set(McrAnalysisRequestModel::ReferenceSettings, parameters.Settings);
    model->Set(McrAnalysisRequestModel::ClosedSystem, parameters.ClosedSystem);
    model->Set(McrAnalysisRequestModel::Normdata, parameters.NormData);
    model->Set(McrAnalysisRequestModel::SampleIds, sampleIds);
    model->Set(McrAnalysisRequestModel::MarkerIds, markersIds);
    model->Set(McrAnalysisRequestModel::Key, parameters.Key);
    model->Set(McrAnalysisRequestModel::TableId, tableId);
    return model;
  }

  void ViewPageProjectMCRGraphicsPlotPage::ParseResponseFromServer(IdentityModelPtr requestModel)
  {
    const auto serializer = ServiceLocator::Instance().Resolve<McrAnalysisRequestModel, IJsonSerializer>();
    const auto jsonRequest = serializer->ToJson(requestModel);
    GetMcrAnalysis(jsonRequest);
  }

  void ViewPageProjectMCRGraphicsPlotPage::GetMcrAnalysis(const QJsonObject& request)
  {
    API::REST::GetTableAnalysisMCR(request,
      [&](QNetworkReply*, QJsonDocument doc)
      {
        const auto obj = doc.object();
        if (obj.value(JsonTagNames::Error).toBool())
        {
          Notification::NotifyError(
            tr("MCR error: %1")
            .arg(obj.value(JsonTagNames::Msg).toString()));
        }
        else
        {
          ParseResponseJson(obj.value(JsonTagNames::Result).toObject());
        }
      },
      [](QNetworkReply*, QNetworkReply::NetworkError err)
      {
        Notification::NotifyError(tr("Error : %1 while receiving data.").arg(err));
      });
  }

  void ViewPageProjectMCRGraphicsPlotPage::ParseResponseJson(const QJsonObject& response)
  {
    auto modelSerializer = ServiceLocator::Instance().Resolve<McrAnalysisResponseModel, IJsonSerializer>();
    auto responseModel = modelSerializer->ToModel(response);
    responseModel->Set<size_t>(McrAnalysisIdentityModel::AnalysisTypeId, static_cast<size_t>(AnalysisType::MCR));
    const auto analysisId = IsRebuildModel && McrIdentityModel
      ? McrIdentityModel->Get<size_t>(McrAnalysisIdentityModel::AnalysisId)
      : 0;

    const auto title = IsRebuildModel && McrIdentityModel
      ? McrIdentityModel->Get<std::string>(McrAnalysisIdentityModel::AnalysisTitle)
      : "";

    const auto comment = IsRebuildModel && McrIdentityModel
      ? McrIdentityModel->Get<std::string>(McrAnalysisIdentityModel::AnalysisComment)
      : "";

    responseModel->Set<Parameters>(McrAnalysisIdentityModel::AnalysisParameters, { ParameterKey });
    ConvertModelFromResponse(responseModel);
    McrIdentityModel->Set(McrAnalysisIdentityModel::AnalysisId, analysisId);
    McrIdentityModel->Set(McrAnalysisIdentityModel::AnalysisTitle, title);
    McrIdentityModel->Set(McrAnalysisIdentityModel::AnalysisComment, comment);
    IsRebuildModel = false;
    emit MCRResponseModelLoaded();
  }
  void ViewPageProjectMCRGraphicsPlotPage::SwitchToPage(const QString& pageId)
  {
    auto& model = McrIdentityModel;
    const auto isExist = static_cast<int>(model->Get<size_t>(McrAnalysisIdentityModel::AnalysisId)) > 0;
    auto dial = Dialogs::Templates::Confirm::confirm(GenesisWindow::Get(),
      !isExist
      ? tr("Save current model?")
      : tr("Save changes in current model?"),
      [this, isExist, pageId]()
      {
        connect(this, &ViewPageProjectMCRGraphicsPlotPage::MCRModelSavingFinished, this,
        [this, pageId](bool error)
          {
            if (!error)
              View::SwitchToPage(pageId);
            else
              View::RejectPageSwitching();
          }, Qt::SingleShotConnection);

    if (isExist)
      SaveChangesMcrModel();
    else
      SaveMcrModel();
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

  void ViewPageProjectMCRGraphicsPlotPage::SwitchToModule(int module)
  {
    auto& model = McrIdentityModel;
    const auto isExist = static_cast<int>(model->Get<size_t>(McrAnalysisIdentityModel::AnalysisId)) > 0;
    auto dial = Dialogs::Templates::Confirm::confirm(GenesisWindow::Get(),
        !isExist
            ? tr("Save current model?")
            : tr("Save changes in current model?"),
        [this, isExist, module]()
        {
          connect(this, &ViewPageProjectMCRGraphicsPlotPage::MCRModelSavingFinished, this,
              [this, module](bool error)
              {
                if (!error)
                  View::SwitchToModule(module);
                else
                  View::RejectModuleSwitching();
              }, Qt::SingleShotConnection);

          if (isExist)
            SaveChangesMcrModel();
          else
            SaveMcrModel();
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

  void ViewPageProjectMCRGraphicsPlotPage::LogOut()
  {
    auto& model = McrIdentityModel;
    const auto isExist = static_cast<int>(model->Get<size_t>(McrAnalysisIdentityModel::AnalysisId)) > 0;
    auto dial = Dialogs::Templates::Confirm::confirm(GenesisWindow::Get(),
      !isExist
      ? tr("Save current model?")
      : tr("Save changes in current model?"),
      [this, isExist]()
      {
        connect(this, &ViewPageProjectMCRGraphicsPlotPage::MCRModelSavingFinished, this,
        [this](bool error)
          {
            if (!error)
              View::LogOut();
            else
              View::RejectLoggingOut();
          }, Qt::SingleShotConnection);

    if (isExist)
      SaveChangesMcrModel();
    else
      SaveMcrModel();
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
}
