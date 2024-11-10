#include "view_page_analysis_data_table_page.h"

#include "genesis_window.h"
#include "genesis_style/style.h"
#include "known_view_names.h"
#include "api/network_manager.h"
#include "dialogs/web_dialog_pls_build_settings_refactor.h"
#include "dialogs/web_dialog_pls_concentration_settings.h"
#include "dialogs/web_dialog_pls_predict_settings.h"
#include "logic/context_root.h"
#include "logic/known_context_names.h"
#include "logic/known_context_tag_names.h"
#include "logic/known_json_tag_names.h"
#include "logic/notification.h"
#include "logic/service/ijson_serializer.h"
#include "logic/service/service_locator.h"
#include "logic/models/pls_analysis_identity_model.h"
#include "logic/models/pls_fit_request_model.h"
#include "logic/models/pls_predict_parent_analysis_list_model.h"
#include "logic/models/reference_analysis_data_table_model.h"
#include "logic/models/pca_analysis_request_model.h"
#include "logic/models/mcr_analysis_request_model.h"
#include "logic/tree_model.h"
#include "ui/controls/data_table_caption_button_controls.h"
#include "ui/dialogs/web_dialog_ratio_matrix_settings.h"
#include "ui/dialogs/web_dialog_analysis_model_settings.h"
#include "ui/dialogs/web_dialog_mcr_build_settings.h"
#include "ui/dialogs/templates/tree_confirmation.h"
#include <ui/dialogs/templates/save_edit.h>
#include "ui/item_models/analysis_data_table_model.h"
#include "ui/widgets/parameter_table_view.h"
#include "ui/dialogs/web_dialog_calculate_coefficient_table.h"

#include <QVBoxLayout>
#include <QFileDialog>
#include <QMetaEnum>
#include <qrandom.h>
#include <QRegExp>
#include <QJsonObject>

using namespace Widgets;
using namespace Controls;
using namespace Names;
using namespace Dialogs;
using namespace Core;
using namespace Constants;
using namespace Models;
using namespace Service;
using namespace Structures;
using namespace Model;

namespace
{
  const static int MinItemsForModel = 3;
}

namespace Views
{
  ViewPageAnalysisDataTablePage::ViewPageAnalysisDataTablePage(QWidget* parent)
    : View(parent)
  {
    SetupUi();
    ConnectSignals();
  }

  ViewPageAnalysisDataTablePage::~ViewPageAnalysisDataTablePage()
  {
    delete TableView;
  }

  void ViewPageAnalysisDataTablePage::ApplyContextStepBasedMarkup(const QString& dataId, const QVariant& data)
  {
    bool isReset = isDataReset(dataId, data);
    {
      QSet<QString> nessesaryTags = { ContextTagNames::ReferenceId,
                                           ContextTagNames::StepBasedMarkupId,
                       ContextTagNames::analysis_builded_model };
      QSet<QString> nessesaryDataTags = { ContextTagNames::checked_peaks,
                         ContextTagNames::checked_samples,
                         ContextTagNames::yConcentrations };
      nessesaryTags.unite(nessesaryDataTags);

      if (!isReset && !nessesaryTags.contains(dataId))
        return;

      if (nessesaryDataTags.contains(dataId))
      {
        UpdateCheckBoxesState(dataId, data);
        return;
      }

      if (dataId == ContextTagNames::analysis_builded_model)
      {
        OpenedFrom = (data.isValid() && !data.isNull())
          ? static_cast<AnalysisType>(data.toInt())
          : AnalysisType::NONE;
        ButtonControls->SetupBuildSettings(OpenedFrom);
        return;
      }
    }

    if (dataId == ContextTagNames::StepBasedMarkupId)
    {
      ButtonControls->ShowReturnToMarkupAction(
        data.isValid()
        && !data.isNull()
        && data.toInt() > 0
      );
      return;
    }

    if (isReset || dataId == ContextTagNames::ReferenceId)
    {
      const auto root = GenesisContextRoot::Get();
      if (!root)
        return;
      OpenedFrom = GetTypeFromContext();
      if (!isReset)
        TableId = data.toInt();
      else
        TableId = root->GetContextStepBasedMarkup()->GetData(ContextTagNames::ReferenceId).toInt();

      QVariant markupId = root->GetContextStepBasedMarkup()->GetData(ContextTagNames::StepBasedMarkupId);

      ButtonControls->ShowReturnToMarkupAction(
        markupId.isValid()
        && !markupId.isNull()
        && markupId.toInt() > 0
      );

      auto overlayId = GenesisWindow::Get()->ShowOverlay(tr("Loading data"));
      API::REST::GetReferenceDataTable(TableId,
        [&, overlayId](QNetworkReply*, QJsonDocument doc)
        {
          if (const auto obj = doc.object();
            obj.value(JsonTagNames::Error).toBool())
          {
            Notification::NotifyError(tr("Get Reference Table network error: %1").arg(obj.value("msg").toString()));
          }
          else
          {
            const auto serializer = ServiceLocator::Instance().Resolve<ReferenceAnalysisDataTableModel, IJsonSerializer>();
            DataTableModel = serializer->ToModel(obj.value("data_table").toObject());
            if (!TableView)
              return;

            auto model = new AnalysisDataTableModel();
            model->SetModel(DataTableModel);
            TableView->SetModel(model);
            TableTitle = QString::fromStdString(DataTableModel->Get<std::string>(ReferenceAnalysisDataTableModel::Title));
            TableComment = QString::fromStdString(DataTableModel->Get<std::string>(ReferenceAnalysisDataTableModel::Comment));
            ButtonControls->SetupBuildSettings(OpenedFrom, TableTitle);
            DataTableModel->Exec<std::string>(ReferenceAnalysisDataTableModel::TableType,
              [&](const std::string& type)
              {
                ButtonControls->ShowRatioMatrixAction(type == JsonTagNames::DataTableType.toStdString() || type == JsonTagNames::MergedTableType.toStdString());
                ButtonControls->ShowCalculateCoefficientAction(type == JsonTagNames::DataTableIdentifiedType.toStdString() || type == JsonTagNames::MergedTableIdentifiedType.toStdString() || type == JsonTagNames::ImportedTableIdentifiedType.toStdString());
              });
            SetupCheckBoxesState();
          }
          GenesisWindow::Get()->RemoveOverlay(overlayId);
        },
        [overlayId](QNetworkReply*, QNetworkReply::NetworkError e)
        {
          Notification::NotifyError(tr("Get Reference Table network error: %1")
            .arg(QMetaEnum::fromType<QNetworkReply::NetworkError>().key(e)));
          GenesisWindow::Get()->RemoveOverlay(overlayId);
        });

      ButtonControls->SetEnabledBuildPlotAction(false);
      ButtonControls->SetEnabledCalculateCoefficientAction(false);
      API::REST::GetAvailableCalculationsForTable(TableId,
        [&, overlayId](QNetworkReply*, QJsonDocument result)
        {
          const auto obj = result.object();
          if (obj.isEmpty() || obj.value(JsonTagNames::Error).toBool())
          {
            Notification::NotifyError(tr("Get Reference Table network error: %1").arg(obj.value("msg").toString()));
          }
          else
          {
            ButtonControls->SetEnabledCalculateCoefficientAction(obj["is_available_coefficients"].toBool());
            ButtonControls->SetEnabledBuildPlotAction(obj["is_available_plot_templates"].toBool());
          }
          GenesisWindow::Get()->RemoveOverlay(overlayId);
        },
        [overlayId](QNetworkReply*, QNetworkReply::NetworkError e)
        {
          Notification::NotifyError(tr("Get Reference Table network error: %1")
            .arg(QMetaEnum::fromType<QNetworkReply::NetworkError>().key(e)));
          GenesisWindow::Get()->RemoveOverlay(overlayId);
        });
    }
  }

