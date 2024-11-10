#ifndef PCA_PLOT_LEGEND_WIDGET_H
#define PCA_PLOT_LEGEND_WIDGET_H

#include "plot_legend_widget_refactor.h"
#include <logic/models/shape_color_grouped_entity_plot_data_model.h>


class AnalysisEntityModel;
class PcaPlotLegendWidget : public PlotLegendWidgetRefactor
{
  Q_OBJECT
public:
  explicit PcaPlotLegendWidget(QWidget* parent = nullptr);
  void setModel(QPointer<AnalysisEntityModel> model);
  void setHottelingMode(bool inHottelingMode);

private:
  void onEntityChanged(EntityType type,
                     TEntityUid eId,
                     DataRoleType role,
                     const QVariant& value);

  void onEntityResetted(EntityType type,
                      TEntityUid eId,
                      AbstractEntityDataModel::ConstDataPtr data);

  void onEntityAdded(EntityType type,
                   TEntityUid eId,
                   AbstractEntityDataModel::ConstDataPtr data);

  void onEntityAboutToRemove(EntityType type,
                           TEntityUid eId);
  void onModelAboutToReset();
  void onModelReset();
  void scanModel();

  QString formTitle(AbstractEntityDataModel::ConstDataPtr ent, QString *tooltip = nullptr);
  QString formTitle(TEntityUid eId, QString *tooltip = nullptr);
  QString formToolTip(int samplesCount, const QString &groupTitle);

private:
  QPointer<AnalysisEntityModel> mModel = nullptr;
  bool mInHottelingMode = false;
  QList<QMetaObject::Connection> mModelConnections;
  TEntityUid mNonParticipantsGroupUid;


  // QWidget interface
protected:
  void resizeEvent(QResizeEvent *event) override;
};



#endif // PCA_PLOT_LEGEND_WIDGET_H
