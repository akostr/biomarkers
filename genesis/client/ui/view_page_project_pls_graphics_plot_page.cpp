#include "view_page_project_pls_graphics_plot_page.h"

#include <qlabel.h>
#include <QRegExp>

#include "genesis_style/style.h"
#include "logic/tree_model.h"
#include "plots/load_plot.h"
#include "logic/known_context_names.h"
#include "logic/known_context_tag_names.h"
#include "logic/known_json_tag_names.h"
#include "logic/models/pls_fit_response_model.h"
#include "logic/models/pls_analysis_identity_model.h"
#include "logic/service/ijson_serializer.h"
#include "logic/service/service_locator.h"
#include "logic/enums.h"
#include "widgets/pls_count_plot_widget.h"
#include "widgets/tab_widget.h"
#include "widgets/pls_load_plot_widget.h"
#include "widgets/pls_slope_offset_plot_widget.h"
#include "widgets/pls_coefficient_plot_widget.h"
#include "genesis_window.h"
#include "logic/notification.h"
#include "widgets/pls_rmse_plot_widget.h"
#include "widgets/pls_dispersion_plot_widget.h"
#include "widgets/model_settings_widget.h"
#include "api/api_rest.h"
#include "logic/context_root.h"
#include "contexts/new_model_settings_context.h"
#include "contexts/exist_model_settings_context.h"
#include "dialogs/web_dialog_pls_build_settings_refactor.h"
#include "ui/known_view_names.h"
#include "ui/dialogs/templates/save_edit.h"
#include "ui/dialogs/export_tables_and_plots_dialog.h"
#include "ui/dialogs/templates/confirm.h"
#include "logic/models/pls_fit_request_model.h"

namespace
{
  const static std::string CV_FULL = "cv_full";
  const static std::string CV = "cv";
  const static std::string TEST = "TEST";
  const static int MinimumCount = 3;
}

namespace Views
{
  using namespace Widgets;
  using namespace Names;
  using namespace ViewPageNames;
  using namespace Service;
  using namespace Widgets;
  using namespace Core;
  using namespace Constants;

  ViewPageProjectPlsGraphicsPlotPage::ViewPageProjectPlsGraphicsPlotPage(QWidget* parent)
    : View(parent)
  {
    SetupUi();
    ConnectSignals();
    ConnectMenuSignals();
  }

  ViewPageProjectPlsGraphicsPlotPage::~ViewPageProjectPlsGraphicsPlotPage()
  {
    PlotWidgets.clear();
  }

  void ViewPageProjectPlsGraphicsPlotPage::ApplyContextMarkupVersionAnalysisPLS(const QString& dataId, const QVariant& data)
  {
    if (isDataReset(dataId, data))
    {
      auto context = GenesisContextRoot::Get()->GetContextMarkupVersionAnalisysPLS();
      if (context->GetClass() != GenesisContextNames::MarkupVersionAnalysisPLS)
        return;
      AnalysisTypeId = static_cast<size_t>(context->GetData(JsonTagNames::AnalysisType).toInt());
      AnalysisId = -1;
      auto idValue = context->GetData(ContextTagNames::AnalysisId);
      auto tableIdValue = context->GetData(JsonTagNames::TableId);
      if (idValue.isValid() && !idValue.isNull())
      {
        AnalysisId = idValue.toInt();
        if (AnalysisId > 0)
        {
          // emit plsResponseModelLoaded when get all data from backend
          LoadResponseModelFromServer(AnalysisId);
        }
      }
      else if (tableIdValue.isValid() && !tableIdValue.isNull())
      {
        TableId = tableIdValue.toInt();
        Key = _S(context->GetData(JsonTagNames::ValueType).toString());
        SamplesPerCount = context->GetData(JsonTagNames::AnalysisSamplesCount).toInt();
        auto request = CreateRequestAnalysisFromContext(context);
        RequestPlsFit(request);
      }
    }
    if (dataId == ContextTagNames::AnalysisId)
    {
      AnalysisId = data.toInt();
      LoadResponseModelFromServer(AnalysisId);
    }
  }

  void ViewPageProjectPlsGraphicsPlotPage::SetupUi()
  {
    //// Self
    setStyleSheet(Style::Genesis::GetUiStyle());

    //// Layout
    const auto layout = new QVBoxLayout(this);

    layout->setContentsMargins(Style::Scale(40), Style::Scale(32), Style::Scale(40), Style::Scale(32));
    layout->setSpacing(0);

    auto toolBarLayout = new QHBoxLayout;
    toolBarLayout->setContentsMargins(0, 0, 0, 0);
    auto caption = new QLabel(tr("Current PLC"));
    caption->setStyleSheet(Style::Genesis::GetH1());
    toolBarLayout->addWidget(caption);

    toolBarLayout->addStretch();

    RecalcButton = new QPushButton(tr("Recalculate model"));
    RecalcButton->setProperty("secondary", true);
    RecalcButton->setVisible(false);

    toolBarLayout->addWidget(RecalcButton);
    toolBarLayout->addWidget(new ModelSettingsWidget(this));

    layout->addLayout(toolBarLayout);
    auto tabContainer = new QWidget(this);
    tabContainer->setProperty("style", "white_base");
    tabContainer->setLayout(new QHBoxLayout());
    tabContainer->layout()->setContentsMargins(26, 26, 26, 26);
    layout->addWidget(tabContainer);

    const auto plotTabWidget = new TabWidget(tabContainer);
    tabContainer->layout()->addWidget(plotTabWidget);

    SlopeOffsetMultiplePlotWidget = new PlsMultiplePlotWidget(this);
    SlopeOffsetPlotWidget = new PlsSlopeOffsetPlotWidget(this);
    CoefficientPlotWidget = new PlsCoefficientPlotWidget(this);
    SlopeOffsetMultiplePlotWidget->PlsAppendPlots(
      {
        SlopeOffsetPlotWidget,
        CoefficientPlotWidget
      });

    CountPlotWidget = new PlsCountPlotWidget(this);
    LoadPlotWidget = new PlsLoadPlotWidget(this);
    RMSEPlotWidget = new PlsMultiplePlotWidget(this);
    RMSEPlotWidget->PlsAppendPlots(
      {
        new PlsRMSEPlotWidget(this),
        new PlsDispersionPlotWidget(this),
      });
    RMSEPlotWidget->SetLabelText(tr("PC count for prediction"));
    PlotWidgets =
    {
      { tr("Slope-offset and regression plot"), SlopeOffsetMultiplePlotWidget },
      { tr("Counts plot"), CountPlotWidget },
      { tr("Load plot"), LoadPlotWidget },
      { tr("RMSE and dispersion plot"), RMSEPlotWidget },
    };

    for (const auto& [name, widget] : PlotWidgets)
      plotTabWidget->AddTabWidget(widget, name);
  }

