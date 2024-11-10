#ifndef MARKUPDATAMODEL_H
#define MARKUPDATAMODEL_H

#include "logic/markup/chromatogram_data_model.h"
#include "json_serializable.h"
#include "genesis_markup_forward_declare.h"
#include "logic/markup/curve_data_model.h"
// #include <QPointer>

namespace GenesisMarkup{

using ChromaId = int;
class MarkupStepController;
class MarkupDataModel : public QObject, public JsonSerializable
{
  Q_OBJECT
public:
  MarkupDataModel(MarkupStepController& controller, QObject *parent = nullptr);
  MarkupDataModel(MarkupStepController& controller, const MarkupDataModel& other, QObject* newParent = nullptr);
  ~MarkupDataModel() = default;

  static void clearCurves();
  //debug purpose:
  QJsonObject print(const QList<int> &ignoreRoles = {}, const QList<int> &ignoreTypes = {});

  // JsonSerializable interface
public:
  void load(const QJsonObject &data) override;
  QJsonObject save() const override;
  MarkupStepController* controller() const;

  ChromatogrammModelPtr getChromatogramm(ChromaId id);
  ChromatogrammModelPtr takeChromatogramm(ChromaId id);
  static CurveDataModelPtr getCurve(ChromaId id);
  static bool hasCurve(ChromaId id);
  static bool setCurve(ChromaId id, CurveDataModelPtr ptr);
  bool addChromatogramm(ChromaId id, ChromatogrammModelPtr ptr = nullptr);
  bool addChromatogramm(ChromaId id, const ChromatogramDataModel &data);
  bool swapChromatogramm(ChromaId id, ChromatogrammModelPtr newChromaPtr);
  bool removeChromatogramm(ChromaId id);
  void clearChromatogramms();

  int getChromatogrammsCount() const;
  QList<ChromaId> getChromatogrammsIdList() const;
  QList<QPair<ChromaId, ChromatogrammModelPtr>> getChromatogrammsList() const;
  QList<ChromatogrammModelPtr> getChromatogrammsListModels() const;
  bool hasChromatogram(int Id) const;

  void setData(const QHash<uint, QVariant>& data);
  const QHash<uint, QVariant>& data() const;
  void setData(uint role, QVariant data);
  QVariant getData(uint role) const;
  void resetData(QHash<uint, QVariant> newData = QHash<uint, QVariant>());

  void setMaster(ChromaId newId);
  void resetMaster();
  ChromatogrammModelPtr getMaster();
  bool hasMaster();
  bool hasInternalMaster();

  void updateValuablePeaksSortedIdList();
  QList<int> getValuablePeaksSortedIdList();

  void updateTitlesDisplayMode();

signals:
  void dataChanged(uint role, const QVariant& data);
  void dataResetted(const QHash<uint, QVariant>& newData);
  void chromatogrammAdded(ChromaId id, ChromatogrammModelPtr ptr);
  void chromatogrammRemoved(ChromaId id);
  void chromatogrammSwapped(ChromaId id, ChromatogrammModelPtr newPtr);
  void chromatogrammsCleared();

  void chromatogramEntityChanged(ChromaId id,
                                 EntityType type,
                                 TEntityUid eId,
                                 DataRoleType role,
                                 const QVariant& value);

  void chromatogramEntityResetted(ChromaId id,
                                  EntityType type,
                                  TEntityUid eId,
                                  AbstractEntityDataModel::ConstDataPtr data);

  void chromatogramEntityAdded(ChromaId id,
                               EntityType type,
                               TEntityUid eId,
                               AbstractEntityDataModel::ConstDataPtr data);

  void chromatogramEntityAboutToRemove(ChromaId id,
                                       EntityType type,
                                       TEntityUid eId);
  void chromatogramModelAboutToReset(ChromaId id);
  void chromatogramModelReset(ChromaId id);


private:
  void connectChroma(ChromatogrammModelPtr chroma);
  void disconnectChroma(ChromatogrammModelPtr chroma);

  QHash<ChromaId, ChromatogrammModelPtr> mChromatogramms;
  static QHash<ChromaId, CurveDataModelPtr> mCurves;
  QHash<uint, QVariant> mData;
  /*QPointer<*/MarkupStepController/*>*/* mController;

};

using MarkupModelPtr = QSharedPointer<MarkupDataModel>;
}//GenesisMarkup
#endif // MARKUPDATAMODEL_H
