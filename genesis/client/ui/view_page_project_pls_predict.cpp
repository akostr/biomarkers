#include "view_page_project_pls_predict.h"

#include "widgets/table_view_widget.h"
#include "genesis_style/style.h"
#include "logic/tree_model.h"
#include "logic/known_context_names.h"
#include "logic/known_context_tag_names.h"
#include "logic/known_json_tag_names.h"
#include "logic/service/service_locator.h"
#include "logic/service/ijson_serializer.h"
#include "logic/models/pls_predict_response_model.h"
#include "logic/models/pls_predict_analysis_model.h"
#include "logic/models/pls_analysis_identity_model.h"
#include "logic/models/pls_predict_parent_analysis_list_model.h"
#include "ui/item_models/pls_predict_table_model.h"
#include "logic/context_root.h"
#include "api/api_rest.h"
#include "logic/notification.h"
#include "ui/dialogs/templates/save_edit.h"
#include "contexts/new_model_settings_context.h"
#include "contexts/exist_model_settings_context.h"
#include "widgets/model_settings_widget.h"
#include "logic/enums.h"
#include "known_view_names.h"
#include "genesis_window.h"
#include "dialogs/web_dialog_pls_predict_settings.h"
#include "logic/models/pls_fit_request_model.h"

#include <QVBoxLayout>
#include <QLabel>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QJsonDocument>
#include <QPushButton>
#include <QFileDialog>
#include <QRegExp>

using namespace Widgets;
using namespace Service;
using namespace Model;
using namespace Models;
using namespace Dialogs;
using namespace Structures;
using namespace Core;
using namespace Names;

#define _Q(str) QString::fromStdString(str)
#define _S(str) str.toStdString()

namespace Views
{
  using namespace Widgets;

  ViewPageProjectPlsPredict::ViewPageProjectPlsPredict(QWidget* parent)
    :View(parent)
  {
    SetupUi();
    ConnectSignals();
  }

  void ViewPageProjectPlsPredict::ApplyContextMarkupVersionAnalysisPLSPredict(const QString& dataId, const QVariant& data)
  {
    if (isDataReset(dataId, data))
    {
      auto context = GenesisContextRoot::Get()->GetContextMarkupVersionAnalisysPLSPredict();
      const auto predictId = context->GetData(ContextTagNames::AnalysisId);
      if (predictId.isValid() && !predictId.isNull())
      {
        API::REST::GetAnalysis(predictId.toInt(),
          [&](QNetworkReply*, QJsonDocument doc)
          {
            if (const auto object = doc.object();
              object.value(JsonTagNames::Error).toBool())
            {
              Notification::NotifyError(tr("Error receive analysis by id."));
            }
            else
            {
              const auto serializer = ServiceLocator::Instance().Resolve<PlsPredictAnalysisModel, IJsonSerializer>();
              PredictModel = serializer->ToModel(object.value(JsonTagNames::Data).toObject());
              emit PredictModelLoaded();
            }
          },
          [](QNetworkReply*, QNetworkReply::NetworkError error)
          {
            Notification::NotifyError(tr("Error receive analysis by id : %1").arg(error));
          });
      }
      else
      {
        const auto json = context->GetData(GenesisContextNames::MarkupVersionAnalysisPLSPredict).toJsonObject();
        const auto serializer = ServiceLocator::Instance().Resolve<PlsPredictResponseModel, IJsonSerializer>();
        PredictResponseModel = serializer->ToModel(json.value(JsonTagNames::Result).toObject());
        ConvertResponseToModel();
        FillPredictModelFromContext();
        emit PredictModelLoaded();
      }
    }
  }

  void ViewPageProjectPlsPredict::SetupUi()
  {
    //// Self
    setStyleSheet(Style::Genesis::GetUiStyle());

    //// Layout
    const auto layout = new QVBoxLayout(this);

    layout->setContentsMargins(Style::Scale(40), Style::Scale(32), Style::Scale(40), Style::Scale(32));
    layout->setSpacing(Style::Scale(32));

    auto toolBarLayout = new QHBoxLayout;
    toolBarLayout->setContentsMargins(0, 0, 0, 0);
    auto caption = new QLabel(tr("Current PLC predict"));
    caption->setStyleSheet(Style::Genesis::GetH1());
    toolBarLayout->addWidget(caption);

    toolBarLayout->addStretch();
    toolBarLayout->addWidget(new ModelSettingsWidget(this));
    layout->addLayout(toolBarLayout);
    auto whiteBase = new QWidget(this);
    whiteBase->setProperty("style", "white_base");
    layout->addWidget(whiteBase);
    whiteBase->setLayout(new QHBoxLayout());
    whiteBase->setContentsMargins(0, 0, 0, 0);
    whiteBase->layout()->setContentsMargins(26, 26, 26, 26);
    TableView = new TableViewWidget(whiteBase);
    TableView->SetModel(new PlsPredictTableModel());
    whiteBase->layout()->addWidget(TableView);
  }