  void ViewPageProjectPlsGraphicsPlotPage::ConnectSignals()
  {
    connect(RecalcButton, &QPushButton::clicked, this, &ViewPageProjectPlsGraphicsPlotPage::RecalcModel);

    connect(this, &ViewPageProjectPlsGraphicsPlotPage::PlsResponseModelLoaded,
      this, &ViewPageProjectPlsGraphicsPlotPage::LoadGraphicsFromResponseModel);

    connect(SlopeOffsetMultiplePlotWidget, &PlsMultiplePlotWidget::PCMaxCountValueChanged, CountPlotWidget, &PlsCountPlotWidget::SetPCMaxCount);
    connect(SlopeOffsetMultiplePlotWidget, &PlsMultiplePlotWidget::PCMaxCountValueChanged, LoadPlotWidget, &PlsLoadPlotWidget::SetPCMaxCount);

    connect(SlopeOffsetPlotWidget, &PlsSlopeOffsetPlotWidget::itemsExcluded, CountPlotWidget, &PlsCountPlotWidget::excludeItems);
    connect(SlopeOffsetPlotWidget, &PlsSlopeOffsetPlotWidget::itemsExcluded, this, &ViewPageProjectPlsGraphicsPlotPage::MoveTitleToPossible);
    connect(SlopeOffsetPlotWidget, &PlsSlopeOffsetPlotWidget::excludedItemsReset, CountPlotWidget, &PlsCountPlotWidget::resetExcludeItems);
    connect(SlopeOffsetPlotWidget, &PlsSlopeOffsetPlotWidget::excludedItemsReset, this, &ViewPageProjectPlsGraphicsPlotPage::ResetPossible);

    connect(CountPlotWidget, &PlsCountPlotWidget::itemsExcluded, SlopeOffsetPlotWidget, &PlsSlopeOffsetPlotWidget::excludeItems);
    connect(CountPlotWidget, &PlsCountPlotWidget::itemsExcluded, this, &ViewPageProjectPlsGraphicsPlotPage::MoveTitleToPossible);
    connect(CountPlotWidget, &PlsCountPlotWidget::excludedItemsReset, SlopeOffsetPlotWidget, &PlsSlopeOffsetPlotWidget::resetExcludeItems);
    connect(CountPlotWidget, &PlsCountPlotWidget::excludedItemsReset, this, &ViewPageProjectPlsGraphicsPlotPage::ResetPossible);
    connect(CountPlotWidget, &PlsCountPlotWidget::excludedStateChanged, this, &ViewPageProjectPlsGraphicsPlotPage::UpdateRecalcButtonVisibleState);
    connect(CountPlotWidget, &PlsCountPlotWidget::AxisChanged, LoadPlotWidget, &PlsLoadPlotWidget::SetAxisForPC);

    connect(LoadPlotWidget, &PlsLoadPlotWidget::excludedStateChanged, this, &ViewPageProjectPlsGraphicsPlotPage::UpdateRecalcButtonVisibleState);
    connect(LoadPlotWidget, &PlsLoadPlotWidget::excludedStateChanged, this, &ViewPageProjectPlsGraphicsPlotPage::CancelExcludePeaksCoefficientPlot);
    connect(LoadPlotWidget, &PlsLoadPlotWidget::AllItemsExcluded, this, &ViewPageProjectPlsGraphicsPlotPage::SyncExcludedPeaksWithCoefficientPlot);
    connect(LoadPlotWidget, &PlsLoadPlotWidget::AxisChanged, CountPlotWidget, &PlsCountPlotWidget::SetAxisForPC);
    connect(LoadPlotWidget, &PlsLoadPlotWidget::ResetExcludedPeaksNumbers, this, &ViewPageProjectPlsGraphicsPlotPage::ResetExcludePeaksCoefficientPlot);

    connect(CoefficientPlotWidget, &PlsCoefficientPlotWidget::OnPeaksExclude, this, &ViewPageProjectPlsGraphicsPlotPage::SyncPeaksStateWithLoadPlot);
    connect(this, &ViewPageProjectPlsGraphicsPlotPage::PlsModelSavingFinished, this, []() { TreeModel::ResetInstances("TreeModelDynamicProjectModelsList"); });
  }

  void ViewPageProjectPlsGraphicsPlotPage::ConnectMenuSignals()
  {
    connect(MenuContext, &AnalysisModelSettingsContext::OpenDataTable, this, &ViewPageProjectPlsGraphicsPlotPage::OpenDataTable);
    connect(MenuContext, &AnalysisModelSettingsContext::OpenModelParameters, this, &ViewPageProjectPlsGraphicsPlotPage::OpenModelParameters);
    connect(MenuContext, &AnalysisModelSettingsContext::OpenMarkup, this, &ViewPageProjectPlsGraphicsPlotPage::OpenMarkup);
    connect(MenuContext, &AnalysisModelSettingsContext::Export, this, &ViewPageProjectPlsGraphicsPlotPage::Export);
  }

  void ViewPageProjectPlsGraphicsPlotPage::DisconnectSignals()
  {
    disconnect(MenuContext, &AnalysisModelSettingsContext::OpenDataTable, this, &ViewPageProjectPlsGraphicsPlotPage::OpenDataTable);
    disconnect(MenuContext, &AnalysisModelSettingsContext::OpenModelParameters, this, &ViewPageProjectPlsGraphicsPlotPage::OpenModelParameters);
    disconnect(MenuContext, &AnalysisModelSettingsContext::OpenMarkup, this, &ViewPageProjectPlsGraphicsPlotPage::OpenMarkup);
    disconnect(MenuContext, &AnalysisModelSettingsContext::Export, this, &ViewPageProjectPlsGraphicsPlotPage::Export);
  }

