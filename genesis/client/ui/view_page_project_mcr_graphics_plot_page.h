#pragma once
#ifndef VIEW_PAGE_PROJECT_MCR_GRAPHICS_PLOT_PAGE_H
#define VIEW_PAGE_PROJECT_MCR_GRAPHICS_PLOT_PAGE_H

#include "view.h"
#include "widgets/pls_plot_widget.h"
#include "contexts/new_model_settings_context.h"
#include "contexts/exist_model_settings_context.h"
#include "widgets/pls_multiple_plot_widget.h"

#include <QBoxLayout>

namespace Widgets
{
class TabWidget;
}

namespace Views
{
  using namespace Model;
  using namespace Widgets;

  class ViewPageProjectMCRGraphicsPlotPage : public View
  {
    Q_OBJECT

  public:
    explicit ViewPageProjectMCRGraphicsPlotPage(QWidget* parent = nullptr);
    ~ViewPageProjectMCRGraphicsPlotPage() = default;

    IdentityModelPtr GetMCRModel() const;

    void SaveMcrModel();
    void SaveChangesMcrModel();

  signals:
    void MCRResponseModelLoaded();
    void MCRModelSavingFinished(bool error);

  private:
    IdentityModelPtr McrIdentityModel = nullptr;
    QPointer<AnalysisModelSettingsContext> MenuContext = nullptr;
    TabWidget* mPlotTabWidget = nullptr;
    std::string ParameterKey;
    QPushButton* RecalcButton = nullptr;
    int TableId = 0;
    bool IsRebuildModel = false;
    void ApplyContextMarkupVersionAnalysisMCR(const QString& dataId, const QVariant& data) override;

    void SetupUi();
    void ConnectSignals();
    void ConnectMenuSignals();
    void DisconnectSignals();
    void UpdateModelMenu();
    void UpdateRecalcButton();
    void OpenDataTable();
    // TODO show open parameters
    void OpenModelParameters();
    // TODO should open markup
    void OpenMarkup();

    void Export();

    void SaveNewModelToServer(IdentityModelPtr mcrModel);
    void LoadGraphicsFromResponseModel();
    void ConvertModelFromResponse(IdentityModelPtr mcrModel);
    AnalysisData CreateAnalysisData(IdentityModelPtr mcrModel);
    IdentityModelPtr CopyModelExceptId();
    IdentityModelPtr CreateRequestModel(GenesisContextPtr context) const;
    IdentityModelPtr CreateRequestModel(const MCRParameters& parameters,
      int tableId, const IntVector& sampleIds, const IntVector& markersIds) const;

    // before call make sure what you update Parameter key and Table
    // and table id
    void ParseResponseFromServer(IdentityModelPtr requestModel);
    void GetMcrAnalysis(const QJsonObject& request);
    void ParseResponseJson(const QJsonObject& response);


    // View interface
  public:
    void SwitchToPage(const QString &pageId) override;
    void SwitchToModule(int module) override;
    void LogOut() override;
  };

}
#endif
