#ifndef ANALYSIS_ENTITY_MODEL_H
#define ANALYSIS_ENTITY_MODEL_H

#include "shape_color_grouped_entity_plot_data_model.h"

#define INVALID_COMPONENT -1
#define INVALID_EXPL_VARIANCE -1.0

// using namespace AnalysisEntity;
class AnalysisEntityModel : public ShapeColorGroupedEntityPlotDataModel
{
  Q_OBJECT
public:
  AnalysisEntityModel(QObject *parent = nullptr);
  ~AnalysisEntityModel();
  QPointF currentEntityCoord(TEntityUid uid);
  double currentEntitySpectralValue(TEntityUid uid);
  void setExplVarianceMap(const TComponentsExplVarianceMap& map);
  double currentXExplVariance();
  double currentYExplVariance();
  void setCurrentXComponent(int comp);
  void setCurrentYComponent(int comp);
  void setCurrentSpectralComponent(int comp);
  void setCurrentXYComponent(int compx, int compy);
  void setColorModelFilter(const TPassportFilter& colorFilter);
  void setShapeModelFilter(const TPassportFilter& shapeFilter);
  void setCurrentModelFilters(const TPassportFilter& colorFilter, const TPassportFilter& shapeFilter);
  int currentXComponent();
  int currentYComponent();
  int currentSpectralComponent();
  void regroup(TPassportFilter colorFilter,
               TPassportFilter shapeFilter,
               const QList<TLegendGroup>& legendGroups);
  void cancelExclude(EntityType type);
  const QHash<int, QVariant>& modelData();
  void setModelData(const QHash<int, QVariant>& modelData);
  void setModelData(int role, const QVariant& value);
  QVariant modelData(int role);

  // JsonSerializable interface
public:
  void load(const QJsonObject &data) override;
  QJsonObject save() const override;


  static QString genericGroupName(const TPassportFilter& colorFilter,
                                  const TPassportFilter& shapeFilter,
                                  const TPassport& passport);
  static QString genericGroupName(const TPassportFilter& colorFilter,
                                  const TPassportFilter& shapeFilter,
                                  const TPassportFilter& passportFilter);
  static QString passportToSampleTitle(const TPassport& passport);

signals:
  //when this signal emitted, we should update all of our graphics items coordinates manually
  //by calling currentEntityCoord(uid)
  //alternatives: multiple emitting of entityDataChanged or single emitting of modelReset
  //wich is not so effectively as spot reaction in plot
  void modelDataChanged(const QHash<int, QVariant>& roleDataMap);

private:
  QHash<int, QVariant> mData;
};

#endif // ANALYSIS_ENTITY_MODEL_H

