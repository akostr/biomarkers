#ifndef ANALYSIS_PLOT_REFACTOR_H
#define ANALYSIS_PLOT_REFACTOR_H

#include <graphicsplot/graphicsplot.h>
#include <logic/models/analysis_entity_model.h>
#include <logic/models/shape_color_grouped_entity_plot_data_model.h>

class GPShapeWithLabelItem;
class SampleInfoPlate;
class AnalysisPlotRefactor : public GraphicsPlot
{
  Q_OBJECT
public:
  static const char* kSampleId;
  static const char* kEntityUid;
  static const char* kOrderNum;
  static const char* kPeakId;

  AnalysisPlotRefactor(QWidget *parent = nullptr, const QSet<EntityType>& displayedTypes = {});

  void setModel(QPointer<AnalysisEntityModel> model);
  AnalysisEntityModel* getModel();

  void resizeCached();
  void resize();

  void setDefaultAxisRanges(const GPRange& xRange, const GPRange& yRange);
  void resizeAxisToDefault();
  void setXAxisLabel(const QString& label);
  void setYAxisLabel(const QString& label);
  void setDisplayedItemTypes(const QSet<EntityType>& types);
  void select(const QSet<TEntityUid>& uids);
  //@TODO:
  void showNames(bool isShow);
  void showPassport(bool isShow);
  bool namesHidden();
  bool passportsHidden();
  void setRenderType(AnalysisEntity::RenderType type);
  void queuedReplot();

signals:
  void contextMenuRequested(QContextMenuEvent* event);

protected:
  virtual void handleHighlighting(QPointF plotPos);

private:
  void disconnectSignals(AnalysisEntityModel* model);
  void connectSignals(AnalysisEntityModel* model);

  void onModelDataChanged(const QHash<int, QVariant>& roleDataMap);
  void onGroupColorChanged(QSet<TEntityUid> affectedEntities);
  void onGroupShapeChanged(QSet<TEntityUid> affectedEntities);
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
  void updateCurve();
  void updateShapes(EntityType type);
  void updateShapes();


private:
  // QRectF mViewRectCache;
  GPRange mXRangeDefault;
  GPRange mYRangeDefault;
  QPointer<AnalysisEntityModel> mModel;
  QHash<GPShapeWithLabelItem*, SampleInfoPlate*> mShapesPlates;//there is only that shapes, wich have plates
  QHash<TEntityUid, GPShapeWithLabelItem*> mUidsShapes; //full list of shapes is here
  QPointer<GPCurve> mCurve;
  bool mIsInResetState = false;
  QSet<EntityType> mDisplayedItemsTypes;
  AnalysisEntity::RenderType mRenderType = AnalysisEntity::Dot;
  bool mShowNames = true;
  bool mShowPassports = true;
  bool mHasHighlighting = false;
  bool mDirty = false;

  // QWidget interface
protected:
  void mousePressEvent(QMouseEvent *event) override;
  void mouseReleaseEvent(QMouseEvent *event) override;
  void mouseMoveEvent(QMouseEvent *event) override;
  void mouseDoubleClickEvent(QMouseEvent *event) override;
  void contextMenuEvent(QContextMenuEvent *event) override;


  // GraphicsPlot interface
protected slots:
  void processRectSelection(const QRect &rect, QMouseEvent *event) override;

  // QWidget interface
protected:
  void showEvent(QShowEvent *event) override;
};



#endif // ANALYSIS_PLOT_REFACTOR_H