  void ViewPageAnalysisDataTablePage::ApplyContextModules(const QString& dataId, const QVariant& data)
  {
    bool isReset = isDataReset(dataId, data);

    using namespace Names::ModulesContextTags;

    if (!isReset && dataId != kModule)
      return;

    Module module = MNoModule;

    if (isReset)
    {
      auto context = Core::GenesisContextRoot::Get()->GetContextModules();
      module = Module(context->GetData(kModule).toInt());
    }
    else if (dataId == kModule)
    {
      module = Module(data.toInt());
    }

    ButtonControls->SetActiveModule(module);
  }

  void ViewPageAnalysisDataTablePage::showEvent(QShowEvent* event)
  {
    View::showEvent(event);

    //    if (const auto root = GenesisContextRoot::Get())
    //    {
    //      if (const auto context = root->GetChildContextStepBasedMarkup())
    //      {
    //        const auto openedData = context->GetData(ContextTagNames::analysis_builded_model);
    //        if (openedData.isValid() && !openedData.isNull())
    //        {
    //          OpenedFrom = static_cast<AnalysisType>(openedData.toInt());
    //          ButtonControls->SetupBuildSettings(OpenedFrom);
    //        }

    //        const auto markupId = context->GetData(ContextTagNames::MarkupId);
    //        if (markupId.isValid() && !markupId.isNull() && markupId.toInt() > 0)
    //          ButtonControls->ShowReturnToMarkupAction(true);
    //      }
    //    }
  }

  void ViewPageAnalysisDataTablePage::SetupUi()
  {
    setStyleSheet(Style::Genesis::GetUiStyle());

    const auto mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(Style::Scale(40), Style::Scale(32),
      Style::Scale(40), Style::Scale(32));
    mainLayout->setSpacing(Style::Scale(40));
    ButtonControls = new DataTableCaptionButtonControls("Data table", this);
    mainLayout->addWidget(ButtonControls);
    TableView = new ParameterTableView(this);
    mainLayout->addWidget(TableView);
    setLayout(mainLayout);
  }

  void ViewPageAnalysisDataTablePage::ConnectSignals()
  {
    connect(ButtonControls, &DataTableCaptionButtonControls::AnalysisIndexChanged, this, &ViewPageAnalysisDataTablePage::IndexChanged);
    connect(ButtonControls, &DataTableCaptionButtonControls::OpenMatrix, this, &ViewPageAnalysisDataTablePage::OpenRatioMatrixClicked);
    connect(ButtonControls, &DataTableCaptionButtonControls::CalculationCoefficientTable, this, &ViewPageAnalysisDataTablePage::CalculateCoefficientTableClicked);
    connect(ButtonControls, &DataTableCaptionButtonControls::CreatePlot, this, &ViewPageAnalysisDataTablePage::CreatePlotClicked);
    connect(ButtonControls, &DataTableCaptionButtonControls::Export, this, &ViewPageAnalysisDataTablePage::ExportClicked);
    connect(ButtonControls, &DataTableCaptionButtonControls::Edit, this, &ViewPageAnalysisDataTablePage::EditClicked);
    connect(ButtonControls, &DataTableCaptionButtonControls::Remove, this, &ViewPageAnalysisDataTablePage::RemoveClicked);
    connect(ButtonControls, &DataTableCaptionButtonControls::ToMarkupData, this, &ViewPageAnalysisDataTablePage::ReturnDataMarkup);
    connect(ButtonControls, &DataTableCaptionButtonControls::BuildModel, this, &ViewPageAnalysisDataTablePage::BuildModel);
    connect(ButtonControls, &DataTableCaptionButtonControls::ReturnTo, this, &ViewPageAnalysisDataTablePage::ReturnToModel);
    connect(ButtonControls, &DataTableCaptionButtonControls::Cancel, this, &ViewPageAnalysisDataTablePage::SetupCheckBoxesState);

    connect(this, &ViewPageAnalysisDataTablePage::ParentModelLoaded, this, &ViewPageAnalysisDataTablePage::SetupTableForPlsPredict);
  }

  void ViewPageAnalysisDataTablePage::IndexChanged(AnalysisType inputType)
  {
    if (!TableView)
      return;
    switch (inputType)
    {
    case AnalysisType::PLS:
      ShowPlsDialog();
      break;
    case AnalysisType::PLSPredict:
      RequestParentPlsAnalysis();
      break;
    default:
      break;
    }

    ButtonControls->ShowTableControl(OpenedFrom == AnalysisType::NONE
      && inputType == AnalysisType::NONE);

    const auto type = inputType != AnalysisType::NONE
      ? inputType
      : OpenedFrom;

    TableView->SetHideYConcentration(type < AnalysisType::PLS);
    TableView->AllowEmptyConcentration(type == AnalysisType::PLSPredict);
    TableView->SetCheckBoxVisible(type != AnalysisType::NONE);
    if (DataTableModel)
    {
      const auto tableType = DataTableModel->Get<std::string>(ReferenceAnalysisDataTableModel::TableType);
      if (tableType == JsonTagNames::HeightRatioMatrixType.toStdString() || tableType == JsonTagNames::ImportedTableType.toStdString() || tableType == JsonTagNames::ImportedTableIdentifiedType.toStdString())
        TableView->FillCustomParameterComboBox();
      else if (tableType == JsonTagNames::CoefficientTableIdentifiedType.toStdString() || type != AnalysisType::NONE)
        TableView->FillParameterComboBoxShort();
      else
        TableView->FillParameterComboBoxFull();
    }
  }

  void ViewPageAnalysisDataTablePage::OpenRatioMatrixClicked()
  {
    const auto dialog = new WebDialogRatioMatrixSettings(this);
    connect(dialog, &WebDialog::Accepted, this, [&, dialog]()
      {
        QJsonObject json;
        json.insert(JsonTagNames::Title, dialog->GetTitle());
        json.insert(JsonTagNames::Comment, dialog->GetComment());
        json.insert(JsonTagNames::TableId, TableId);
        if (const auto contextRoot = GenesisContextRoot::Get())
        {
          if (const auto projectContext = contextRoot->GetContextProject())
            json.insert(JsonTagNames::project_id, projectContext->GetData(JsonTagNames::Id).toInt());
        }
        QJsonObject params{
          { JsonTagNames::WindowSize, dialog->GetWindowSize() },
          { JsonTagNames::MinValue, dialog->GetMinimum() },
          { JsonTagNames::MaxValue, dialog->GetMaximum() },
          { JsonTagNames::IncludeMarkers, dialog->GetIsIncludeMarkers() },
        };
        json.insert(JsonTagNames::Parameters, params);
        auto overlayId = GenesisWindow::Get()->ShowOverlay(tr("Loading data..."));
        API::REST::GetHeigthRatioMatrix(json,
          [&, overlayId, title = dialog->GetTitle()](QNetworkReply*, QJsonDocument doc)
          {
            const auto object = doc.object();
            if (object.value(JsonTagNames::Error).toBool())
            {
              Notification::NotifyError(object.value(JsonTagNames::Msg).toString(), tr("Error receive ratio matrix data"));
            }
            else
            {
              Notification::NotifySuccess(tr("Matrix %1 generated successfuly").arg(title), tr("Ratio matrix generated succes"));
              TreeModel::ResetInstances("TreeModelHeightRatio");
            }
            GenesisWindow::Get()->RemoveOverlay(overlayId);
          },
          [overlayId](QNetworkReply*, QNetworkReply::NetworkError err)
          {
            Notification::NotifyError(tr("Error receive ratio matrix data."), err);
            GenesisWindow::Get()->RemoveOverlay(overlayId);
          });
      });
    dialog->Open();
  }