  void ViewPageProjectPlsPredict::ConnectSignals()
  {
    connect(this, &ViewPageProjectPlsPredict::ParentModelLoaded, this, &ViewPageProjectPlsPredict::RecalcModel);
    connect(this, &ViewPageProjectPlsPredict::PredictModelLoaded, this, &ViewPageProjectPlsPredict::UpdateTableFromPredictModel);
  }

  void ViewPageProjectPlsPredict::ShowSaveDialog()
  {
    Dialogs::Templates::SaveEdit::Settings s;
    s.header = tr("PLS Predict saving");
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
    dialog->Open();
    connect(dialog, &WebDialog::Accepted, this,
      [&, dialog]() { SavePredictAnalysis(dialog->getTitle(), dialog->getComment()); });
  }

  void ViewPageProjectPlsPredict::SavePredictAnalysis(const QString& title, const QString& comment)
  {
    PredictModel->Set<std::string>(PlsAnalysisIdentityModel::AnalysisTitle, _S(title));
    PredictModel->Set<std::string>(PlsAnalysisIdentityModel::AnalysisComment, _S(comment));
    const auto serializer = ServiceLocator::Instance().Resolve<PlsPredictAnalysisModel, IJsonSerializer>();
    assert(serializer);
    const auto json = serializer->ToJson(PredictModel);

    API::REST::SaveAnalysis(json,
      [&](QNetworkReply*, QJsonDocument doc)
      {
        if (const auto jsonResponse = doc.object();
          jsonResponse.value(JsonTagNames::Error).toBool())
        {
          Notification::NotifyError(tr("Error : %1 while saving.").arg(jsonResponse.value(JsonTagNames::Status).toString()));
        }
        else
        {
          const auto analysis_id = static_cast<size_t>(jsonResponse.value(JsonTagNames::analysis_id).toInt());
          PredictModel->Set<size_t>(PlsAnalysisIdentityModel::AnalysisId, analysis_id);
          Notification::NotifySuccess(tr("Model Pls was saved."));
          TreeModel::ResetInstances("TreeModelDynamicProjectModelsList");
        }
      },
      [](QNetworkReply*, QNetworkReply::NetworkError error)
      {
        // qDebug() << error;
        Notification::NotifyError(tr("Error : %1 while saving.").arg(error));
      });
  }

  void ViewPageProjectPlsPredict::FillPredictModelFromContext()
  {
    if (const auto root = GenesisContextRoot::Get())
    {
      if (const auto context = root->GetContextMarkupVersionAnalisysPLSPredict())
      {
        const auto tableId = context->GetData(JsonTagNames::TableId);
        if (tableId.isValid() && !tableId.isNull())
          PredictModel->Set<size_t>(PlsAnalysisIdentityModel::AnalysisTableId, static_cast<size_t>(tableId.toInt()));

        const auto analysisType = context->GetData(JsonTagNames::AnalysisType);
        if (analysisType.isValid() && !analysisType.isNull())
          PredictModel->Set<size_t>(PlsAnalysisIdentityModel::AnalysisTypeId, static_cast<size_t>(analysisType.toInt()));

        const auto analysisParentId = context->GetData(JsonTagNames::AnalysisParentId);
        if (analysisParentId.isValid() && !analysisParentId.isNull())
          PredictModel->Set<size_t>(PlsPredictAnalysisModel::ParentId, static_cast<size_t>(analysisParentId.toInt()));

        const auto analysisId = context->GetData(JsonTagNames::analysis_id);
        if (analysisId.isValid() && !analysisId.isNull())
          PredictModel->Set<size_t>(PlsAnalysisIdentityModel::AnalysisId, static_cast<size_t>(analysisId.toInt()));

        const auto analysisTable = context->GetData(JsonTagNames::AnalysisTable);
        if (analysisTable.isValid() && !analysisTable.isNull())
          PredictModel->Set<AnalysisTable>(PlsAnalysisIdentityModel::AnalysisTable, analysisTable.value<AnalysisTable>());

        const auto valueType = context->GetData(JsonTagNames::ValueType);
        if (valueType.isValid() && !valueType.isNull())
          PredictModel->Set<Parameters>(PlsAnalysisIdentityModel::AnalysisParameters, { _S(valueType.toString()), });
        PredictModel->Set<AnalysisData>(PlsAnalysisIdentityModel::AnalysisData, {});
      }
    }
  }
}

