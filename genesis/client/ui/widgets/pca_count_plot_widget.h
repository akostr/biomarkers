#ifndef PCA_COUNT_PLOT_WIDGET_H
#define PCA_COUNT_PLOT_WIDGET_H

#include "logic/markup/abstract_data_model.h"
#include "pca_plot_widget.h"
#include "plot_toolbar_widget.h"
// #include "ui/flow_layout.h"
#include "ui/plots/counts_plot_manual_hotteling.h"
// #include "plot_legend_widget.h"
#include <logic/models/identity_model.h>

#include <QDialogButtonBox>

// class CountsPlotManualHotteling;
class AnalysisPlotRefactor;
class PcaPlotLegendWidget;
namespace Widgets
{
  class PlotSetupViewContext;
  class PcaCountPlotWidget final : public PcaPlotWidget
  {
    Q_OBJECT

  public:
    explicit PcaCountPlotWidget(QWidget* parent);
    ~PcaCountPlotWidget() override = default;
    void setModel(QPointer<AnalysisEntityModel> model) override;

    void updateHotteling(const QJsonObject& obj);
    void clearHotteling();
    bool hottelingManualGroupingIsActive();
    bool hotellingLoaded() const;
    QJsonObject hottelingExportRequestBody() const;

  signals:
    // void AxisChanged(int yPC, int xPC);
    void pageTitleCaptionOverride(QString caption);
    void restorePageTitle();
    // void ExcludeStateChanged();

  private:
    void setupUi();
    void connectSignals();
    void regroupItems();
    void removeGroupAndRegroup(const TEntityUid &groupUid);
    void removeGroupsAndRegroup(const QSet<TEntityUid> &groupsUids);

    void showPickAxis();
    void setParamGroup();
    void setupCustomItemStyle();

    void onExcludedStateChanged();

    void calcHottelingCriterion();
    void exportHotteling();

    void setupUiForManualHotteling();
    void restoreUiFromManualHotteling();

    void requestHotteling(const QJsonObject& json);
    void recalcManualHotteling();

    bool eventFilter(QObject* watched, QEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

    void onPlotContextMenuRequested(QContextMenuEvent* event);

    void onGroupEditRequested(const QUuid& uid);

    Model::IdentityModelPtr createHottelingRequestModel(const QString& criterion, uint gk);
    Model::IdentityModelPtr createHottelingRequestModel(const QHash<QString, QList<QUuid>>& groups, uint gk);
    Model::IdentityModelPtr createHottelingRequestModel(uint gk);

    void onEntityModelEntityDataChanged(EntityType type,
                                        TEntityUid eId,
                                        DataRoleType role,
                                        const QVariant& value);
    void onEntityAboutToRemove(EntityType type,
                               TEntityUid eId);
    void onEntityModelAboutToReset();
    void onEntityModelReset();
    void onEntityModelDataChanged(const QHash<int, QVariant>& roleDataMap);

  private:
    void createHottelingContextMenu(QContextMenuEvent* event);
    void createContextMenu(QContextMenuEvent* event);
    void confirmRemoveFromGroup();
    QColor randomColor();
    QPainterPath randomShape();
    void grayAll(bool gray);

    //helpers:
    void excludeFromGroup(QList<TEntityUid> uids);
    void rescaleXAxisToYAxis();
    void rescaleYAxisToXAxis();

  private:

    PlotToolbarWidgetPtr mPlotToolBar = nullptr;
    PlotSetupViewContext* mToolbarMenu = nullptr;
    QPointer<AnalysisPlotRefactor> mPlot;
    QPointer<QMenu> mContextMenu = nullptr;

    QVBoxLayout* mMainLayout = nullptr;
    QScrollArea* mScrollArea = nullptr;
    PcaPlotLegendWidget* mPlotLegend = nullptr;
    QPushButton* mHottelingCalcButton = nullptr;

    QDialogButtonBox* mRecalcHottelingManual = nullptr;
    QJsonObject mHottelingRequestBody;

    bool mHotellingLoaded = false;
    bool mInHottelingManualMode = false;
    QUuid mNonParticipantsGroupUid;

  };
}
#endif
