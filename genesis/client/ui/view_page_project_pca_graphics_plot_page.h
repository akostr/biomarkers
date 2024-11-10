#ifndef VIEW_PAGE_PROJECT_PCA_GRAPHICS_PLOT_PAGE_H
#define VIEW_PAGE_PROJECT_PCA_GRAPHICS_PLOT_PAGE_H

#include "view.h"

#include "contexts/analysis_model_settings_context.h"
#include "logic/models/identity_model.h"
#include "logic/structures/analysis_model_structures.h"
#include "logic/models/analysis_entity_model.h"
#include <graphicsplot/graphicsplot_extended.h>
#include <QPointer>

class AnalysisPlotRefactor;
class AnalysisEntityModel;
namespace Widgets
{
class PcaCountPlotWidget;
class PcaPlotWidget;
}
namespace Views
{
  class ViewPageProjectPcaGraphicsPlotPage : public View
  {
    Q_OBJECT

  public:
    explicit ViewPageProjectPcaGraphicsPlotPage(QWidget* parent = nullptr);
    ~ViewPageProjectPcaGraphicsPlotPage() override = default;

    void ApplyContextMarkupVersionAnalysisPCA(const QString& dataId, const QVariant& data) override;
    Model::IdentityModelPtr getIdentityModel() const;

    void savePcaModel();
    void saveChangesPcaModel();
    void LogOut() override;
    void SwitchToPage(const QString& pageId) override;
    void SwitchToModule(int module) override;
    void setEntityModel(QPointer<AnalysisEntityModel> model);

  signals:
    void pcaModelSavingFinished(bool error);

  private:
    bool mIsRebuildModel = false;

    void setupUi();
    void loadGraphicsFromEntityModel();
    void loadResponseModelFromServer(int analysis_id);
    void updateModelMenu();
    void connectMenuSignals();
    void disconnectMenuSignals();
    void openDataTable();
    void openModelParameters();
    // TODO should open markup
    void exportPage();
    void createPcaModel();
    void removeExcludedSamples();
    void removeExcludedPeaks();
    void onModelReset();
    void rebuildModel();
    void saveNewModelToServer(Model::IdentityModelPtr model);
    void updateModelByResponse(Model::IdentityModelPtr model);
    void getAnalysis(const QJsonObject& jsonRequest);

    void onEntityChanged(EntityType type,
                         TEntityUid eId,
                         DataRoleType role,
                         const QVariant& value);

    Model::IdentityModelPtr copyModelExceptId();
    Model::IdentityModelPtr convertPcaModelToResponse(Model::IdentityModelPtr model);
    QPointer<AnalysisEntityModel> extractEntitiesFromPcaModel(Model::IdentityModelPtr model);

    Structures::AnalysisData createAnalysisData() const;
    Structures::AnalysisInnerData createAnalysisInnerData() const;
    Structures::IntVector getCheckedSamples() const;
    Structures::IntVector getCheckedPeaks() const;

  private:
    QPushButton* mRecalcButton = nullptr;
    Model::IdentityModelPtr mPcaResponseModel = nullptr;
    Model::IdentityModelPtr mPcaModel = nullptr;
    QList<QPair<QString, QPointer<Widgets::PcaPlotWidget>>> mPlotWidgets;
    QMap<QString, QPointer<AnalysisPlotRefactor>> mPlots;
    QPointer<Widgets::AnalysisModelSettingsContext> mMenuContext = nullptr;
    QPointer<Widgets::PcaCountPlotWidget> mCountPlot;
    QPointer<AnalysisEntityModel> mEntityModel;
  };
}

#endif
