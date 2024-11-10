#pragma once
#ifndef VIEW_PAGE_PROJECT_PLS_PREDICT_H
#define VIEW_PAGE_PROJECT_PLS_PREDICT_H

#include "view.h"
#include "logic/models/identity_model.h"
#include "ui/contexts/analysis_model_settings_context.h"

namespace Widgets
{
  class TableViewWidget;
}

namespace Views
{
  class ViewPageProjectPlsPredict : public View
  {
    Q_OBJECT

  public:
    explicit ViewPageProjectPlsPredict(QWidget* parent = nullptr);
    ~ViewPageProjectPlsPredict() = default;

    void ApplyContextMarkupVersionAnalysisPLSPredict(const QString& dataId, const QVariant& data);

  signals:
    void ParentModelLoaded();
    void PredictModelLoaded();

  private:
    void SetupUi();
    void ConnectSignals();

    void ShowSaveDialog();
    void SavePredictAnalysis(const QString& title, const QString& comment);
    void FillPredictModelFromContext();

    void UpdateModelMenu();

    void ConnectMenuSignals();
    void DisconnectMenuSignals();

    void OpenDataTable();
    void OpenMarkup();
    void Export();

    void RequestParentAnalyisList();
    void ShowPlsPredictDialog(Model::IdentityModelPtr parentAnalysisListModel);

    void LoadParentPlsAnalysis(size_t id);
    void RecalcModel();
    void UpdateTableFromPredictModel();
    void ConvertResponseToModel();

    Model::IdentityModelPtr PredictResponseModel = nullptr;
    Model::IdentityModelPtr PredictModel = nullptr;
    Model::IdentityModelPtr ParentModel = nullptr;
    QPointer<Widgets::TableViewWidget> TableView;
    QPointer<Widgets::AnalysisModelSettingsContext> MenuContext = nullptr;
  };
}

#endif
