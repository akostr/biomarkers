#ifndef CHROMATOGRAMMDATAMODEL_H
#define CHROMATOGRAMMDATAMODEL_H

#include "abstract_data_model.h"
#include "genesis_markup_forward_declare.h"

namespace GenesisMarkup{

using ChromaId = int;

class ChromatogramDataModel: public AbstractEntityDataModel
{
  Q_OBJECT

public:
  ChromatogramDataModel(ChromaId id);
  ChromatogramDataModel(const ChromatogramDataModel& other);

  ~ChromatogramDataModel() = default;

  void setId(ChromaId id);
  ChromaId id() const;
  void setCutPoint(double x);
  void resetCutPoint();
  double cutPoint();
  bool hasCutPoint();
  void updateTitlesDisplayMode();
  QPair<QUuid, QUuid> getNeighbourPeaksUids(const QUuid& uid) const;
  QPair<QUuid, QUuid> getNeighbourIntervalsUids(const QUuid& uid) const;
  QUuid getLeftPeakUid(const QUuid& uid) const;
  QUuid getRightPeakUid(const QUuid& uid) const;
  QUuid getLeftIntervalUid(const QUuid& uid) const;
  QUuid getRightIntervalUid(const QUuid& uid) const;
  QUuid getPeakUidFromX(double x);
  QList<QUuid> getOrderedPeaks();
  ChromaSettings getChromaSettings() const;
  void setChromaSettings(const ChromaSettings& settings);
  BaseLineDataModelPtr getBaseLine();

  void load(const QJsonObject& data) override;
  QJsonObject save() const override;
  QJsonArray getJPeaksForStepCalculations() const;
  QJsonArray getJBaseLineForStepCalculations() const;
  void parseJPeaks(const QJsonArray& peaks);
  void parseJBaseLine(const QJsonArray& jbline);
  void parseJBaseLines(const QJsonObject &jbline, int defaultBaseline);
  void parseJIntervals(const QJsonArray &intervals);
  bool isMaster() const;

  const QHash<int, QVariant>& internalDataReference() const;

  //debug purpose
  QJsonObject print(const QList<int> &ignoreRoles, const QList<int> &ignoreTypes);
  //
  MarkupDataModel* markupModel() const;
  void setMarkupModel(MarkupDataModel* newModel);

  void setChromatogramValue(int DataRole, QVariant newValue);
  QVariant getChromatogramValue(int DataRole);
  ChromatogrammModelPtr getCopyWithFilteredEntities(std::function<bool(ConstDataPtr)> filter);
  
  QList<QColor> interMarkerColors() const;
  void setInterMarkerColors(const QList<QColor> &newInterMarkerColors);
  void resetInterMarkerColors();

  QColor markerColor() const;
  void setMarkerColor(const QColor &newMarkerColor);
  void resetMarkerColor();

  QColor peakColor() const;
  void setPeakColor(const QColor &newPeakColor);
  void resetPeakColor();

  QString markerPrefix() const;
  void setMarkerPrefix(const QString &newMarkerPrefix);
  void resetMarkerPrefix();

  Q_PROPERTY(QList<QColor> interMarkerColors READ interMarkerColors WRITE setInterMarkerColors RESET resetInterMarkerColors NOTIFY interMarkerColorsChanged FINAL)
  Q_PROPERTY(QColor markerColor READ markerColor WRITE setMarkerColor RESET resetMarkerColor NOTIFY markerColorChanged FINAL)
  Q_PROPERTY(QColor peakColor READ peakColor WRITE setPeakColor RESET resetPeakColor NOTIFY peakColorChanged FINAL)
  Q_PROPERTY(QString markerPrefix READ markerPrefix WRITE setMarkerPrefix RESET resetMarkerPrefix NOTIFY markerPrefixChanged FINAL)

signals:
  void chromatogramDataChanged(int DataRole, QVariant newValue);
  void markerPrefixChanged();
  void interMarkerColorsChanged();
  void markerColorChanged();
  void peakColorChanged();


private:
  QHash<int, QVariant> mData;
  MarkupDataModel* mMarkupModel;

  QMap<double, TEntityUid> mPeaksOrderingMap;//for peaks sorting by xStart coord
  QMap<double, TEntityUid> mIntervalsOrderingMap;//for peaks sorting by xStart coord

  QList<QColor> mInterMarkerColors;
  QColor mMarkerColor;
  QColor mPeakColor;
  QString mMarkerPrefix;

public:
  bool setEntityData(TEntityUid Id, DataRoleType role, const QVariant &value) override;
  bool resetEntityData(TEntityUid Id, const DataModel &newData) override;
  QUuid addNewEntity(EntityType type, const DataModel &data, QUuid uid = QUuid()) override;
  QUuid addNewEntity(EntityType type, DataModel *data = nullptr, QUuid uid = QUuid()) override;
  bool removeEntity(TEntityUid Id) override;

  QHash<int, QVariant> data() const;
  void setData(const QHash<int, QVariant> &newData);
  void resolvePeakNaming(TEntityUid Id, ChromatogrammTitlesDisplayModes mode);

  static QList<QColor> DefaultInterMarkerColors;
  static QColor DefaultMarkerColor;
  static QColor DefaultPeakColor;
  static QString DefaultMarkerPrefix;


private:
  void resolveAllPeaksOrderingNamingAndColoring(QList<int> affectedPeaksTypes);

  void validatePeakRequiredValues(TEntityUid peakId);
  void validatePeakRequiredValues(DataModel peakData);

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
  void onEntityAboutToRemove(EntityType type, TEntityUid uid);
  void onModelAboutToReset();
  void onModelReset();

  void recalcPeaksOrderMap();
};

}//GenesisMarkup

//struct Pt
//{
//  Pt(double X, double Y) : x(X), y(Y){};
//  double x;
//  double y;
//};

//void createAndFillExample()
//{
//  GenesisMarkup::ChromatogramDataModel model;
//  auto newEntityUid = model.addNewEntity(GenesisMarkup::EntityType::DataPeak, new AbstractEntityDataModel::DataModel());
//  model.resetEntityData(GenesisMarkup::EntityType::DataPeak, newEntityUid);
//  model.setEntityData(GenesisMarkup::EntityType::DataPeak, newEntityUid, GenesisMarkup::PeakStart, QVariant::fromValue<Pt>(Pt(0,0)));
//}

#endif // CHROMATOGRAMMDATAMODEL_H