void ViewPageProjectPlsPredict::UpdateModelMenu()
{
  if (const auto modelMenuButton = findChild<ModelSettingsWidget*>())
  {
    if (MenuContext)
      DisconnectMenuSignals();

    const auto id = static_cast<int>(PredictModel->Get<size_t>(PlsAnalysisIdentityModel::AnalysisId));
    if (id > 0)
    {
      const auto existModelContext = new ExistModelSettingsContext(modelMenuButton);
      connect(existModelContext, &ExistModelSettingsContext::SaveAsNew, this, &ViewPageProjectPlsPredict::ShowSaveDialog);
      existModelContext->SaveChangesAction->setVisible(false);
      MenuContext = existModelContext;
    }
    else
    {
      const auto newModelContext = new NewModelSettingsContext(modelMenuButton);
      connect(newModelContext, &NewModelSettingsContext::SaveNewModel, this, &ViewPageProjectPlsPredict::ShowSaveDialog);
      MenuContext = newModelContext;
    }

    ConnectMenuSignals();
    modelMenuButton->SetMenu(MenuContext);
  }
}

void ViewPageProjectPlsPredict::ConnectMenuSignals()
{
  connect(MenuContext, &AnalysisModelSettingsContext::OpenDataTable, this, &ViewPageProjectPlsPredict::OpenDataTable);
  connect(MenuContext, &AnalysisModelSettingsContext::OpenModelParameters, this, &ViewPageProjectPlsPredict::RequestParentAnalyisList);
  connect(MenuContext, &AnalysisModelSettingsContext::OpenMarkup, this, &ViewPageProjectPlsPredict::OpenMarkup);
  connect(MenuContext, &AnalysisModelSettingsContext::Export, this, &ViewPageProjectPlsPredict::Export);
}

void ViewPageProjectPlsPredict::DisconnectMenuSignals()
{
  disconnect(MenuContext, &AnalysisModelSettingsContext::OpenDataTable, this, &ViewPageProjectPlsPredict::OpenDataTable);
  disconnect(MenuContext, &AnalysisModelSettingsContext::OpenModelParameters, this, &ViewPageProjectPlsPredict::RequestParentAnalyisList);
  disconnect(MenuContext, &AnalysisModelSettingsContext::OpenMarkup, this, &ViewPageProjectPlsPredict::OpenMarkup);
  disconnect(MenuContext, &AnalysisModelSettingsContext::Export, this, &ViewPageProjectPlsPredict::Export);
}

void ViewPageProjectPlsPredict::OpenDataTable()
{
  if (auto contextRoot = GenesisContextRoot::Get())
  {
    if (const auto markupContext = contextRoot->GetChildContextStepBasedMarkup())
    {
      const auto tableId = PredictModel->Get<size_t>(AnalysisIdentityModel::AnalysisTableId);
      markupContext->SetData(ContextTagNames::ReferenceId, static_cast<int>(tableId));
      markupContext->SetData(ContextTagNames::analysis_builded_model, static_cast<int>(Constants::AnalysisType::PLSPredict));
      const auto& analysisTable = PredictModel->Get<AnalysisTable>(AnalysisIdentityModel::AnalysisTable);
      markupContext->SetData(ContextTagNames::checked_peaks, QVariant::fromValue<IntVector>(analysisTable.CheckedValues.MarkersIds));
      markupContext->SetData(ContextTagNames::checked_samples, QVariant::fromValue<IntVector>(analysisTable.CheckedValues.SamplesIds));
      const auto data = PredictModel->Get<PredictAnalysisMetaInfoList>(PlsPredictResponseModel::TTitle);
      std::vector<std::optional<double>> concentrations(analysisTable.Concentrations.front().size(), std::nullopt);
      for (const auto& item : data)
        concentrations[item.SampleId] = item.YConcentration;

      markupContext->SetData(ContextTagNames::yConcentrations, QVariant::fromValue(concentrations));
      markupContext->SetData(ContextTagNames::AnalysisId,
        static_cast<int>(PredictModel->Get<size_t>(AnalysisIdentityModel::AnalysisId)));
      GenesisWindow::Get()->ShowPage(ViewPageNames::ViewPageAnalysisDataTablePageName);
    }
  }
}

void ViewPageProjectPlsPredict::OpenMarkup()
{
}