  void ViewPageProjectPlsGraphicsPlotPage::SavePlsModel()
  {
    Dialogs::Templates::SaveEdit::Settings s;
    s.header = tr("PLS model saving");
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
        model->Set<std::string>(PlsAnalysisIdentityModel::AnalysisTitle, _S(dialog->getTitle()));
        model->Set<std::string>(PlsAnalysisIdentityModel::AnalysisComment, _S(dialog->getComment()));
        SaveNewModelToServer(model);
      });
    dialog->Open();
  }

  void ViewPageProjectPlsGraphicsPlotPage::SaveChangesPlsModel()
  {
    if (PlsAnalysisModel)
    {
      UpdateParametersFromResponse();
      UpdateModelOnServer(PlsAnalysisModel);
    }
  }

  IdentityModelPtr ViewPageProjectPlsGraphicsPlotPage::GetPlsModel()
  {
    return PlsAnalysisModel;
  }

  void ViewPageProjectPlsGraphicsPlotPage::CreatePlsAnalysisIdentityModel()
  {
    QString title;
    QString comment;
    if (PlsAnalysisModel)
    {
      title = QString::fromStdString(PlsAnalysisModel->Get<std::string>(PlsAnalysisIdentityModel::AnalysisTitle));
      comment = QString::fromStdString(PlsAnalysisModel->Get<std::string>(PlsAnalysisIdentityModel::AnalysisComment));
    }
    PlsAnalysisModel = std::make_shared<PlsAnalysisIdentityModel>();
    PlsAnalysisModel->Set<size_t>(PlsAnalysisIdentityModel::AnalysisTypeId, AnalysisTypeId);
    PlsAnalysisModel->Set<size_t>(PlsAnalysisIdentityModel::AnalysisId, AnalysisId);
    PlsAnalysisModel->Set<size_t>(PlsAnalysisIdentityModel::AnalysisTableId, static_cast<size_t>(TableId));
    PlsAnalysisModel->Set<Parameters>(PlsAnalysisIdentityModel::AnalysisParameters, { Key });
    PlsAnalysisModel->Set(PlsAnalysisIdentityModel::ExportRequestBody, PlsResponseModel->Get<QByteArray>(PlsFitResponseModel::ExportRequestBody));
    PlsAnalysisModel->Set<AnalysisTable>(PlsAnalysisIdentityModel::AnalysisTable, PlsResponseModel->Get<AnalysisTable>(AnalysisIdentityModel::AnalysisTable));
    UpdateParametersFromResponse();
    PlsAnalysisModel->Set<AnalysisData>(PlsAnalysisIdentityModel::AnalysisData, CreateAnalysisData());
    PlsAnalysisModel->Set<PlsFitResponse>(PlsAnalysisIdentityModel::AnalysisData, CreateFitResponse());
    PlsAnalysisModel->Set<ByteArray>(PlsAnalysisIdentityModel::PlsFitPickle, PlsResponseModel->Get<ByteArray>(PlsFitResponseModel::BinaryModel));

    PlsAnalysisModel->Set<std::string>(PlsAnalysisIdentityModel::AnalysisTitle, title.toStdString());
    PlsAnalysisModel->Set<std::string>(PlsAnalysisIdentityModel::AnalysisComment, comment.toStdString());
  }

  void ViewPageProjectPlsGraphicsPlotPage::UpdateParametersFromResponse() const
  {
    PlsAnalysisModel->Set<PlsParameters>(PlsAnalysisIdentityModel::PlsParameters,
      {
        PlsResponseModel->Get<std::string>(PlsFitResponseModel::Predict),
        PlsResponseModel->Get<std::string>(PlsFitResponseModel::LayerName),
        PlsResponseModel->Get<size_t>(PlsFitResponseModel::DefaultNumberPC),
        PlsResponseModel->Get<size_t>(PlsFitResponseModel::ChosenNumberPC),
        PlsResponseModel->Get<size_t>(PlsFitResponseModel::SamplesPerCount),
        PlsResponseModel->Get<size_t>(PlsFitResponseModel::nMaxLv),
        PlsResponseModel->Get<bool>(PlsFitResponseModel::Autoscale),
        PlsResponseModel->Get<bool>(PlsFitResponseModel::Normdata),
      }
      );
  }

  AnalysisData ViewPageProjectPlsGraphicsPlotPage::CreateAnalysisData() const
  {
    return
    {
      CreateAnalysisInnerData(),

    };
  }

  AnalysisInnerData ViewPageProjectPlsGraphicsPlotPage::CreateAnalysisInnerData() const
  {
    return
    {
      PlsResponseModel->Get<AnalysisMetaInfoList>(PlsFitResponseModel::T_TitleGrad),
      PlsResponseModel->Get<AnalysisMetaInfoList>(PlsFitResponseModel::T_TitlePossible),
      {},  //T_Title_expanded
      {}, // Analysis components
      PlsResponseModel->Get<IntVector>(PlsFitResponseModel::P_id),
      PlsResponseModel->Get<StringList>(PlsFitResponseModel::P_Title)
    };
  }

  std::map<int, QString> ViewPageProjectPlsGraphicsPlotPage::GetTestData() const
  {
    const auto peaks = PlsResponseModel->Get<AnalysisMetaInfoList>(PlsFitResponseModel::T_TitlePossible);
    std::map<int, QString> data;
    std::transform(peaks.begin(), peaks.end(), std::inserter(data, data.end()),
      [&](const AnalysisMetaInfo& info)
      {
        return std::make_pair(info.SampleId, QString::fromStdString(info.FileName));
      });
    return data;
  }

  IntVector ViewPageProjectPlsGraphicsPlotPage::GetCheckedSamplesId() const
  {
    const auto samples = CountPlotWidget->GetExcludedSampleIds();

    IntVector checkedSampleIds;
    PlsAnalysisModel->Exec<AnalysisData>(PlsAnalysisIdentityModel::AnalysisData,
      [&](const AnalysisData& data)
      {
        checkedSampleIds.reserve(data.Data.T_Title.size() - samples.size());
        for (const auto& info : data.Data.T_Title)
        {
          const auto foundIt = std::find(samples.begin(), samples.end(), info.SampleId);
          if (foundIt == samples.end())
            checkedSampleIds.append(info.SampleId);
        }
      });

    return checkedSampleIds;
  }

  IntVector ViewPageProjectPlsGraphicsPlotPage::GetCheckedPeaks() const
  {
    const auto excludedPeaks = LoadPlotWidget->GetExcludedPeaks();

    const auto& data = PlsAnalysisModel->Get<AnalysisData>(PlsAnalysisIdentityModel::AnalysisData);
    IntVector checkedPeaks;
    checkedPeaks.reserve(data.Data.P_Id.size());
    for (size_t index = 0; index < data.Data.P_Title.size(); index++)
    {
      if (!excludedPeaks.contains(QString::fromStdString(data.Data.P_Title[index])))
        checkedPeaks.append(data.Data.P_Id[index]);
    }
    return checkedPeaks;
  }

  IntVector ViewPageProjectPlsGraphicsPlotPage::GetTestIds(const QStringList& names) const
  {
    IntVector testData;
    testData.reserve(names.size());
    PlsResponseModel->Exec<AnalysisMetaInfoList>(PlsFitResponseModel::T_TitlePossible,
      [&](const AnalysisMetaInfoList& list)
      {
        for (const auto& name : names)
        {
          const auto it = std::find_if(list.begin(), list.end(),
            [&](const AnalysisMetaInfo& info)
            {
              return QString::fromStdString(info.FileName).contains(name);
            });
          if (it != list.end())
            testData.push_back(it->SampleId);
        }
      });
    return testData;
  }

  void ViewPageProjectPlsGraphicsPlotPage::SwitchToPage(const QString& pageId)
  {
    auto model = PlsAnalysisModel;
    const auto isExist = static_cast<int>(model->Get<size_t>(PlsAnalysisIdentityModel::AnalysisId)) > 0;
    auto dial = Dialogs::Templates::Confirm::confirm(GenesisWindow::Get(),
      !isExist
      ? tr("Save current model?")
      : tr("Save changes in current model?"),
      [this, pageId, isExist]()
      {
        connect(this, &ViewPageProjectPlsGraphicsPlotPage::PlsModelSavingFinished, this,
          [this, pageId](bool error)
          {
            if (!error)
              View::SwitchToPage(pageId);
            else
              View::RejectPageSwitching();
          }, Qt::SingleShotConnection);

        if (isExist)
          SaveChangesPlsModel();
        else
          SavePlsModel();
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

  void ViewPageProjectPlsGraphicsPlotPage::SwitchToModule(int module)
  {
    auto model = PlsAnalysisModel;
    const auto isExist = static_cast<int>(model->Get<size_t>(PlsAnalysisIdentityModel::AnalysisId)) > 0;
    auto dial = Dialogs::Templates::Confirm::confirm(GenesisWindow::Get(),
      !isExist
      ? tr("Save current model?")
      : tr("Save changes in current model?"),
      [this, module, isExist]()
      {
        connect(this, &ViewPageProjectPlsGraphicsPlotPage::PlsModelSavingFinished, this,
          [this, module](bool error)
          {
            if (!error)
              View::SwitchToModule(module);
            else
              View::RejectModuleSwitching();
          }, Qt::SingleShotConnection);

        if (isExist)
          SaveChangesPlsModel();
        else
          SavePlsModel();
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

  void ViewPageProjectPlsGraphicsPlotPage::LogOut()
  {
    auto model = PlsAnalysisModel;
    const auto isExist = static_cast<int>(model->Get<size_t>(PlsAnalysisIdentityModel::AnalysisId)) > 0;
    auto dial = Dialogs::Templates::Confirm::confirm(GenesisWindow::Get(),
      !isExist
      ? tr("Save current model?")
      : tr("Save changes in current model?"),
      [this, isExist]()
      {
        connect(this, &ViewPageProjectPlsGraphicsPlotPage::PlsModelSavingFinished, this,
          [this](bool error)
          {
            if (!error)
              View::LogOut();
            else
              View::RejectLoggingOut();
          }, Qt::SingleShotConnection);

        if (isExist)
          SaveChangesPlsModel();
        else
          SavePlsModel();
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

  void ViewPageProjectPlsGraphicsPlotPage::SaveNewModelToServer(IdentityModelPtr plsModel)
  {
    auto overlayId = GenesisWindow::Get()->ShowOverlay(tr("Saving data..."));
    QScopeGuard guard([overlayId]() { GenesisWindow::Get()->RemoveOverlay(overlayId); });

    const auto serializer = ServiceLocator::Instance().Resolve<PlsAnalysisIdentityModel, IJsonSerializer>();
    assert(serializer);
    const auto jsonModel = serializer->ToJson(plsModel);
    API::REST::SaveAnalysis(jsonModel,
      [&](QNetworkReply*, QJsonDocument doc)
      {
        if (const auto jsonResponse = doc.object();
          jsonResponse.value(JsonTagNames::Error).toBool())
        {
          Notification::NotifyError(tr("Error : %1 while saving.").arg(jsonResponse.value(JsonTagNames::Status).toString()));
          emit PlsModelSavingFinished(true);
        }
        else
        {
          const auto analysis_id = static_cast<size_t>(jsonResponse.value(JsonTagNames::analysis_id).toInt());
          PlsAnalysisModel->Set<size_t>(PlsAnalysisIdentityModel::AnalysisId, analysis_id);
          UpdateModelMenu();
          Notification::NotifySuccess(tr("Model Pls was saved."));
          emit PlsModelSavingFinished(false);
        }
      },
      [this](QNetworkReply*, QNetworkReply::NetworkError error)
      {
        // qDebug() << error;
        Notification::NotifyError(tr("Error : %1 while saving.").arg(error));
        emit PlsModelSavingFinished(true);
      });
  }

  void ViewPageProjectPlsGraphicsPlotPage::UpdateModelOnServer(IdentityModelPtr plsModel)
  {
    auto overlayId = GenesisWindow::Get()->ShowOverlay(tr("Saving data..."));
    QScopeGuard guard([overlayId]() { GenesisWindow::Get()->RemoveOverlay(overlayId); });

    const auto serializer = ServiceLocator::Instance().Resolve<PlsAnalysisIdentityModel, IJsonSerializer>();
    assert(serializer);
    const auto jsonModel = serializer->ToJson(plsModel);
    API::REST::SaveAnalysis(jsonModel, [&](QNetworkReply*, QJsonDocument doc)
      {
        if (const auto jsonResponse = doc.object();
          jsonResponse.value(JsonTagNames::Error).toBool())
        {
          Notification::NotifyError(tr("Error : %1 while saving.").arg(jsonResponse.value(JsonTagNames::Status).toString()));
          emit PlsModelSavingFinished(true);
        }
        else
        {
          Notification::NotifySuccess(tr("Model Pls was updated."));
          emit PlsModelSavingFinished(false);
        }
      },
      [this](QNetworkReply*, QNetworkReply::NetworkError error)
      {
        Notification::NotifyError(tr("Error : %1 while saving.").arg(error));
        emit PlsModelSavingFinished(true);
      });
  }

  void ViewPageProjectPlsGraphicsPlotPage::UpdateRecalcButtonVisibleState(bool hasExcluded)
  {
    if (CountPlotWidget)
    {
      hasExcluded |= CountPlotWidget->HasExcludedItems();
    }

    if (LoadPlotWidget)
    {
      hasExcluded |= LoadPlotWidget->HasExcludedItems();
    }
    RecalcButton->setVisible(hasExcluded);
  }

  void ViewPageProjectPlsGraphicsPlotPage::CancelExcludePeaksCoefficientPlot(bool hasExcluded)
  {
    if (!hasExcluded && CoefficientPlotWidget)
      CoefficientPlotWidget->ClearExlcudePeaks();
  }

  void ViewPageProjectPlsGraphicsPlotPage::ResetExcludePeaksCoefficientPlot(const std::vector<int>& peaks)
  {
    if (CoefficientPlotWidget)
      CoefficientPlotWidget->CancelExcludePeaks(peaks);
  }

  void ViewPageProjectPlsGraphicsPlotPage::SyncExcludedPeaksWithCoefficientPlot()
  {
    if (LoadPlotWidget)
    {
      const auto peaksNames = LoadPlotWidget->GetExcludedPeaks();
      std::vector<int> excludedPeaks;
      excludedPeaks.reserve(peaksNames.size());
      PlsAnalysisModel->Exec<AnalysisData>(PlsAnalysisIdentityModel::AnalysisData,
        [&](const AnalysisData& data)
        {
          for (size_t index = 0; index < data.Data.P_Title.size(); index++)
          {
            if (peaksNames.contains(QString::fromStdString(data.Data.P_Title[index])))
              excludedPeaks.push_back(index + 1); // first start from 1
          }
        });

      if (CoefficientPlotWidget)
        CoefficientPlotWidget->AddExcludedPeaks(excludedPeaks);
    }
  }

  void ViewPageProjectPlsGraphicsPlotPage::SyncPeaksStateWithLoadPlot()
  {
    if (CoefficientPlotWidget)
    {
      if (LoadPlotWidget)
      {
        const auto peaks = CoefficientPlotWidget->GetExcludedPeaks();
        if (peaks.isEmpty())
          LoadPlotWidget->CancelExclude();
        else
          LoadPlotWidget->ExcludePeaks(peaks);

        UpdateRecalcButtonVisibleState(!peaks.isEmpty());
      }
    }
  }

  void ViewPageProjectPlsGraphicsPlotPage::OpenDataTable()
  {
    RemoveExcludedSamples();
    RemoveExcludedPeaks();
    if (auto contextRoot = GenesisContextRoot::Get())
    {
      if (const auto markupContext = contextRoot->GetChildContextStepBasedMarkup())
      {
        const auto tableId = PlsAnalysisModel->Get<size_t>(AnalysisIdentityModel::AnalysisTableId);
        markupContext->SetData(ContextTagNames::ReferenceId, static_cast<int>(tableId));
        markupContext->SetData(ContextTagNames::analysis_builded_model, static_cast<int>(AnalysisType::PLS));
        const auto& analysisTable = PlsAnalysisModel->Get<AnalysisTable>(AnalysisIdentityModel::AnalysisTable);
        markupContext->SetData(ContextTagNames::checked_peaks, QVariant::fromValue<IntVector>(analysisTable.CheckedValues.MarkersIds));
        markupContext->SetData(ContextTagNames::checked_samples, QVariant::fromValue<IntVector>(analysisTable.CheckedValues.SamplesIds));
        const auto data = PlsAnalysisModel->Get<AnalysisData>(PlsAnalysisIdentityModel::AnalysisData);
        std::vector<std::optional<double>> concentrations;
        concentrations.reserve(data.Data.T_Title.size());
        std::transform(data.Data.T_Title.begin(), data.Data.T_Title.end(), std::back_inserter(concentrations),
          [](const AnalysisMetaInfo& info) { return info.YConcentration; });
        PlsAnalysisModel->Exec<PlsFitResponse>(PlsAnalysisIdentityModel::AnalysisData,
          [&](const PlsFitResponse& params)
          {
            int offset = 0;
            for (const auto& id : params.PossibleTestSamplesIds)
            {
              const auto lower = std::lower_bound(analysisTable.CheckedValues.SamplesIds.begin(), analysisTable.CheckedValues.SamplesIds.end(), id);
              if (lower == analysisTable.CheckedValues.SamplesIds.end())
                continue;
              const auto index = std::distance(analysisTable.CheckedValues.SamplesIds.begin(), lower);
              if (params.PossibleTestSamplesConcentrations.size() > index)
                concentrations.insert(concentrations.begin() + index, params.PossibleTestSamplesConcentrations[index]);
              else
                concentrations.insert(concentrations.begin() + index, std::nullopt);
              offset++;
            }
          });
        markupContext->SetData(ContextTagNames::yConcentrations, QVariant::fromValue(concentrations));
        markupContext->SetData(ContextTagNames::AnalysisId,
          static_cast<int>(PlsAnalysisModel->Get<size_t>(PlsAnalysisIdentityModel::AnalysisId)));
      }
      GenesisWindow::Get()->ShowPage(ViewPageNames::ViewPageAnalysisDataTablePageName);
    }
  }

  void ViewPageProjectPlsGraphicsPlotPage::OpenModelParameters()
  {
    const auto maxPC = std::min(
      PlsResponseModel->Get<AnalysisMetaInfoList>(PlsFitResponseModel::T_TitleGrad).size(),
      PlsResponseModel->Get<StringList>(PlsFitResponseModel::P_Title).size()
    );
    const auto dialog = new WebDialogPLSBuildSettingsRefactor(this);
    dialog->SetMaxPCCount(maxPC);
    dialog->SetMaxSamplesPerSegmentCount(maxPC - 1);
    dialog->SetAutoscale(PlsResponseModel->Get<bool>(PlsFitResponseModel::Autoscale));
    dialog->SetNormalization(PlsResponseModel->Get<bool>(PlsFitResponseModel::Normdata));
    dialog->SetSamplesPerSegmentCount(PlsResponseModel->Get<size_t>(PlsFitResponseModel::SamplesPerCount));
    dialog->SetPCValue(std::min(maxPC, PlsResponseModel->Get<size_t>(PlsFitResponseModel::nMaxLv)));
    const auto predict = PlsResponseModel->Get<std::string>(PlsFitResponseModel::Predict);
    dialog->SetIsSegment(predict == CV);
    dialog->SetIsCross(predict != TEST);
    dialog->SetTestData(GetTestData());

    dialog->Open();
    connect(dialog, &WebDialog::Accepted, this,
      [&, dialog]()
      {
        SamplesPerCount = dialog->SamplesPerSegment();
        PlsResponseModel->Set(PlsFitResponseModel::Autoscale, dialog->Autoscale());
        PlsResponseModel->Set(PlsFitResponseModel::Normdata, dialog->Normalization());
        PlsResponseModel->Set(PlsFitResponseModel::SamplesPerCount, SamplesPerCount);
        PlsResponseModel->Set(PlsFitResponseModel::Predict,
          dialog->IsCross()
          ? (dialog->IsSegment()
            ? CV
            : CV_FULL)
          : TEST);
        UpdateParametersFromResponse();
        auto requestModel = CreateRequestAnalysisFromExistingModel(dialog->GetSelectedData(),
          SamplesPerCount, dialog->MaxCountOfPC(),
          dialog->Autoscale(), dialog->Normalization(),
          dialog->IsSegment(), dialog->IsCross());
        RequestPlsFit(requestModel);
      });
    dialog->adjustSize();
  }

  void ViewPageProjectPlsGraphicsPlotPage::MoveTitleToPossible(const std::vector<int>& ids)
  {
    for (const auto& fileId : ids)
    {
      const auto samples = PlsResponseModel->Get<AnalysisMetaInfoList>(PlsFitResponseModel::T_TitleGrad);
      const auto foundIt = std::find_if(samples.begin(), samples.end(),
        [&](const AnalysisMetaInfo& item) { return item.SampleId == static_cast<size_t>(fileId); });

      if (foundIt == samples.end())
        continue;

      PlsResponseModel->Exec<AnalysisMetaInfoList>(PlsFitResponseModel::T_TitlePossible,
        [&](AnalysisMetaInfoList& possible) { possible.push_back(*foundIt); });

      PlsResponseModel->Exec<IntVector>(PlsFitResponseModel::PossibleTestSamplesIds,
        [&](IntVector& possible) { possible.push_back(foundIt->SampleId); });

      PlsResponseModel->Exec<DoubleVector>(PlsFitResponseModel::PossibleTestSamplesConcentrations,
        [&](DoubleVector& possible)
        {
          if (foundIt->YConcentration.has_value())
            possible.push_back(foundIt->YConcentration.value());
        });

    }
  }

  void ViewPageProjectPlsGraphicsPlotPage::ResetPossible(const std::vector<int>& reset)
  {
    AnalysisMetaInfoList returned;
    PlsResponseModel->Exec<AnalysisMetaInfoList>(PlsFitResponseModel::T_TitlePossible,
      [&](AnalysisMetaInfoList& list)
      {
        returned.reserve(list.size() + reset.size());
        for (const auto& id : reset)
        {
          const auto foundIt = std::find_if(list.begin(), list.end(),
            [&](const AnalysisMetaInfo& item)
            { return item.SampleId == static_cast<size_t>(id); });
          if (foundIt != list.end())
          {
            returned.push_back(*foundIt);
            list.erase(foundIt);
          }
        }
      });

    PlsResponseModel->Exec<AnalysisMetaInfoList>(PlsFitResponseModel::T_TitleGrad,
      [&](AnalysisMetaInfoList& gradList)
      {
        std::copy(returned.begin(), returned.end(), std::back_inserter(gradList));
      });

    PlsResponseModel->Exec<AnalysisMetaInfoList>(PlsFitResponseModel::T_TitleProv,
      [&](AnalysisMetaInfoList& provList)
      {
        std::copy(returned.begin(), returned.end(), std::back_inserter(provList));
      });
  }

  void ViewPageProjectPlsGraphicsPlotPage::OpenMarkup()
  {
  }

  void ViewPageProjectPlsGraphicsPlotPage::Export()
  {
    ItemSelected tables;
    auto modelDataLabel = tr("Model Data");
    tables.push_back({ modelDataLabel, Qt::Checked });

    ItemSelected plots;
    for (const auto& [name, widget] : PlotWidgets)
      plots.push_back({ name, Qt::Checked });

    auto dialog = new ExportTablesAndPlotsDialog(tables, plots, this);
    connect(dialog, &WebDialog::Accepted, [&, dialog, modelDataLabel]
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
              const auto requestData = PlsResponseModel->Get<QByteArray>(PlsAnalysisIdentityModel::ExportRequestBody);
              const auto doc = QJsonDocument::fromJson(requestData);
              auto request = doc.object();
              request.insert(JsonTagNames::project_id, GenesisContextRoot::Get()->ProjectId());
              API::REST::ExportTables(request, [path](QNetworkReply* reply, QByteArray data)
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
          if (checked == Qt::Checked)
          {
            const auto it = std::find_if(PlotWidgets.begin(), PlotWidgets.end(),
              [plotName = name](const std::pair<QString, Widgets::PlsPlotWidget*> pair) { return plotName == pair.first; });

            if (it == PlotWidgets.end())
              continue;

            const auto plots = it->second->findChildren<GraphicsPlotExtended*>();
            for (int i = 0; i < plots.size(); i++)
            {
              auto pixmap = plots[i]->toPixmap(1920, 1080);
              const auto fullname = name + "_" + QString::number(i) + ".png";
#ifdef Q_OS_WASM
              QBuffer buffer;
              plots[i]->savePng(&buffer);
              QFileDialog::saveFileContent(buffer.data(), fullname);
#else
              pixmap.save(path + "/" + fullname);
#endif
            }
          }
        }
      });
    dialog->Open();
  }

  void ViewPageProjectPlsGraphicsPlotPage::RemoveExcludedSamples()
  {
    PlsAnalysisModel->Exec<AnalysisTable>(PlsAnalysisIdentityModel::AnalysisTable,
      [&](AnalysisTable& table) { table.CheckedValues.SamplesIds = GetCheckedSamplesId(); });
  }

  void ViewPageProjectPlsGraphicsPlotPage::RemoveExcludedPeaks()
  {
    PlsAnalysisModel->Exec<AnalysisTable>(PlsAnalysisIdentityModel::AnalysisTable,
      [&](AnalysisTable& table) { table.CheckedValues.MarkersIds = GetCheckedPeaks(); });
  }

  void ViewPageProjectPlsGraphicsPlotPage::UpdateModelMenu()
  {
    if (const auto modelMenuButton = findChild<ModelSettingsWidget*>())
    {
      if (MenuContext)
        DisconnectSignals();

      const auto id = static_cast<int>(PlsAnalysisModel->Get<size_t>(PlsAnalysisIdentityModel::AnalysisId));
      if (id > 0)
      {
        const auto existModelContext = new ExistModelSettingsContext(modelMenuButton);
        connect(existModelContext, &ExistModelSettingsContext::SaveAsNew, this, &ViewPageProjectPlsGraphicsPlotPage::SavePlsModel);
        connect(existModelContext, &ExistModelSettingsContext::SaveChanges, this, &ViewPageProjectPlsGraphicsPlotPage::SaveChangesPlsModel);
        MenuContext = existModelContext;
      }
      else
      {
        const auto newModelContext = new NewModelSettingsContext(modelMenuButton);
        connect(newModelContext, &NewModelSettingsContext::SaveNewModel, this, &ViewPageProjectPlsGraphicsPlotPage::SavePlsModel);
        MenuContext = newModelContext;
      }

      ConnectMenuSignals();
      modelMenuButton->SetMenu(MenuContext);
    }
  }

  void ViewPageProjectPlsGraphicsPlotPage::LoadGraphicsFromResponseModel()
  {
    const auto pcMaxCount = static_cast<int>(PlsResponseModel->Get<size_t>(PlsFitResponseModel::DefaultNumberPC));
    RMSEPlotWidget->SetPCMaxCountValue(PlsResponseModel->Get<size_t>(PlsFitResponseModel::ChosenNumberPC));
    for (const auto& [name, widget] : PlotWidgets)
    {
      widget->SetModel(PlsResponseModel);
      widget->SetAxisForPC(1, 2);
      widget->SetPCMaxCount(pcMaxCount);
    }
    connect(RMSEPlotWidget, &PlsMultiplePlotWidget::PCMaxCountValueChanged, this,
      [&](int value) { PlsResponseModel->Set(PlsFitResponseModel::ChosenNumberPC, static_cast<size_t>(value)); });

    connect(SlopeOffsetMultiplePlotWidget, &PlsMultiplePlotWidget::PCMaxCountValueChanged, this,
      [&](int value) { PlsResponseModel->Set(PlsFitResponseModel::DefaultNumberPC, static_cast<size_t>(value)); });

    UpdateModelMenu();
  }

  void ViewPageProjectPlsGraphicsPlotPage::RecalcModel()
  {
    const int excludedSamples = CountPlotWidget->GetExcludedSampleIds().size();
    const int excludedPeaks = LoadPlotWidget->GetExcludedPeaks().size();
    const auto table = PlsAnalysisModel->Get<AnalysisTable>(PlsAnalysisIdentityModel::AnalysisTable);
    if (table.CheckedValues.SamplesIds.size() - excludedSamples < MinimumCount
      || table.CheckedValues.MarkersIds.size() - excludedPeaks < MinimumCount)
    {
      Notification::NotifyError(tr("To rebuild a model, at least 3 samples should be selected on the counts plot and at least 3 peaks on the loads plot or at least 3 samples on the slope-offset plot and at least 3 variables on the regression coefficients plot"), tr("Analysis launch error"));
      return;
    }

    auto dial = new WebDialogPLSBuildSettingsRefactor(this);
    const auto checkedSamples = GetCheckedSamplesId();
    const auto checkedPeaks = GetCheckedPeaks();
    PlsAnalysisModel->Exec<PlsParameters>(PlsAnalysisIdentityModel::PlsParameters,
      [&](const PlsParameters& params)
      {
        dial->SetPCValue(std::min(static_cast<size_t>(checkedSamples.size()), params.nMaxLv));
      });
    dial->SetMaxPCCount(std::min(checkedSamples.size(), checkedPeaks.size()));
    dial->SetMaxSamplesPerSegmentCount(checkedSamples.size() - 1);
    dial->SetTestData(GetTestData());
    connect(dial, &WebDialog::Accepted, this,
      [&, dial]()
      {
        auto request = CreateRequestAnalysisFromExistingModel(
          dial->GetSelectedData(),
          dial->SamplesPerSegment(), dial->MaxCountOfPC(),
          dial->Autoscale(), dial->Normalization(),
          dial->IsSegment(), dial->IsCross());
        RequestPlsFit(request);
      });
    dial->Open();
  }

  IdentityModelPtr ViewPageProjectPlsGraphicsPlotPage::CreateRequestAnalysisFromContext(GenesisContextPtr context)
  {
    const auto isCross = context->GetData(JsonTagNames::IsCross).toBool();
    const auto isSegment = context->GetData(JsonTagNames::IsSegment).toBool();
    const auto predict = isCross
      ? isSegment
      ? "cv"
      : "cv_full"
      : "test";
    const auto plsRequestModel = std::make_shared<PlsFitRequestModel>();
    plsRequestModel->Set<bool>(PlsFitRequestModel::Autoscale, context->GetData(JsonTagNames::Autoscale).toBool());
    plsRequestModel->Set<bool>(PlsFitRequestModel::Normdata, context->GetData(JsonTagNames::Normdata).toBool());
    const auto analysisTable = context->GetData(JsonTagNames::AnalysisTable).value<AnalysisTable>();
    plsRequestModel->Set<IntVector>(PlsFitRequestModel::SampleIds, analysisTable.CheckedValues.SamplesIds);
    plsRequestModel->Set<IntVector>(PlsFitRequestModel::MarkerIds, analysisTable.CheckedValues.MarkersIds);
    plsRequestModel->Set<std::string>(PlsFitRequestModel::Key, Key);
    plsRequestModel->Set<std::string>(PlsFitRequestModel::Predict, predict);
    plsRequestModel->Set<size_t>(PlsFitRequestModel::CVCount,
      static_cast<size_t>(context->GetData(JsonTagNames::AnalysisSamplesCount).toInt()));
    plsRequestModel->Set<size_t>(PlsFitRequestModel::NMaxLV,
      static_cast<size_t>(context->GetData(JsonTagNames::NMaxLV).toInt()));
    plsRequestModel->Set<size_t>(PlsFitRequestModel::ParentId, 0);
    plsRequestModel->Set<int>(PlsFitRequestModel::TableId, TableId);
    plsRequestModel->Set<std::string>(PlsFitRequestModel::LayerName,
      context->GetData(JsonTagNames::LayerName).toString().toStdString());
    plsRequestModel->Set(PlsFitRequestModel::SampleConcentrations,
      context->GetData(JsonTagNames::SamplesConcentrations).value<OptionalDoubleVector>());
    plsRequestModel->Set(PlsFitRequestModel::TestSampleIds,
      context->GetData(JsonTagNames::TestData).value<IntVector>());
    plsRequestModel->Set(PlsFitRequestModel::TestSampleConcentrations,
      context->GetData(JsonTagNames::TestSampleConcentrations).value<DoubleVector>());
    plsRequestModel->Set(PlsFitRequestModel::PossibleTestSampleIds,
      context->GetData(JsonTagNames::PossibleTestSampleIds).value<IntVector>());
    plsRequestModel->Set(PlsFitRequestModel::PossibleTestSampleConcentrations,
      context->GetData(JsonTagNames::PossibleTestSampleConcentrations).value<DoubleVector>());
    // save local data for correct rewrite pls response model
    PlsResponseModel = std::make_shared<PlsFitResponseModel>();
    PlsResponseModel->Set<bool>(PlsFitResponseModel::Autoscale, context->GetData(JsonTagNames::Autoscale).toBool());
    PlsResponseModel->Set<bool>(PlsFitResponseModel::Normdata, context->GetData(JsonTagNames::Normdata).toBool());
    PlsResponseModel->Set<std::string>(PlsFitResponseModel::Predict, predict);
    PlsResponseModel->Set<size_t>(PlsFitResponseModel::SamplesPerCount, static_cast<size_t>(SamplesPerCount));
    return plsRequestModel;
  }

  IdentityModelPtr ViewPageProjectPlsGraphicsPlotPage::CreateRequestAnalysisFromExistingModel(const IntVector& testData,
    size_t samplesPerCount, size_t maxCountOfPc,
    bool autoscale, bool normalization, bool isSegment, bool isCross)
  {
    const auto predict = isCross
      ? isSegment
      ? CV
      : CV_FULL
      : TEST;
    const auto plsRequestModel = std::make_shared<PlsFitRequestModel>();
    plsRequestModel->Set<bool>(PlsFitRequestModel::Autoscale, autoscale);
    plsRequestModel->Set<bool>(PlsFitRequestModel::Normdata, normalization);
    const auto checkedSamples = GetCheckedSamplesId();
    plsRequestModel->Set<IntVector>(PlsFitRequestModel::SampleIds, checkedSamples);
    PlsAnalysisModel->Exec<AnalysisData>(PlsAnalysisIdentityModel::AnalysisData,
      [&](const AnalysisData& data)
      {
        OptionalDoubleVector sampleConcentrations;
        sampleConcentrations.reserve(checkedSamples.size());
        for (const auto& id : checkedSamples)
        {
          const auto foundIt = std::find_if(data.Data.T_Title.begin(), data.Data.T_Title.end(),
            [&](const AnalysisMetaInfo& info) { return info.SampleId == id; });

          sampleConcentrations.append(foundIt->YConcentration);
        }
        plsRequestModel->Set<OptionalDoubleVector>(PlsFitRequestModel::SampleConcentrations, sampleConcentrations);
      });

    plsRequestModel->Set<IntVector>(PlsFitRequestModel::MarkerIds, GetCheckedPeaks());
    plsRequestModel->Set<std::string>(PlsFitRequestModel::Predict, predict);
    plsRequestModel->Set<size_t>(PlsFitRequestModel::CVCount, samplesPerCount);
    plsRequestModel->Set<size_t>(PlsFitRequestModel::NMaxLV, maxCountOfPc);
    plsRequestModel->Set<size_t>(PlsFitRequestModel::ParentId, 0);
    plsRequestModel->Set<int>(PlsFitRequestModel::TableId, TableId);
    plsRequestModel->Set<std::string>(PlsFitRequestModel::Key, Key);
    const auto pIDs = PlsResponseModel->Get<IntVector>(PlsFitResponseModel::PossibleTestSamplesIds);
    DoubleVector concentrations;
    PlsResponseModel->Exec<std::string>(PlsFitResponseModel::LayerName,
      [&](const std::string& name) {plsRequestModel->Set<std::string>(PlsFitRequestModel::LayerName, name); });
    PlsResponseModel->Exec<DoubleVector>(PlsFitResponseModel::PossibleTestSamplesConcentrations,
      [&](const DoubleVector& con)
      {
        concentrations.reserve(testData.size());
        for (int i = 0; i < pIDs.size(); i++)
        {
          const auto it = std::find(testData.begin(), testData.end(), pIDs[i]);
          if (it != testData.end())
          {
            concentrations.push_back(con[i]);
          }
        }
      });

    plsRequestModel->Set(PlsFitRequestModel::TestSampleIds, testData);
    plsRequestModel->Set<DoubleVector>(PlsFitRequestModel::TestSampleConcentrations, concentrations);

    PlsResponseModel->Exec<IntVector>(PlsFitResponseModel::PossibleTestSamplesIds,
      [&](const IntVector& vec) { plsRequestModel->Set(PlsFitRequestModel::PossibleTestSampleIds, vec); });
    PlsAnalysisModel->Exec<size_t>(PlsAnalysisIdentityModel::AnalysisId,
      [&](size_t id) { AnalysisId = id; });
    PlsResponseModel->Exec<DoubleVector>(PlsFitResponseModel::PossibleTestSamplesConcentrations,
      [&](const DoubleVector& vec) { plsRequestModel->Set(PlsFitRequestModel::PossibleTestSampleConcentrations, vec); });

    return plsRequestModel;
  }

  void ViewPageProjectPlsGraphicsPlotPage::RequestPlsFit(IdentityModelPtr requestModel)
  {
    auto overlayId = GenesisWindow::Get()->ShowOverlay(tr("Loading data..."));
    const auto serializer = ServiceLocator::Instance().Resolve<PlsFitRequestModel, IJsonSerializer>();
    auto request = serializer->ToJson(requestModel);
    API::REST::MarkupPlsFit(request,
      [&, overlayId](QNetworkReply* reply, QJsonDocument doc)
      {
        if (const auto jsonObj = doc.object(); jsonObj.value(JsonTagNames::Error).toBool())
          Notification::NotifyError("Pls fit request return Error.");
        else
        {
          const auto response = jsonObj.value(JsonTagNames::Result).toObject();
          const auto serializer = ServiceLocator::Instance().Resolve<PlsFitResponseModel, IJsonSerializer>();
          assert(serializer);

          const auto autoscale = PlsResponseModel->Get<bool>(PlsFitResponseModel::Autoscale);
          const auto normdata = PlsResponseModel->Get<bool>(PlsFitResponseModel::Normdata);
          const auto predict = PlsResponseModel->Get<std::string>(PlsFitResponseModel::Predict);

          PlsResponseModel = serializer->ToModel(response);
          PlsResponseModel->Set<bool>(PlsFitResponseModel::Autoscale, autoscale);
          PlsResponseModel->Set<bool>(PlsFitResponseModel::Normdata, normdata);
          PlsResponseModel->Set<std::string>(PlsFitResponseModel::Predict, predict);
          PlsResponseModel->Set<size_t>(PlsFitResponseModel::SamplesPerCount, static_cast<size_t>(SamplesPerCount));
          CreatePlsAnalysisIdentityModel();
          emit PlsResponseModelLoaded();
        }
        GenesisWindow::Get()->RemoveOverlay(overlayId);
      },
      [overlayId](QNetworkReply* reply, QNetworkReply::NetworkError)
      {
        Notification::NotifyError("Markup pls fit error.");
        GenesisWindow::Get()->RemoveOverlay(overlayId);
      });
  }

  PlsFitResponse ViewPageProjectPlsGraphicsPlotPage::CreateFitResponse() const
  {
    return
    {
      PlsResponseModel->Get<PlsComponents>(PlsFitResponseModel::PlsComponents),
      PlsResponseModel->Get<DoubleVector>(PlsFitResponseModel::VarianceX),
      PlsResponseModel->Get<DoubleVector>(PlsFitResponseModel::VarianceYGrad),
      PlsResponseModel->Get<DoubleVector>(PlsFitResponseModel::VarianceYProv),
      PlsResponseModel->Get<DoubleVector>(PlsFitResponseModel::RMSEGradAll),
      PlsResponseModel->Get<DoubleVector>(PlsFitResponseModel::RMSEProvAll),
      PlsResponseModel->Get<DoubleVector>(PlsFitResponseModel::YxGrad),
      PlsResponseModel->Get<DoubleVector>(PlsFitResponseModel::YxProv),
      PlsResponseModel->Get<DoubleVector>(PlsFitResponseModel::PossibleTestSamplesConcentrations),
      PlsResponseModel->Get<IntVector>(PlsFitResponseModel::PossibleTestSamplesIds),
      PlsResponseModel->Get<DoubleVector>(PlsFitResponseModel::TestSamplesConcentrations),
      PlsResponseModel->Get<IntVector>(PlsFitResponseModel::TestSamplesIds),
      PlsResponseModel->Get<AnalysisMetaInfoList>(PlsFitResponseModel::T_TitlePossible),
    };
  }

  void ViewPageProjectPlsGraphicsPlotPage::LoadResponseModelFromServer(int analysis_id)
  {
    auto overlayId = GenesisWindow::Get()->ShowOverlay(tr("Loading data"));
    API::REST::GetAnalysis(analysis_id,
      [&, analysis_id, overlayId](QNetworkReply*, QJsonDocument doc)
      {
        if (const auto json = doc.object(); json.value(JsonTagNames::Error).toBool())
        {
          Notification::NotifyError(tr("Error : %1 while loading analysis.").arg(json.value(JsonTagNames::Status).toString()));
        }
        else
        {
          const auto jsonObject = json.value(JsonTagNames::Data).toObject();
          const auto modelSerializer = ServiceLocator::Instance().Resolve<PlsAnalysisIdentityModel, IJsonSerializer>();
          assert(modelSerializer);
          PlsAnalysisModel = modelSerializer->ToModel(jsonObject);
          PlsAnalysisModel->Set<size_t>(PlsAnalysisIdentityModel::AnalysisId, analysis_id);
          PlsAnalysisModel->Exec<Parameters>(PlsAnalysisIdentityModel::AnalysisParameters,
            [&](const Parameters& params) { Key = params.ValueType; });
          PlsAnalysisModel->Exec<size_t>(PlsAnalysisIdentityModel::AnalysisTableId,
            [&](size_t id) { TableId = id; });

          PlsAnalysisModel->Exec<size_t>(PlsAnalysisIdentityModel::AnalysisTypeId,
            [&](size_t id) { AnalysisTypeId = id; });
          PlsResponseModel = ConvertToPlsFitResponseModel(PlsAnalysisModel);
          emit PlsResponseModelLoaded();
        }
        GenesisWindow::Get()->RemoveOverlay(overlayId);
      },
      [overlayId](QNetworkReply*, QNetworkReply::NetworkError)
      {
        GenesisWindow::Get()->RemoveOverlay(overlayId);
        Notification::NotifyError(tr("Error : while loading analysis."));
      });
  }

  IdentityModelPtr ViewPageProjectPlsGraphicsPlotPage::CopyModelExceptId()
  {
    auto model = std::make_shared<PlsAnalysisIdentityModel>();

    model->Set<size_t>(PlsAnalysisIdentityModel::AnalysisId, 0);
    PlsAnalysisModel->Exec<AnalysisData>(PlsAnalysisIdentityModel::AnalysisData,
      [&](const AnalysisData& data)
      {
        model->Set<AnalysisData>(PlsAnalysisIdentityModel::AnalysisData, data);
      });

    PlsAnalysisModel->Exec<size_t>(PlsAnalysisIdentityModel::AnalysisTypeId,
      [&](const size_t& id)
      {
        model->Set<size_t>(PlsAnalysisIdentityModel::AnalysisTypeId, id);
      });

    PlsAnalysisModel->Exec<size_t>(PlsAnalysisIdentityModel::AnalysisTableId,
      [&](const size_t& id)
      {
        model->Set<size_t>(PlsAnalysisIdentityModel::AnalysisTableId, id);
      });

    PlsAnalysisModel->Exec<Parameters>(PlsAnalysisIdentityModel::AnalysisParameters,
      [&](const Parameters& params)
      {
        model->Set<Parameters>(PlsAnalysisIdentityModel::AnalysisParameters, params);
      });

    PlsAnalysisModel->Exec<AnalysisTable>(PlsAnalysisIdentityModel::AnalysisTable,
      [&](const AnalysisTable& params)
      {
        model->Set<AnalysisTable>(PlsAnalysisIdentityModel::AnalysisTable, params);
      });

    PlsAnalysisModel->Exec<QByteArray>(PlsAnalysisIdentityModel::ExportRequestBody,
      [&](const QByteArray& requestBody)
      {
        model->Set<QByteArray>(PlsAnalysisIdentityModel::ExportRequestBody, requestBody);
      });

    PlsAnalysisModel->Exec<PlsFitResponse>(AnalysisIdentityModel::AnalysisData,
      [&](const PlsFitResponse& requestBody)
      {
        model->Set<PlsFitResponse>(PlsAnalysisIdentityModel::AnalysisData, requestBody);
      });

    PlsAnalysisModel->Exec<ByteArray>(PlsAnalysisIdentityModel::PlsFitPickle,
      [&](const ByteArray& requestBody)
      {
        model->Set<ByteArray>(PlsAnalysisIdentityModel::PlsFitPickle, requestBody);
      });

    PlsAnalysisModel->Exec<PlsParameters>(PlsAnalysisIdentityModel::PlsParameters,
      [&](const PlsParameters& requestBody)
      {
        model->Set<PlsParameters>(PlsAnalysisIdentityModel::PlsParameters, requestBody);
      });

    return model;
  }

  IdentityModelPtr ViewPageProjectPlsGraphicsPlotPage::ConvertToPlsFitResponseModel(IdentityModelPtr model) const
  {
    const auto plsFitResponseModel = std::make_shared<PlsFitResponseModel>();

    const auto plsParameters = model->Get<PlsParameters>(PlsAnalysisIdentityModel::PlsParameters);
    plsFitResponseModel->Set(PlsFitResponseModel::DefaultNumberPC, plsParameters.DefaultNumberPC);
    plsFitResponseModel->Set(PlsFitResponseModel::ChosenNumberPC, plsParameters.ChosenNumberPC);
    plsFitResponseModel->Set(PlsFitResponseModel::Autoscale, plsParameters.Autoscale);
    plsFitResponseModel->Set(PlsFitResponseModel::Normdata, plsParameters.Normdata);
    plsFitResponseModel->Set(PlsFitResponseModel::nMaxLv, plsParameters.nMaxLv);
    plsFitResponseModel->Set(PlsFitResponseModel::Predict, plsParameters.Predict);
    plsFitResponseModel->Set(PlsFitResponseModel::SamplesPerCount, plsParameters.SamplesPerCount);
    plsFitResponseModel->Set(PlsFitResponseModel::LayerName, plsParameters.LayerName);
    plsFitResponseModel->Set(PlsFitResponseModel::ExportRequestBody, model->Get<QByteArray>(PlsAnalysisIdentityModel::ExportRequestBody));
    const auto analysisData = model->Get<AnalysisData>(PlsAnalysisIdentityModel::AnalysisData);
    plsFitResponseModel->Set(PlsFitResponseModel::P_Title, analysisData.Data.P_Title);
    plsFitResponseModel->Set(PlsFitResponseModel::T_TitleGrad, analysisData.Data.T_Title);
    plsFitResponseModel->Set(PlsFitResponseModel::P_id, analysisData.Data.P_Id);
    // save test data selected by user
    plsFitResponseModel->Set(PlsFitResponseModel::T_TitleProv, plsParameters.Predict == TEST
      ? analysisData.Data.TestData
      : analysisData.Data.T_Title);

    const auto plsFitresponse = model->Get<PlsFitResponse>(AnalysisIdentityModel::AnalysisData);
    plsFitResponseModel->Set(PlsFitResponseModel::PlsComponents, plsFitresponse.Components);
    plsFitResponseModel->Set(PlsFitResponseModel::VarianceX, plsFitresponse.VarianceX);
    plsFitResponseModel->Set(PlsFitResponseModel::VarianceYGrad, plsFitresponse.VarianceYGrad);
    plsFitResponseModel->Set(PlsFitResponseModel::VarianceYProv, plsFitresponse.VarianceYProv);
    plsFitResponseModel->Set(PlsFitResponseModel::RMSEGradAll, plsFitresponse.RMSEGradAll);
    plsFitResponseModel->Set(PlsFitResponseModel::RMSEProvAll, plsFitresponse.RMSEProvAll);
    plsFitResponseModel->Set(PlsFitResponseModel::YxGrad, plsFitresponse.YxGrad);
    plsFitResponseModel->Set<AnalysisMetaInfoList>(PlsFitResponseModel::T_TitlePossible, plsFitresponse.PossibleTTitle);
    plsFitResponseModel->Set<DoubleVector>(PlsFitResponseModel::YxProv, plsFitresponse.YxProv);
    plsFitResponseModel->Set<DoubleVector>(PlsFitResponseModel::PossibleTestSamplesConcentrations, plsFitresponse.PossibleTestSamplesConcentrations);
    plsFitResponseModel->Set<IntVector>(PlsFitResponseModel::PossibleTestSamplesIds, plsFitresponse.PossibleTestSamplesIds);
    plsFitResponseModel->Set<IntVector>(PlsFitResponseModel::TestSamplesIds, plsFitresponse.TestSamplesIds);
    plsFitResponseModel->Set<DoubleVector>(PlsFitResponseModel::TestSamplesConcentrations, plsFitresponse.TestSamplesConcentrations);
    plsFitResponseModel->Set(PlsFitResponseModel::BinaryModel, model->Get<ByteArray>(PlsAnalysisIdentityModel::PlsFitPickle));
    plsFitResponseModel->Set<AnalysisTable>(AnalysisIdentityModel::AnalysisTable, model->Get<AnalysisTable>(AnalysisIdentityModel::AnalysisTable));
    return plsFitResponseModel;
  }
}