  void ViewPageAnalysisDataTablePage::CalculateCoefficientTableClicked()
  {
    WebDialogCalculateCoefficientTable* dialog = new WebDialogCalculateCoefficientTable(this);
    dialog->LoadCoefficientByTableId(TableId);
    dialog->Open();
  }

  void ViewPageAnalysisDataTablePage::CreatePlotClicked()
  {
    auto context = Core::GenesisContextRoot::Get()->GetCreationPlot();
    if (!context)
      return;

    context->SetData(Names::CreationPlot::TableId, TableId);
    context->SetData(Names::CreationPlot::ParameterId, TableView->GetModel()->GetCurrentParameter());
    // context->SetData(Names::CreationPlot::Data, QVariant::fromValue(TableView->GetModel()->GetCurrentData()));
    // context->SetData(Names::CreationPlot::ElementIds, QVariant::fromValue(TableView->GetModel()->GetLibraryElementIds()));
    const auto serializer = ServiceLocator::Instance().Resolve<ReferenceAnalysisDataTableModel, IJsonSerializer>();
    auto modelJson = serializer->ToJson(TableView->GetModel()->GetModel());
    context->SetData(Names::CreationPlot::ModelJson, modelJson);


    GenesisWindow::Get()->ShowPage(ViewPageNames::ViewPageCreateIdentificationPlotName);
  }

