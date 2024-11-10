#pragma once
#ifndef VIEW_PAGE_ANALYSIS_DATA_TABLE_PAGE_H
#define VIEW_PAGE_ANALYSIS_DATA_TABLE_PAGE_H

#include "view.h"
#include "logic/enums.h"
#include "logic/models/identity_model.h"
#include "logic/structures/analysis_model_structures.h"

namespace Widgets
{
  class ParameterTableView;
}

namespace Controls
{
  class DataTableCaptionButtonControls;
}

namespace Views
{
  class ViewPageAnalysisDataTablePage : public View
  {
    Q_OBJECT

  public:
    explicit ViewPageAnalysisDataTablePage(QWidget* parent = nullptr);
    ~ViewPageAnalysisDataTablePage() override;

    void ApplyContextStepBasedMarkup(const QString& dataId, const QVariant& data) override;
    void ApplyContextModules(const QString& dataId, const QVariant& data) override;

  signals:
    void ParentModelLoaded();

  protected:
    void showEvent(QShowEvent* event) override;

  private:
    int TableId = 0;
    QString TableTitle;
    QString TableComment;
    bool AutoLayerConcentraion = false;
    bool EmptyConcentration = false;
    QString SelectedLayerForPls;
    Constants::AnalysisType OpenedFrom = Constants::AnalysisType::NONE;
    Widgets::ParameterTableView* TableView = nullptr;
    Controls::DataTableCaptionButtonControls* ButtonControls = nullptr;
    Model::IdentityModelPtr DataTableModel = nullptr;
    Model::IdentityModelPtr PlsModelForPredict = nullptr;
    void SetupUi();
    void ConnectSignals();

    void IndexChanged(Constants::AnalysisType type);

    void OpenRatioMatrixClicked();
    void CalculateCoefficientTableClicked();
    void CreatePlotClicked();
    void ExportClicked();
    void EditClicked();
    void RemoveClicked();

    void ReturnDataMarkup();
    void ReturnToModel();
    void BuildModel(Constants::AnalysisType type);

    void BuildPls();
    void ShowPlsDialog();
    void BuildMcr();
    void BuildPca();
    void BuildPca(bool normalization, bool autoscale, bool pc9599);
    void BuildPlsPredict();
    void RequestParentPlsAnalysis();
    void RequestParentAnalyisList();
    void ShowPlsPredictDialog(Model::IdentityModelPtr parentAnalysisListModel);
    void UncheckSampleCheckboxes();
    void SetupCheckBoxesState();
    void UpdateCheckBoxesState(QString dataId, QVariant data);

    void LoadParentPlsAnalysis(size_t id);
    void SetupTableForPlsPredict();

    Constants::AnalysisType GetTypeFromContext() const;
    // This 2 func return checked data
    // and not checked data from data table
    std::map<int, QString> GetTestingData() const;

    void SendPlsRequestData(const Structures::IntVector&testData,
      size_t samplesPerCount, size_t maxCountOfPc,
      bool autoscale, bool normalization, bool isSegment, bool isCross) const;

    Structures::AnalysisTable GetAnalysisTable() const;
    Structures::IntVector GetChecked(const std::vector<bool>& checkedState, const Structures::IntVector& ids) const;
    Structures::IntVector GetUnChecked(const std::vector<bool>& checkedState, const Structures::IntVector& ids) const;
    std::vector<bool> CompareCurrentIdsWithChecked(const Structures::IntVector& currentIds, const Structures::IntVector& checked);
    Structures::IntVector GetCheckedSamples() const;
    QStringList GetCheckedSampleNames() const;
    Structures::IntVector GetCheckedPeaks() const;
    Structures::IntVector GetSampeIdsByNames(const QStringList& sampleNames) const;

    Model::IdentityModelPtr GetPlsRequestModel(const Structures::IntVector& testData,
      size_t samplesPerCount, size_t maxCountOfPc,
      bool autoscale, bool normalization, bool isSegment, bool isCross) const;

    void SetPlsContextData(const Structures::IntVector& testData,
      size_t samplesPerCount, size_t maxCountOfPc,
      bool autoscale, bool normalization, bool isSegment, bool isCross) const;
  };
}
#endif
