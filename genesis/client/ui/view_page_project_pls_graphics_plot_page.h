#pragma once
#ifndef VIEW_PAGE_PROJECT_GRAPHICS_PLOT_PAGE_H
#define VIEW_PAGE_PROJECT_GRAPHICS_PLOT_PAGE_H

#include "view.h"
#include "logic/models/identity_model.h"
#include "logic/structures/pca_data_structures.h"
#include "widgets/pls_plot_widget.h"
#include "contexts/new_model_settings_context.h"
#include "contexts/exist_model_settings_context.h"
#include "widgets/pls_multiple_plot_widget.h"

#include <QBoxLayout>

namespace Widgets
{
  class PlsSlopeOffsetPlotWidget;
  class PlsCoefficientPlotWidget;
  class PlsCountPlotWidget;
  class PlsLoadPlotWidget;
}

namespace Views
{
  using namespace Model;
  using namespace Widgets;

  class ViewPageProjectPlsGraphicsPlotPage : public View
  {
    Q_OBJECT

  public:
    explicit ViewPageProjectPlsGraphicsPlotPage(QWidget* parent = nullptr);
    ~ViewPageProjectPlsGraphicsPlotPage();

    void ApplyContextMarkupVersionAnalysisPLS(const QString& dataId, const QVariant& data);
    void SavePlsModel();
    void SaveChangesPlsModel();

    IdentityModelPtr GetPlsModel();


  signals:
    void PlsResponseModelLoaded();
    void PlsModelSavingFinished(bool error);

  private:
    std::string Key;
    int TableId = 0;
    int AnalysisId = 0;
    int AnalysisTypeId = 0;
    int SamplesPerCount = 0;
    Views::PlsSlopeOffsetPlotWidget* SlopeOffsetPlotWidget = nullptr;
    Views::PlsCoefficientPlotWidget* CoefficientPlotWidget = nullptr;
    Views::PlsCountPlotWidget* CountPlotWidget = nullptr;
    Views::PlsLoadPlotWidget* LoadPlotWidget = nullptr;
    // Основная модель по которой рисуются графики
    IdentityModelPtr PlsResponseModel = nullptr;
    // Вспомогательная модель из которой составляем основную
    IdentityModelPtr PlsAnalysisModel = nullptr;
    std::vector<std::pair<QString, PlsPlotWidget*>> PlotWidgets;
    Widgets::PlsMultiplePlotWidget* RMSEPlotWidget = nullptr;
    Widgets::PlsMultiplePlotWidget* SlopeOffsetMultiplePlotWidget = nullptr;
    QPointer<AnalysisModelSettingsContext> MenuContext = nullptr;
    QPushButton* RecalcButton = nullptr;

    void SetupUi();
    void ConnectSignals();
    void ConnectMenuSignals();
    void DisconnectSignals();

    void SaveNewModelToServer(IdentityModelPtr plsModel);
    void UpdateModelOnServer(IdentityModelPtr plsModel);
    void UpdateRecalcButtonVisibleState(bool hasExcluded);
    void CancelExcludePeaksCoefficientPlot(bool hasExcluded);
    void ResetExcludePeaksCoefficientPlot(const std::vector<int>& peaks);
    void SyncExcludedPeaksWithCoefficientPlot();
    void SyncPeaksStateWithLoadPlot();
    void OpenDataTable();
    void OpenModelParameters();
    void MoveTitleToPossible(const std::vector<int>& ids);
    void ResetPossible(const std::vector<int>& reset);

    // TODO Should open markup
    void OpenMarkup();
    void Export();
    void RemoveExcludedSamples();
    void RemoveExcludedPeaks();
    void UpdateModelMenu();

    void LoadGraphicsFromResponseModel();
    void RecalcModel();

    IdentityModelPtr CreateRequestAnalysisFromContext(GenesisContextPtr context);

    IdentityModelPtr CreateRequestAnalysisFromExistingModel(const Structures::IntVector& testData,
      size_t samplesPerCount, size_t maxCountOfPc,
      bool autoscale, bool normalization, bool isSegment, bool isCross);

    void RequestPlsFit(IdentityModelPtr requestModel);

    PlsFitResponse CreateFitResponse() const;

    void LoadResponseModelFromServer(int analysis_id);

    IdentityModelPtr CopyModelExceptId();

    // Convert from pls analysis model to pls fit response
    IdentityModelPtr ConvertToPlsFitResponseModel(IdentityModelPtr model) const;

    // Convert from pls fit response to pls analysis model
    void CreatePlsAnalysisIdentityModel();
    void UpdateParametersFromResponse() const;
    AnalysisData CreateAnalysisData() const;
    AnalysisInnerData CreateAnalysisInnerData() const;
    std::map<int, QString> GetTestData() const;
    IntVector GetCheckedSamplesId() const;
    IntVector GetCheckedPeaks() const;
    IntVector GetTestIds(const QStringList& names) const;

    // View interface
  public:
    void SwitchToPage(const QString &pageId) override;
    void SwitchToModule(int module) override;
    void LogOut() override;
  };
}
#endif
