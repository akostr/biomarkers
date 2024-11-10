#ifndef CHROMATOGRAMPASSPORTMODEL_H
#define CHROMATOGRAMPASSPORTMODEL_H

#include "abstract_data_model.h"

namespace GenesisMarkup{

class ChromatogramPassportModel : public AbstractEntityDataModel
{
  Q_OBJECT
public:
  ChromatogramPassportModel();
  ChromatogramPassportModel(const ChromatogramPassportModel& other);
public:
  ConstDataPtr getPassport(int chromatogrammId) const;
  // JsonSerializable interface
public:
  void load(const QJsonObject &data) override;
  QJsonObject save() const override;

  bool setPassportData(int chromaId, DataRoleType role, const QVariant& value);
  bool resetPassportData(int chromaId, const DataModel &newData);
  void addNewPassport(int chromaId);
  bool removePassport(int chromaId);

  QList<int> getIdList() const;
  // AbstractEntityDataModel interface
  // move default access functions to private to moc it
private:
  bool setEntityData(TEntityUid Id, DataRoleType role, const QVariant &value) override;
  bool resetEntityData(TEntityUid Id, const DataModel &newData) override;
  QUuid addNewEntity(EntityType type, const DataModel &data, QUuid uid = QUuid()) override;
  QUuid addNewEntity(EntityType type, DataModel *data = nullptr, QUuid uid = QUuid()) override;
  bool removeEntity(TEntityUid Id) override;
  ConstDataPtr getEntity(TEntityUid entityId) const override;
//  const QHash<EntityUId, DataPtr> &getData() const override;
//  void setData(const QHash<EntityUId, DataPtr> &newData) override;
//  void clearData() override;

  QHash<int, QUuid> mIdToUidMap;
};
}//namespace GenesisMarkup
#endif // CHROMATOGRAMPASSPORTMODEL_H
