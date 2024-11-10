#ifndef ABSTRACTDATAMODEL_H
#define ABSTRACTDATAMODEL_H

#include <QObject>
#include <QVariant>
#include "json_serializable.h"

using DataRoleType = uint32_t;
using EntityType = uint32_t;
using TEntityUid = QUuid;
class DataModel : public QHash<DataRoleType, QVariant>
{
public:
  //just to be shure
  DataModel(DataModel::Iterator begin, DataModel::Iterator end) : QHash(begin, end){};
  DataModel(DataModel::ConstIterator begin, DataModel::ConstIterator end) : QHash(begin, end){};
  DataModel(QHash<DataRoleType, QVariant> &&other) : QHash(other){};
  DataModel(const QHash<DataRoleType, QVariant> &&other) : QHash(other){};
  DataModel(std::initializer_list<std::pair<DataRoleType, QVariant>> list) : QHash(list){};
  DataModel() : QHash(){};
  DataModel(DataModel &other) : QHash(other){};
  DataModel(const DataModel &other) : QHash(other){};

  QVariant getData(DataRoleType role) const
  {
    auto iter = constFind(role);
    if(iter == constEnd())
      return QVariant();
    return *iter;
  }

  bool hasData(DataRoleType role) const
  {
    auto iter = constFind(role);
    if(iter == constEnd())
      return false;
    return true;
  }

  bool hasDataAndItsValid(DataRoleType role) const
  {
    auto iter = constFind(role);
    if(iter == constEnd())
      return false;
    return iter->isValid();
  }

  EntityType getType() const;
  TEntityUid getUid() const;
};

class AbstractEntityDataModel : public QObject, public JsonSerializable
{
  Q_OBJECT

public:
  enum EntityDataRoles
  {
    RoleEntityType, //0
    RoleEntityUid, //1
    RoleEntityLast  //2
  };
  Q_ENUM(EntityDataRoles)

  using ConstDataPtr = QSharedPointer<const DataModel>;
  using DataPtr = QSharedPointer<DataModel>;
public:
  AbstractEntityDataModel(QObject *parent = nullptr);
  AbstractEntityDataModel(const AbstractEntityDataModel& other);

  virtual ~AbstractEntityDataModel() = default;

  virtual ConstDataPtr getEntity(TEntityUid entityId) const;
  virtual QList<ConstDataPtr> getEntities(EntityType type) const;
  //filter is pair of data role and it's value
  virtual QList<ConstDataPtr> getEntities(const QList<QPair<int, QVariant>>& filters) const;
  virtual QList<ConstDataPtr> getEntities(std::function<bool(ConstDataPtr)> predicate) const;
  virtual QSet<TEntityUid> getEntitiesUidsSet(std::function<bool(ConstDataPtr)> predicate) const;
  virtual QList<TEntityUid> getUIdListOfEntities(EntityType type) const;
  virtual int countEntities(EntityType type) const;
  virtual int countEntities(const QList<QPair<int, QVariant>>& filters) const;
  virtual int countEntities(std::function<bool(ConstDataPtr)> predicate) const;

  virtual bool setEntityData(TEntityUid Id, DataRoleType role, const QVariant& value);
  virtual bool resetEntityData(TEntityUid Id, const DataModel& newData = DataModel());

  virtual TEntityUid addNewEntity(EntityType type, const DataModel& data, TEntityUid uid = TEntityUid());
  virtual TEntityUid addNewEntity(EntityType type, DataModel* data = nullptr, TEntityUid uid = TEntityUid());//if nullptr, will create a new empty one
  virtual bool removeEntity(TEntityUid Id);

  virtual const QHash<TEntityUid, DataPtr>& getData() const;
  virtual void setData(const QHash<TEntityUid, DataPtr>& newData);
  virtual void clearData();
  virtual void clearEntities(EntityType type);
  virtual void removeEntities(const QList<QPair<int, QVariant>>& filters);
  virtual void removeEntitiesIf(std::function<bool(ConstDataPtr)> predicate);

  static void copyEntitiesIf(AbstractEntityDataModel* from, AbstractEntityDataModel* to, std::function<bool(ConstDataPtr)> predicate);

signals:
  void entityChanged(EntityType type,
                     TEntityUid eId,
                     DataRoleType role,
                     const QVariant& value);

  void entityResetted(EntityType type,
                      TEntityUid eId,
                      AbstractEntityDataModel::ConstDataPtr data);

  void entityAdded(EntityType type,
                   TEntityUid eId,
                   AbstractEntityDataModel::ConstDataPtr data);

  void entityAboutToRemove(EntityType type,
                     TEntityUid eId);
  void modelAboutToReset();
  void modelReset();

protected:
  void beginResetModel();
  void endResetModel();

private:
  QHash<TEntityUid, DataPtr> mData;
  QHash<EntityType, QSet<DataPtr>> mTypeToDataCache;

};

#endif // ABSTRACTDATAMODEL_H