  void ViewPageAnalysisDataTablePage::ExportClicked()
  {
    const auto root = GenesisContextRoot::Get();
    if (!root)
      return;
    const auto projectId = GenesisContextRoot::Get()->ProjectId();
    API::REST::ExportDataTables(projectId, { TableId },
      [](QNetworkReply* reply, QByteArray buffer)
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
            file.write(buffer);
            file.close();
          }
          else
          {
            Notification::NotifyError(tr("Can't open file: %1").arg(fileName));
          }
        }
      },
      [](QNetworkReply*, QNetworkReply::NetworkError e)
      {
        Notification::NotifyError(tr("Get export data tables network error: %1").arg(QMetaEnum::fromType<QNetworkReply::NetworkError>().key(e)));
      });
  }

  void ViewPageAnalysisDataTablePage::EditClicked()
  {
    auto modelTitle = TableTitle;
    auto modelComment = TableComment;
    auto id = TableId;

    Dialogs::Templates::SaveEdit::Settings s;
    s.header = tr("Editing");
    s.titleHeader = tr("Model title");
    s.commentHeader = tr("Comment");
    s.titlePlaceholder = tr("Enter title");
    s.commentPlaceholder = tr("Enter comment");
    s.defaultTitle = modelTitle;
    s.defaultComment = modelComment;
    s.titleTooltipsSet.empty = tr("Title should be not empty");
    s.titleTooltipsSet.forbidden = tr("Title is alredy in use");
    s.titleTooltipsSet.notChecked = tr("Title is not checked yet");
    s.titleTooltipsSet.valid = tr("Title is valid");

    auto dial = new Dialogs::Templates::SaveEdit(this, s);
    connect(dial, &WebDialog::Accepted, this,
      [this, dial, id, modelTitle, modelComment]()
      {
        auto newTitle = dial->getTitle();
        auto newComment = dial->getComment();

        if (newTitle == modelTitle && newComment == modelComment)
        {
          Notification::NotifyInfo(tr("There were no changes"));
          return;
        }
        else
        {
          API::REST::EditTable(id, newTitle, newComment,
            [](QNetworkReply*, QJsonDocument doc)
            {
              const auto jsonObject = doc.object();
              if (jsonObject.value(JsonTagNames::Error).toBool())
              {
                Notification::NotifyError(jsonObject.value(JsonTagNames::Msg).toString(), tr("Server error"));
                return;
              }
              TreeModel::ResetInstances("TreeModelDynamicProjectModelsList");
              TreeModel::ResetInstances("TreeModelNumericData");
              GenesisContextRoot::Get()->GetContextStepBasedMarkup()->ReloadData();
              Notification::NotifySuccess(tr("Analysis successfully updated"));
            },
            [](QNetworkReply*, QNetworkReply::NetworkError e)
            {
              Notification::NotifyError(tr("Network error"), e);
            });
        }
      });
    dial->Open();
  }

  void ViewPageAnalysisDataTablePage::RemoveClicked()
  {
    auto dial = Dialogs::Templates::TreeConfirmation::DeleteTablesWithDependencies({ {TableId, TableTitle} }, this, "TreeModelNumericData");
    connect(dial, &WebDialog::Accepted, this,
      []()
      {
        TreeModel::ResetInstances("TreeModelDynamicProjectModelsList");
        GenesisWindow::Get()->ShowPage(ViewPageNames::ViewProjectInfoPageName);
      });
    dial->Open();
  }

  void ViewPageAnalysisDataTablePage::ReturnDataMarkup()
  {
    if (const auto contextRoot = GenesisContextRoot::Get())
    {
      GenesisWindow::Get()->ShowPage(ViewPageNames::ViewPageProjectMarkupName);
    }
  }

  void ViewPageAnalysisDataTablePage::ReturnToModel()
  {
    QString viewPageName = ViewPageNames::ViewPageProjectPCAGraphicsPlotPageName;
    switch (OpenedFrom)
    {
    case AnalysisType::PLS:
      viewPageName = ViewPageNames::ViewPageProjectPLSGraphicsPlotPageName;
      break;
    case AnalysisType::MCR:
      viewPageName = ViewPageNames::ViewPageProjectMcrGraphicPlotPageName;
      break;
    case AnalysisType::PLSPredict:
      viewPageName = ViewPageNames::ViewPageProjectPlsPredictName;
      break;
    default:
      break;
    }
    GenesisWindow::Get()->ShowPage(viewPageName);
  }

  void ViewPageAnalysisDataTablePage::BuildModel(AnalysisType type)
  {
    switch (type)
    {
    case AnalysisType::PCA:
      BuildPca();
      break;
    case AnalysisType::MCR:
      BuildMcr();
      break;
    case AnalysisType::PLS:
      BuildPls();
      break;
    case AnalysisType::PLSPredict:
      BuildPlsPredict();
      break;
    }
  }

  void ViewPageAnalysisDataTablePage::BuildPls()
  {
    if (const auto tableModel = TableView->GetModel())
    {
      const auto checkedSamples = tableModel->GetCheckedSamples();
      const auto checkedPeaks = tableModel->GetCheckedPeaks();
      auto checkedRows = std::count(checkedSamples.begin(), checkedSamples.end(), true);
      auto checkedCols = std::count(checkedPeaks.begin(), checkedPeaks.end(), true);
      if (checkedRows >= MinItemsForModel && checkedCols >= MinItemsForModel)
      {
        auto dial = new WebDialogPLSBuildSettingsRefactor(this);
        dial->SetMaxPCCount(std::min(checkedCols, checkedRows));
        dial->SetMaxSamplesPerSegmentCount(checkedRows - 1);
        const auto testData = GetTestingData();
        dial->SetTestData(testData);
        connect(dial, &WebDialog::Accepted, this,
          [&, dial]()
          {
            SendPlsRequestData(dial->GetSelectedData(),
              dial->SamplesPerSegment(), dial->MaxCountOfPC(),
              dial->Autoscale(), dial->Normalization(),
              dial->IsSegment(), dial->IsCross());
          });
        dial->Open();
      }
      else
      {
        Notification::NotifyError(tr("To run the analysis, you must select at least 3 rows and 3 columns in the table"), tr("Analysis launch error"));
      }
    }
  }

  void ViewPageAnalysisDataTablePage::ShowPlsDialog()
  {
    auto dial = new WebDialogPLSConcentrationSettings(this);
    DataTableModel->Exec<StringList>(ReferenceAnalysisDataTableModel::LayerNames,
      [&](const StringList& names)
      {
        QStringList layers;
        layers.reserve(names.size());
        std::transform(names.begin(), names.end(), std::back_inserter(layers),
          [&](const std::string& name) { return QString::fromStdString(name); });
        dial->FillLayers(layers);
      });
    using method = WebDialogPLSConcentrationSettings::ConcentrationGatheringMethod;
    connect(dial, &WebDialog::Accepted, this,
      [&, dial]()
      {
        const auto method = dial->getConcentrationGatheringMethod();
        const auto model = TableView->GetModel();
        if (method == method::autoRandomly)
        {
          auto randomConcentrations = model->GetConcentrations();
          std::generate(randomConcentrations.begin(), randomConcentrations.end(),
            []() { return QRandomGenerator::global()->bounded(5, 95); });
          model->SetConcentration(randomConcentrations);
        }
        else if (method == method::autoFromLayers)
        {
          SelectedLayerForPls = dial->SelectedLayer();
          const auto names = DataTableModel->Get<StringList>(ReferenceAnalysisDataTableModel::LayerNames);
          const auto it = std::find(names.begin(), names.end(), SelectedLayerForPls.toStdString());
          if (it != names.end())
          {
            const auto index = std::distance(names.begin(), it);
            DataTableModel->Exec<OptionalDouble2DVector>(ReferenceAnalysisDataTableModel::LayerConcentrationData,
              [&](const OptionalDouble2DVector& data)
              {
                std::vector<std::optional<double>> concentrations;
                concentrations.reserve(data[index].size());
                std::copy(data[index].begin(), data[index].end(), std::back_inserter(concentrations));
                model->SetConcentration(concentrations);
              });
          }
        }
        UncheckSampleCheckboxes();
      });
    connect(dial, &WebDialog::Rejected, this, [&]()
      {
        ButtonControls->SetAnalysisType(AnalysisType::NONE);
      });
    dial->Open();
  }

  void ViewPageAnalysisDataTablePage::BuildMcr()
  {
    if (const auto tableModel = TableView->GetModel())
    {
      const auto checkedSamples = tableModel->GetCheckedSamples();
      const auto checkedPeaks = tableModel->GetCheckedPeaks();
      auto checkedRows = std::count(checkedSamples.begin(), checkedSamples.end(), true);
      auto checkedCols = std::count(checkedPeaks.begin(), checkedPeaks.end(), true);
      if (checkedRows >= MinItemsForModel && checkedCols >= MinItemsForModel)
      {
        const auto dialog = new WebDialogMCRBuildSettings(this);
        dialog->SetParameterType(TableView->GetParameterType());
        dialog->SetSampleIds(GetCheckedSamples());
        dialog->SetMarkersIds(GetCheckedPeaks());
        dialog->SetSampleNames(GetCheckedSampleNames());
        dialog->SetTableId(TableId);
        connect(dialog, &WebDialog::Accepted, this, [&, dialog]()
          {
            auto model = std::make_shared<McrAnalysisRequestModel>();
            model->Set(McrAnalysisRequestModel::NumberComponents, dialog->GetComponentCurrentValue());
            model->Set(McrAnalysisRequestModel::TolErrorChange, dialog->GetEvaluatedValue());
            model->Set(McrAnalysisRequestModel::Method, dialog->GetMethod());
            model->Set(McrAnalysisRequestModel::MaxIter, dialog->GetMaxIterations());
            model->Set(McrAnalysisRequestModel::ReferenceSettings, dialog->GetReferenceSettings());
            model->Set(McrAnalysisRequestModel::ClosedSystem, dialog->IsClosedSystem());
            model->Set(McrAnalysisRequestModel::Normdata, dialog->IsNormData());
            model->Set(McrAnalysisRequestModel::SampleIds, GetCheckedSamples());
            model->Set(McrAnalysisRequestModel::MarkerIds, GetCheckedPeaks());
            model->Set(McrAnalysisRequestModel::Key, TableView->GetParameterType().toStdString());
            model->Set(McrAnalysisRequestModel::TableId, TableId);
            const auto serializer = ServiceLocator::Instance().Resolve<McrAnalysisRequestModel, IJsonSerializer>();
            const auto jsonRequest = serializer->ToJson(model);
            auto overlayId = GenesisWindow::Get()->ShowOverlay(tr("Loading data"));
            API::REST::GetTableAnalysisMCR(jsonRequest,
              [&, overlayId](QNetworkReply*, QJsonDocument doc)
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
                  const auto contextRoot = GenesisContextRoot::Get();
                  if (!contextRoot)
                    return;
                  if (auto mcrContext = contextRoot->GetContextMarkupVersionAnalisysMCR())
                  {
                    mcrContext->SetData({
                      { ContextTagNames::McrResponseJson, obj.value(JsonTagNames::Result).toObject()},
                      { JsonTagNames::TableId, TableId }
                      });
                  }
                  GenesisWindow::Get()->ShowPage(ViewPageNames::ViewPageProjectMcrGraphicPlotPageName);
                }
                GenesisWindow::Get()->RemoveOverlay(overlayId);
              },
              [overlayId](QNetworkReply*, QNetworkReply::NetworkError err)
              {
                GenesisWindow::Get()->RemoveOverlay(overlayId);
                Notification::NotifyError(tr("Error : %1 while receiving data.").arg(err));
              });

          });
        dialog->Open();
      }
      else
      {
        Notification::NotifyError(tr("To run the analysis, you must select at least 3 rows and 3 columns in the table"), tr("Analysis launch error"));
      }
    }
  }

  void ViewPageAnalysisDataTablePage::BuildPca()
  {
    if (const auto tableModel = TableView->GetModel())
    {
      const auto checkedSamples = tableModel->GetCheckedSamples();
      const auto checkedPeaks = tableModel->GetCheckedPeaks();
      auto checkedRows = std::count(checkedSamples.begin(), checkedSamples.end(), true);
      auto checkedCols = std::count(checkedPeaks.begin(), checkedPeaks.end(), true);
      if (checkedRows >= MinItemsForModel && checkedCols >= MinItemsForModel)
      {
        auto dialog = new WebDialogAnalysisModelSettings(this);
        std::map<WebDialogAnalysisModelSettings::Option, bool> opt;
        opt.emplace(WebDialogAnalysisModelSettings::Autoscale, false);
        opt.emplace(WebDialogAnalysisModelSettings::Normalization, false);
        dialog->AddEnabledOptions(opt);

        dialog->SetButtonName(QDialogButtonBox::StandardButton::Apply, tr("Set up the model"));
        connect(dialog, &WebDialogAnalysisModelSettings::Accepted, this, [&, dialog]()
          {
            const auto options = dialog->GetOptions();
            BuildPca(options.testFlag(WebDialogAnalysisModelSettings::Option::Normalization),
              options.testFlag(WebDialogAnalysisModelSettings::Option::Autoscale),
              options.testFlag(WebDialogAnalysisModelSettings::Option::FilterPKEnabled)
            );
          });

        dialog->Open();
      }
      else
      {
        Notification::NotifyError(tr("To run the analysis, you must select at least 3 rows and 3 columns in the table"), tr("Analysis launch error"));
      }
    }
  }

  void ViewPageAnalysisDataTablePage::BuildPca(bool normalization, bool autoscale, bool pc9599)
  {
    const auto model = std::make_shared<PcaAnalysisRequestModel>();
    model->Set<bool>(PcaAnalysisRequestModel::Autoscale, autoscale);
    model->Set<bool>(PcaAnalysisRequestModel::Normdata, normalization);
    model->Set<IntVector>(PcaAnalysisRequestModel::SampleIds, GetCheckedSamples());
    model->Set<IntVector>(PcaAnalysisRequestModel::MarkerIds, GetCheckedPeaks());
    model->Set<std::string>(PcaAnalysisRequestModel::Key, TableView->GetParameterType().toStdString());
    model->Set<int>(PcaAnalysisRequestModel::TableId, TableId);
    const auto jsonRequestSerializer = ServiceLocator::Instance().Resolve<PcaAnalysisRequestModel, IJsonSerializer>();
    const auto jsonRequest = jsonRequestSerializer->ToJson(model);
    auto overlayId = GenesisWindow::Get()->ShowOverlay(tr("Loading data"));
    API::REST::GetTableAnalysisPCA(jsonRequest,
      [&, overlayId](QNetworkReply*, QJsonDocument doc)
      {
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
          if (const auto contextRoot = GenesisContextRoot::Get())
          {
            if (auto pcaContext = contextRoot->GetContextMarkupVersionAnalisysPCA())
            {
              pcaContext->SetData
              (
                {
                  { ContextTagNames::PCAData, obj.value(JsonTagNames::Result).toObject() },
                  { JsonTagNames::analysis_id, 0},
                  { JsonTagNames::AnalysisType, static_cast<int>(AnalysisType::PCA) },
                  { JsonTagNames::AnalysisTable, QVariant::fromValue<AnalysisTable>(GetAnalysisTable()) },
                  { JsonTagNames::TableId, TableId },
                  { JsonTagNames::ValueType, TableView->GetParameterType() },
                }
                );
            }
            GenesisWindow::Get()->ShowPage(ViewPageNames::ViewPageProjectPCAGraphicsPlotPageName);
          }
          GenesisWindow::Get()->RemoveOverlay(overlayId);
        }
      },
      [overlayId](QNetworkReply*, QNetworkReply::NetworkError)
      {
        Notification::NotifyError("Netowrk error while receiving data.");
        GenesisWindow::Get()->RemoveOverlay(overlayId);
      });
  }

  void ViewPageAnalysisDataTablePage::BuildPlsPredict()
  {
    if (!PlsModelForPredict)
    {
      Notification::NotifyInfo(
        tr("You should save PLS analysis for build Predict."),
        tr("Return to PLS model and save it"));
      return;
    }
    const auto parentId = PlsModelForPredict->Get<size_t>(PlsAnalysisIdentityModel::AnalysisId);
    if (parentId == 0)
    {
      Notification::NotifyInfo(
        tr("You should save PLS analysis for build Predict."),
        tr("Return to PLS model and save it"));
      return;
    }

    if (const auto tableModel = TableView->GetModel())
    {
      const auto checkedSamples = tableModel->GetCheckedSamples();
      auto checkedRows = std::count(checkedSamples.begin(), checkedSamples.end(), true);
      if (checkedRows >= 1)
      {
        const auto model = std::make_shared<PlsFitRequestModel>();
        const auto& parameters = PlsModelForPredict->Get<PlsParameters>(PlsAnalysisIdentityModel::PlsParameters);
        model->Set(PlsFitRequestModel::Autoscale, parameters.Autoscale);
        model->Set(PlsFitRequestModel::Normdata, parameters.Normdata);
        model->Set(PlsFitRequestModel::Predict, parameters.Predict);
        model->Set(PlsFitRequestModel::CVCount, parameters.SamplesPerCount);
        model->Set(PlsFitRequestModel::NMaxLV, parameters.ChosenNumberPC);
        model->Set(PlsFitRequestModel::ParentId, parentId);
        model->Set(PlsFitRequestModel::SampleIds, GetCheckedSamples());
        model->Set(PlsFitRequestModel::MarkerIds, GetCheckedPeaks());
        const auto tdata = GetTestingData();
        IntVector testData;
        for (auto& [id, name] : tdata)
          testData.append(id);
        model->Set<IntVector>(PlsFitRequestModel::TestSampleIds, testData);
        model->Set<std::string>(PlsFitRequestModel::Key, TableView->GetParameterType().toStdString());
        const auto tableModel = TableView->GetModel();
        const auto refmodel = tableModel->GetModel();
        const auto sampleIds = refmodel->Get<IntVector>(ReferenceAnalysisDataTableModel::SampleDataIds);
        const auto concentrations = tableModel->GetConcentrations();
        const auto checkedSamples = tableModel->GetCheckedSamples();

        OptionalDoubleVector sampleConcentrations;
        sampleConcentrations.reserve(concentrations.size());
        std::copy(concentrations.begin(), concentrations.end(), std::back_inserter(sampleConcentrations));
        model->Set(PlsFitRequestModel::SampleConcentrations, sampleConcentrations);
        model->Set<IntVector>(PlsFitRequestModel::TestSampleIds, {});
        model->Set<DoubleVector>(PlsFitRequestModel::TestSampleConcentrations, {});
        model->Set<IntVector>(PlsFitRequestModel::PossibleTestSampleIds, {});
        model->Set<DoubleVector>(PlsFitRequestModel::PossibleTestSampleConcentrations, {});
        model->Set<int>(PlsFitRequestModel::TableId, TableId);
        model->Set<std::string>(PlsFitRequestModel::LayerName, SelectedLayerForPls.toStdString());
        const auto jsonRequestSerializer = ServiceLocator::Instance().Resolve<PlsFitRequestModel, IJsonSerializer>();
        const auto jsonRequest = jsonRequestSerializer->ToJson(model);
        API::REST::MarkupPlsPred(jsonRequest,
          [&](QNetworkReply* reply, QJsonDocument doc)
          {
            if (const auto jsonObj = doc.object(); jsonObj.value(JsonTagNames::Error).toBool())
            {
              //        qDebug() << doc;
              Notification::NotifyError("Pls predict request return Error.");
            }
            else
            {
              if (auto contextRoot = GenesisContextRoot::Get())
              {
                if (auto markupPlsPredict = contextRoot->GetContextMarkupVersionAnalisysPLSPredict())
                {
                  markupPlsPredict->SetData(
                    {
                      { GenesisContextNames::MarkupVersionAnalysisPLSPredict, jsonObj },
                      /// pls predict is pls type
                      { JsonTagNames::AnalysisType, static_cast<int>(AnalysisType::PLS) },
                      { JsonTagNames::ValueType, TableView->GetParameterType() },
                      { JsonTagNames::AnalysisTable, QVariant::fromValue<AnalysisTable>(GetAnalysisTable()) },
                      { JsonTagNames::AnalysisParentId, static_cast<int>(PlsModelForPredict->Get<size_t>(PlsAnalysisIdentityModel::AnalysisId)) },
                      { JsonTagNames::analysis_id, 0},
                      { JsonTagNames::TableId, static_cast<int>(PlsModelForPredict->Get<size_t>(PlsAnalysisIdentityModel::AnalysisTableId)) },
                    }
                    );
                }
                GenesisWindow::Get()->ShowPage(ViewPageNames::ViewPageProjectPlsPredictName);
              }
            }
          },
          [](QNetworkReply*, QNetworkReply::NetworkError)
          {
            Notification::NotifyError("Markup pls predict error.");
          });
      }
      else
      {
        Notification::NotifyError(tr("To run the analysis, you must select at least 1 row in the table"), tr("Analysis launch error"));
      }
    }
  }

  void ViewPageAnalysisDataTablePage::RequestParentPlsAnalysis()
  {
    if (OpenedFrom != AnalysisType::PLS)
    {
      RequestParentAnalyisList();
      return;
    }

    if (const auto root = GenesisContextRoot::Get())
    {
      if (const auto context = root->GetChildContextStepBasedMarkup())
      {
        LoadParentPlsAnalysis(context->GetData(ContextTagNames::AnalysisId).toInt());
      }
    }
  }

  void ViewPageAnalysisDataTablePage::RequestParentAnalyisList()
  {
    API::REST::GetParentAnalysisList(TableId, Constants::AnalysisType::PLS,
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

  void ViewPageAnalysisDataTablePage::ShowPlsPredictDialog(IdentityModelPtr parentAnalysisListModel)
  {
    const auto parentModels = parentAnalysisListModel->Get<ParentAnalysisIds>(PlsPredictParentAnalysisListModel::ParentList);
    const auto dialog = new WebDialogPlsPredictSettings();
    dialog->SetItems(parentModels);
    connect(dialog, &WebDialog::Accepted, this,
      [&, dialog, parentModels]()
      {
        TableView->SetHideYConcentration(dialog->IsEmpty());
        const auto it = std::find_if(parentModels.begin(), parentModels.end(),
          [&](const ParentAnalysisId& item) { return item.Title == _S(dialog->GetSelected()); });
        if (it == parentModels.end())
          return;
        AutoLayerConcentraion = dialog->IsAuto();
        LoadParentPlsAnalysis(it->Id);
        UncheckSampleCheckboxes();
      });
    dialog->Open();
  }

  void ViewPageAnalysisDataTablePage::SetupCheckBoxesState()
  {
    TableView->SetCheckBoxChecked();
    if (const auto root = GenesisContextRoot::Get())
    {
      if (const auto context = root->GetChildContextStepBasedMarkup())
      {
        if (const auto model = TableView->GetModel())
        {
          const auto checkedPeaks = context->GetData(ContextTagNames::checked_peaks);
          const auto checkedSamples = context->GetData(ContextTagNames::checked_samples);
          if (checkedPeaks.isValid()
            && !checkedPeaks.isNull()
            && checkedSamples.isValid()
            && !checkedSamples.isNull())
          {
            const auto& peakIds = DataTableModel->Get<IntVector>(ReferenceAnalysisDataTableModel::ValuesIds);
            const auto& sampleIds = DataTableModel->Get<IntVector>(ReferenceAnalysisDataTableModel::SampleDataIds);
            model->SetCheckedPeaks(CompareCurrentIdsWithChecked(peakIds, checkedPeaks.value<IntVector>()));
            model->SetCheckedSamples(CompareCurrentIdsWithChecked(sampleIds, checkedSamples.value<IntVector>()));
            TableView->UpdatePeakCheckBoxLabel();
            TableView->UpdateSampleCheckBoxLabel();
          }
          if (OpenedFrom == AnalysisType::PLS || OpenedFrom == AnalysisType::PLSPredict)
          {
            const auto concentrations = context->GetData(ContextTagNames::yConcentrations);
            if (concentrations.isValid() && !concentrations.isNull())
              model->SetConcentration(concentrations.value<std::vector<std::optional<double>>>());
          }
          if (OpenedFrom == AnalysisType::PLSPredict)
          {
            TableView->SetPeaksCheckBoxVisible(false);
          }
        }
      }
    }
  }

  void ViewPageAnalysisDataTablePage::UpdateCheckBoxesState(QString dataId, QVariant data)
  {
    if (!data.isValid() || data.isNull())
      return;

    if (const auto model = TableView->GetModel())
    {
      if (dataId == ContextTagNames::checked_peaks)
      {
        const auto& peakIds = DataTableModel->Get<IntVector>(ReferenceAnalysisDataTableModel::ValuesIds);
        model->SetCheckedPeaks(CompareCurrentIdsWithChecked(peakIds, data.value<IntVector>()));
        TableView->UpdatePeakCheckBoxLabel();
      }
      else if (dataId == ContextTagNames::checked_samples)
      {
        const auto& sampleIds = DataTableModel->Get<IntVector>(ReferenceAnalysisDataTableModel::SampleDataIds);
        model->SetCheckedSamples(CompareCurrentIdsWithChecked(sampleIds, data.value<IntVector>()));
        TableView->UpdateSampleCheckBoxLabel();
      }
      else if (dataId == ContextTagNames::yConcentrations)
      {
        if (OpenedFrom == AnalysisType::PLS)
          model->SetConcentration(data.value<std::vector<std::optional<double>>>());
      }
    }
  }

  void ViewPageAnalysisDataTablePage::LoadParentPlsAnalysis(size_t id)
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
          PlsModelForPredict = serializer->ToModel(json.value(JsonTagNames::Data).toObject());
          emit ParentModelLoaded();
        }
      },
      [](QNetworkReply*, QNetworkReply::NetworkError err)
      {
        Notification::NotifyError(tr("Error receive pls analysis model : %1").arg(err));
      });
  }

  void ViewPageAnalysisDataTablePage::SetupTableForPlsPredict()
  {
    TableView->SetPeaksCheckBoxVisible(false);
    if (const auto model = TableView->GetModel())
    {
      const auto& analysiTable = PlsModelForPredict->Get<AnalysisTable>(PlsAnalysisIdentityModel::AnalysisTable);
      const auto& peakIds = DataTableModel->Get<IntVector>(ReferenceAnalysisDataTableModel::ValuesIds);
      model->SetCheckedPeaks(CompareCurrentIdsWithChecked(peakIds, analysiTable.CheckedValues.MarkersIds));
      TableView->UpdatePeakCheckBoxLabel();
      TableView->UpdateSampleCheckBoxLabel();

      // in case auto by model
      if (AutoLayerConcentraion)
      {
        const auto params = PlsModelForPredict->Get<PlsParameters>(PlsAnalysisIdentityModel::PlsParameters);
        const auto names = DataTableModel->Get<StringList>(ReferenceAnalysisDataTableModel::LayerNames);
        const auto it = std::find_if(names.begin(), names.end(), [&](const std::string& name) { return name == params.LayerName; });
        if (it == names.end())
          return;
        std::vector<std::optional<double>> concentrations;
        SelectedLayerForPls = QString::fromStdString(*it);
        DataTableModel->Exec<OptionalDouble2DVector>(ReferenceAnalysisDataTableModel::LayerConcentrationData,
          [&](const OptionalDouble2DVector& data)
          {
            const auto index = std::distance(names.begin(), it);
            concentrations.reserve(data[index].size());
            std::copy(data[index].begin(), data[index].end(), std::back_inserter(concentrations));
          });
        model->SetConcentration(concentrations);
      }
    }
  }

  Constants::AnalysisType ViewPageAnalysisDataTablePage::GetTypeFromContext() const
  {
    if (const auto root = GenesisContextRoot::Get())
    {
      if (const auto context = root->GetChildContextStepBasedMarkup())
      {
        const auto buildedModel = context->GetData(ContextTagNames::analysis_builded_model);
        return (buildedModel.isValid() && !buildedModel.isNull())
          ? static_cast<AnalysisType>(buildedModel.toInt())
          : AnalysisType::NONE;
      }
    }
    return AnalysisType::NONE;
  }

  void ViewPageAnalysisDataTablePage::UncheckSampleCheckboxes()
  {
    if (const auto model = TableView->GetModel())
    {
      model->FillSamplesChecked(false);
      TableView->SetSampleState(Qt::Unchecked);
    }
  }

  std::map<int, QString> ViewPageAnalysisDataTablePage::GetTestingData() const
  {
    std::map<int, QString> info;
    const auto model = TableView->GetModel();
    const auto checkedSamples = model->GetCheckedSamples();
    const auto names = DataTableModel->Get<StringList>(ReferenceAnalysisDataTableModel::SampleData);
    const auto concentrations = model->GetConcentrations();
    const auto ids = DataTableModel->Get<IntVector>(ReferenceAnalysisDataTableModel::SampleDataIds);
    for (int i = 0; i < checkedSamples.size(); i++)
    {
      if (!checkedSamples[i])
        info.emplace(ids[i],
          QString("%1 (%2)")
          .arg(names[i].data())
          .arg(concentrations[i].has_value()
            ? QString::number(concentrations[i].value()) : ""));
    }
    return info;
  }

  void ViewPageAnalysisDataTablePage::SendPlsRequestData(const IntVector& testData,
    size_t samplesPerCount, size_t maxCountOfPc, bool autoscale, bool normalization,
    bool isSegment, bool isCross) const
  {
    SetPlsContextData(testData, samplesPerCount, maxCountOfPc, autoscale, normalization, isSegment, isCross);
    GenesisWindow::Get()->ShowPage(ViewPageNames::ViewPageProjectPLSGraphicsPlotPageName);
  }

  AnalysisTable ViewPageAnalysisDataTablePage::GetAnalysisTable() const
  {
    const auto model = TableView->GetModel();
    return
    {
      {
      GetChecked(model->GetCheckedPeaks(), DataTableModel->Get<IntVector>(ReferenceAnalysisDataTableModel::ValuesIds)),
      GetChecked(model->GetCheckedSamples(), DataTableModel->Get<IntVector>(ReferenceAnalysisDataTableModel::SampleDataIds)),
      },
      DataTableModel->Get<OptionalDouble2DVector>(ReferenceAnalysisDataTableModel::LayerConcentrationData)
    };
  }

  IntVector ViewPageAnalysisDataTablePage::GetChecked(const std::vector<bool>& checkedState, const IntVector& ids) const
  {
    if (ids.isEmpty())
    {
      Notification::NotifyError(tr("Values ids not found"), tr("Check data from backend"));
      return{};
    }
    IntVector checkedIds;
    checkedIds.reserve(checkedState.size());

    for (size_t i = 0; i < checkedState.size(); i++)
    {
      if (checkedState[i])
        checkedIds.append(ids[i]);
    }
    return checkedIds;
  }

  IntVector ViewPageAnalysisDataTablePage::GetUnChecked(const std::vector<bool>& checkedState, const Structures::IntVector& ids) const
  {
    IntVector checkedIds;
    checkedIds.reserve(checkedState.size());

    for (size_t i = 0; i < checkedState.size(); i++)
    {
      if (!checkedState[i])
        checkedIds.append(ids[i]);
    }
    return checkedIds;
  }

  std::vector<bool> ViewPageAnalysisDataTablePage::CompareCurrentIdsWithChecked(const IntVector& current, const IntVector& checked)
  {
    std::vector<bool> result(current.size(), false);
    for (qsizetype index = 0; index < current.size(); index++)
    {
      result[index] = checked.indexOf(current[index]) != -1;
    }
    return result;
  }

  IntVector ViewPageAnalysisDataTablePage::GetCheckedSamples() const
  {
    const auto model = TableView->GetModel();
    return GetChecked(model->GetCheckedSamples(), DataTableModel->Get<IntVector>(ReferenceAnalysisDataTableModel::SampleDataIds));
  }

  QStringList ViewPageAnalysisDataTablePage::GetCheckedSampleNames() const
  {
    const auto samples = GetCheckedSamples();
    const auto data = DataTableModel->Get<StringList>(ReferenceAnalysisDataTableModel::SampleData);
    QStringList names;
    names.reserve(samples.size());
    for (const auto& sample : samples)
      names.append(QString::fromStdString(data[sample]));
    return names;
  }

  IntVector ViewPageAnalysisDataTablePage::GetCheckedPeaks() const
  {
    const auto model = TableView->GetModel();
    return GetChecked(model->GetCheckedPeaks(), DataTableModel->Get<IntVector>(ReferenceAnalysisDataTableModel::ValuesIds));
  }

  IntVector ViewPageAnalysisDataTablePage::GetSampeIdsByNames(const QStringList& sampleNames) const
  {
    IntVector sampleIds;
    sampleIds.reserve(sampleNames.size());
    const auto sampleDataIds = DataTableModel->Get<IntVector>(ReferenceAnalysisDataTableModel::SampleDataIds);
    const auto sampleDataNames = DataTableModel->Get<StringList>(ReferenceAnalysisDataTableModel::SampleData);
    for (const auto& name : sampleNames)
    {
      const auto it = std::find_if(sampleDataNames.begin(), sampleDataNames.end(),
        [&](const std::string& dataName) { return name == QString::fromStdString(dataName); });
      if (it != sampleDataNames.end())
      {
        sampleIds.append(sampleDataIds[std::distance(sampleDataNames.begin(), it)]);
      }
    }
    return sampleIds;
  }

  IdentityModelPtr ViewPageAnalysisDataTablePage::GetPlsRequestModel(const Structures::IntVector& testData,
    size_t samplesPerCount, size_t maxCountOfPc,
    bool autoscale, bool normalization, bool isSegment, bool isCross) const
  {
    const auto predict = isCross
      ? isSegment
      ? "cv"
      : "cv_full"
      : "test";
    const auto plsRequestModel = std::make_shared<PlsFitRequestModel>();
    plsRequestModel->Set<bool>(PlsFitRequestModel::Autoscale, autoscale);
    plsRequestModel->Set<bool>(PlsFitRequestModel::Normdata, normalization);
    plsRequestModel->Set<IntVector>(PlsFitRequestModel::SampleIds, GetCheckedSamples());
    plsRequestModel->Set<IntVector>(PlsFitRequestModel::MarkerIds, GetCheckedPeaks());
    plsRequestModel->Set<std::string>(PlsFitRequestModel::Key, TableView->GetParameterType().toStdString());
    plsRequestModel->Set<std::string>(PlsFitRequestModel::Predict, predict);
    plsRequestModel->Set<size_t>(PlsFitRequestModel::CVCount, samplesPerCount);
    plsRequestModel->Set<size_t>(PlsFitRequestModel::NMaxLV, maxCountOfPc);
    plsRequestModel->Set<size_t>(PlsFitRequestModel::ParentId, 0);
    plsRequestModel->Set<int>(PlsFitRequestModel::TableId, TableId);
    plsRequestModel->Set<std::string>(PlsFitRequestModel::LayerName, SelectedLayerForPls.toStdString());
    const auto model = TableView->GetModel();
    const auto refmodel = model->GetModel();
    const auto sampleIds = refmodel->Get<IntVector>(ReferenceAnalysisDataTableModel::SampleDataIds);
    const auto concentrations = model->GetConcentrations();
    const auto checkedSamples = model->GetCheckedSamples();
    OptionalDoubleVector sampleConcentrations;
    sampleConcentrations.reserve(concentrations.size());

    for (int i = 0; i < checkedSamples.size(); i++)
    {
      if (checkedSamples[i] && concentrations[i].has_value())
        sampleConcentrations.push_back(concentrations[i].value());
    }
    plsRequestModel->Set(PlsFitRequestModel::SampleConcentrations, sampleConcentrations);

    DoubleVector testConcentrations;
    testConcentrations.reserve(testData.size());
    for (const auto& i : testData)
    {
      if (concentrations[sampleIds[i]].has_value())
        testConcentrations.push_back(concentrations[sampleIds[i]].value());
    }

    plsRequestModel->Set(PlsFitRequestModel::TestSampleIds, testData);
    plsRequestModel->Set(PlsFitRequestModel::TestSampleConcentrations, testConcentrations);

    IntVector possibleIds;
    possibleIds.reserve(concentrations.size() - testConcentrations.size() - sampleConcentrations.size());
    for (int i = 0; i < sampleIds.size(); i++)
    {
      if (!checkedSamples[i])
      {
        const auto it = std::find(testData.begin(), testData.end(), sampleIds[i]);
        if (it == testData.end())
          possibleIds.push_back(sampleIds[i]);
      }
    }
    plsRequestModel->Set(PlsFitRequestModel::PossibleTestSampleIds, possibleIds);

    DoubleVector possibleConcentrations;
    possibleConcentrations.reserve(possibleIds.size());
    for (int i = 0; i < sampleIds.size(); i++)
    {
      if (!checkedSamples[i])
      {
        const auto it = std::find(testData.begin(), testData.end(), sampleIds[i]);
        if (it == testData.end() && concentrations[i].has_value())
          possibleConcentrations.push_back(concentrations[i].value());
      }
    }

    plsRequestModel->Set(PlsFitRequestModel::PossibleTestSampleConcentrations, possibleConcentrations);
    return plsRequestModel;
  }

  void ViewPageAnalysisDataTablePage::SetPlsContextData(const IntVector& testData, size_t samplesPerCount, size_t maxCountOfPc, bool autoscale, bool normalization, bool isSegment, bool isCross) const
  {
    const auto contextRoot = GenesisContextRoot::Get();
    if (!contextRoot)
      return;

    if (auto plsContext = contextRoot->GetContextMarkupVersionAnalisysPLS())
    {
      const auto model = TableView->GetModel();
      const auto refmodel = model->GetModel();
      const auto sampleIds = refmodel->Get<IntVector>(ReferenceAnalysisDataTableModel::SampleDataIds);
      const auto checkedSamples = model->GetCheckedSamples();
      const auto concentrations = model->GetConcentrations();

      OptionalDoubleVector sampleConcentrations;
      sampleConcentrations.reserve(concentrations.size());
      for (int i = 0; i < checkedSamples.size(); i++)
      {
        if (checkedSamples[i] && concentrations[i].has_value())
          sampleConcentrations.push_back(concentrations[i].value());
      }

      DoubleVector testConcentrations;
      testConcentrations.reserve(testData.size());
      for (const auto& i : testData)
      {
        if (concentrations[sampleIds[i]].has_value())
          testConcentrations.push_back(concentrations[sampleIds[i]].value());
      }

      IntVector possibleIds;
      possibleIds.reserve(concentrations.size() - testConcentrations.size() - sampleConcentrations.size());
      for (int i = 0; i < sampleIds.size(); i++)
      {
        if (!checkedSamples[i])
        {
          const auto it = std::find(testData.begin(), testData.end(), sampleIds[i]);
          if (it == testData.end())
            possibleIds.push_back(sampleIds[i]);
        }
      }

      plsContext->SetData(
        {
          { JsonTagNames::analysis_id, -1 },
          { JsonTagNames::TableId, TableId },
          { JsonTagNames::AnalysisType, static_cast<int>(AnalysisType::PLS) },
          { JsonTagNames::ValueType, TableView->GetParameterType() },
          { JsonTagNames::AnalysisTable, QVariant::fromValue<AnalysisTable>(GetAnalysisTable()) },
          { JsonTagNames::Normdata, normalization },
          { JsonTagNames::Autoscale, autoscale },
          { JsonTagNames::AnalysisSamplesCount, static_cast<int>(samplesPerCount) },
          { JsonTagNames::IsSegment, isSegment },
          { JsonTagNames::NMaxLV, static_cast<int>(maxCountOfPc) },
          { JsonTagNames::PC95_99, false },
          { JsonTagNames::TestData, QVariant::fromValue<IntVector>(testData) },
          { JsonTagNames::IsCross, isCross },
          { JsonTagNames::AnalysisParentId, 0 },
          { JsonTagNames::TableId, TableId },
          { JsonTagNames::LayerName, SelectedLayerForPls },
          { JsonTagNames::SamplesConcentrations, QVariant::fromValue<OptionalDoubleVector>(sampleConcentrations) },
          { JsonTagNames::TestSampleConcentrations, QVariant::fromValue<DoubleVector>(testConcentrations) },
          { JsonTagNames::PossibleTestSampleIds, QVariant::fromValue<IntVector>(possibleIds) },
        });
    }
  }
}