void ViewPageProjectPlsPredict::Export()
{
  const auto requestData = PredictModel->Get<QByteArray>(AnalysisIdentityModel::ExportRequestBody);
  const auto doc = QJsonDocument::fromJson(requestData);
  auto request = doc.object();
  request.insert(JsonTagNames::project_id, GenesisContextRoot::Get()->ProjectId());
  API::REST::ExportTables(request, [](QNetworkReply* reply, QByteArray data)
    {
      auto path = QFileDialog::getExistingDirectory(nullptr, tr("Open folder"), QDir::currentPath(),
        QFileDialog::ReadOnly | QFileDialog::ShowDirsOnly);
      path = QDir::toNativeSeparators(path);
      if (path.isEmpty())
        return;
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

void ViewPageProjectPlsPredict::RequestParentAnalyisList()
{
  const auto tableId = PredictModel->Get<size_t>(AnalysisIdentityModel::AnalysisTableId);
  API::REST::GetParentAnalysisList(tableId, Constants::AnalysisType::PLS,
    [&](QNetworkReply*, QJsonDocument document)
    {
      const auto object = document.object();
      if (object.value(JsonTagNames::Error).toBool())
      {
        Notification::NotifyError(tr("Error receive analysis model list."));
      }
      else
      {
        const auto serialzier = ServiceLocator::Instance().Resolve<PlsPredictParentAnalysisListModel, IJsonSerializer>();
        const auto model = serialzier->ToModel(object);
        ShowPlsPredictDialog(model);
      }
    },
    [](QNetworkReply*, QNetworkReply::NetworkError err)
    {
      Notification::NotifyError(tr("Error receive analysis model list."), err);
    });
}

void ViewPageProjectPlsPredict::ShowPlsPredictDialog(Model::IdentityModelPtr parentAnalysisListModel)
{
  ParentAnalysisIds parentModels;
  const auto parentId = PredictModel->Get<size_t>(PlsPredictAnalysisModel::ParentId);

  parentAnalysisListModel->Exec<ParentAnalysisIds>(PlsPredictParentAnalysisListModel::ParentList,
    [&](const ParentAnalysisIds& parentIds)
    {
      const auto it = std::find_if(parentIds.begin(), parentIds.end(),
        [&](const ParentAnalysisId& parentItem) { return parentItem.Id == parentId; });
      if (it != parentIds.end())
        parentModels.push_back(*it);
    });
  const auto dialog = new WebDialogPlsPredictSettings();
  dialog->SetItems(parentModels);
  connect(dialog, &WebDialog::Accepted, this, [&, parentId]() { LoadParentPlsAnalysis(parentId); });
  dialog->Open();
}

void ViewPageProjectPlsPredict::LoadParentPlsAnalysis(size_t id)
{
  API::REST::GetAnalysis(id,
    [&](QNetworkReply*, QJsonDocument doc)
    {
      const auto json = doc.object();
      if (json.value(JsonTagNames::Error).toBool())
      {
        Notification::NotifyError(tr("Error receive pls analysis model."));
      }
      else
      {
        const auto serializer = ServiceLocator::Instance().Resolve<PlsAnalysisIdentityModel, IJsonSerializer>();
        ParentModel = serializer->ToModel(json.value(JsonTagNames::Data).toObject());
        emit ParentModelLoaded();
      }
    },
    [](QNetworkReply*, QNetworkReply::NetworkError err)
    {
      Notification::NotifyError(tr("Error receive pls analysis model : %1").arg(err));
    });
}

void ViewPageProjectPlsPredict::RecalcModel()
{
  const auto model = std::make_shared<PlsFitRequestModel>();
  ParentModel->Exec<PlsParameters>(PlsAnalysisIdentityModel::PlsParameters,
    [&](const PlsParameters& parameters)
    {
      model->Set(PlsFitRequestModel::Autoscale, parameters.Autoscale);
      model->Set(PlsFitRequestModel::Normdata, parameters.Normdata);
      model->Set(PlsFitRequestModel::Predict, parameters.Predict);
      model->Set(PlsFitRequestModel::CVCount, static_cast<size_t>(parameters.SamplesPerCount));
      model->Set(PlsFitRequestModel::NMaxLV, static_cast<size_t>(parameters.nMaxLv));
      model->Set<std::string>(PlsFitRequestModel::LayerName, parameters.LayerName);
    });
  PredictModel->Exec<size_t>(PlsAnalysisIdentityModel::AnalysisTableId,
    [&](size_t tableId) { model->Set<int>(PlsFitRequestModel::TableId, static_cast<int>(tableId)); });

  PredictModel->Exec<size_t>(PlsPredictAnalysisModel::ParentId,
    [&](size_t value) { model->Set(PlsFitRequestModel::ParentId, value); });

  PredictModel->Exec<AnalysisTable>(PlsPredictResponseModel::AnalysisTable,
    [&](const AnalysisTable& data)
    {
      model->Set(PlsFitRequestModel::SampleIds, data.CheckedValues.SamplesIds);
      model->Set(PlsFitRequestModel::MarkerIds, data.CheckedValues.MarkersIds);
    });
  PredictModel->Exec<AnalysisData>(AnalysisIdentityModel::AnalysisData,
    [&](const AnalysisData& data)
    {
      IntVector sampleIds;
      sampleIds.reserve(data.Data.TestData.size());
      std::transform(data.Data.TestData.begin(), data.Data.TestData.end(), std::back_inserter(sampleIds),
        [&](const AnalysisMetaInfo& info) { return info.SampleId; });
      model->Set(PlsFitRequestModel::TestSampleIds, sampleIds);
    });

  PredictModel->Exec<Parameters>(AnalysisIdentityModel::AnalysisParameters,
    [&](const Parameters& params) { model->Set<std::string>(PlsFitRequestModel::Key, params.ValueType); });

  auto& analysisTable = PredictModel->Get<AnalysisTable>(AnalysisIdentityModel::AnalysisTable);
  PredictModel->Exec<PredictAnalysisMetaInfoList>(PlsPredictResponseModel::TTitle,
    [&](const PredictAnalysisMetaInfoList& list)
    {
      OptionalDoubleVector concentrations(analysisTable.Concentrations.front().size(), std::nullopt);
      for (const auto& item : list)
        concentrations[item.SampleId] = item.YConcentration;

      model->Set(PlsFitRequestModel::SampleConcentrations, concentrations);
    });

  model->Set<IntVector>(PlsFitRequestModel::TestSampleIds, {});
  model->Set<DoubleVector>(PlsFitRequestModel::TestSampleConcentrations, {});
  model->Set<IntVector>(PlsFitRequestModel::PossibleTestSampleIds, {});
  model->Set<DoubleVector>(PlsFitRequestModel::PossibleTestSampleConcentrations, {});

  const auto jsonRequestSerializer = ServiceLocator::Instance().Resolve<PlsFitRequestModel, IJsonSerializer>();
  const auto jsonRequest = jsonRequestSerializer->ToJson(model);

  API::REST::MarkupPlsPred(jsonRequest,
    [&](QNetworkReply* reply, QJsonDocument doc)
    {
      qDebug() << doc;
      if (const auto jsonObj = doc.object(); jsonObj.value(JsonTagNames::Error).toBool())
      {
        //        qDebug() << doc;
        Notification::NotifyError("Pls predict request return Error.");
      }
      else
      {
        const auto jsonSerializer = ServiceLocator::Instance().Resolve<PlsPredictResponseModel, IJsonSerializer>();
        PredictResponseModel = jsonSerializer->ToModel(jsonObj.value(JsonTagNames::Result).toObject());
        ConvertResponseToModel();
        FillPredictModelFromContext();
        emit PredictModelLoaded();
      }
    },
    [](QNetworkReply*, QNetworkReply::NetworkError)
    {
      Notification::NotifyError("Markup pls predict error.");
    });
}

void ViewPageProjectPlsPredict::UpdateTableFromPredictModel()
{
  UpdateModelMenu();
  const auto tableModel = new PlsPredictTableModel();
  tableModel->SetPredictTable(PredictModel->Get<PredictAnalysisMetaInfoList>(PlsPredictResponseModel::TTitle));
  TableView->SetModel(tableModel);
  TableView->SetResizeMode(QHeaderView::ResizeToContents);
}

void ViewPageProjectPlsPredict::ConvertResponseToModel()
{
  if (!PredictModel)
    PredictModel = std::make_shared<PlsPredictAnalysisModel>();
  PredictResponseModel->Exec<PredictAnalysisMetaInfoList>(PlsPredictResponseModel::TTitle,
    [&](const PredictAnalysisMetaInfoList& list) { PredictModel->Set(PlsPredictResponseModel::TTitle, list); });
  PredictResponseModel->Exec<QByteArray>(AnalysisIdentityModel::ExportRequestBody,
    [&](const QByteArray& data)
    {
      PredictModel->Set(AnalysisIdentityModel::ExportRequestBody, data);
    });
  PredictResponseModel->Exec<PlsPredictParameter>(PlsPredictResponseModel::AnalysisParams,
    [&](const PlsPredictParameter& params) { PredictModel->Set(PlsPredictResponseModel::AnalysisParams, params); });
}
