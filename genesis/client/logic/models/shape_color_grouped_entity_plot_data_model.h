#ifndef SHAPE_COLOR_GROUPED_ENTITY_PLOT_DATA_MODEL_H
#define SHAPE_COLOR_GROUPED_ENTITY_PLOT_DATA_MODEL_H

#include <logic/markup/abstract_data_model.h>

#include <QPainterPath>
#include <QColor>

namespace AnalysisEntity
{
Q_NAMESPACE
enum EntityTypes
{
  // TypeSample,
  // TypePeak,
  TypeUnknown = 0,
  TypeColorGroup,//1
  TypeShapeGroup,//2
  TypeGroup,
  TypePeak,
  TypeSample,
  TypeHottelingGroup,
  TypeLine,
  TypeCurve,
  TypeLast
};
Q_ENUM_NS(EntityTypes)
enum EntityDataRoles
{
  RoleShape = AbstractEntityDataModel::RoleEntityLast + 1,//3
  RoleShapePixelSize,//4
  RoleColor,//5
  RoleGroupTooltip,//6
  RoleTitle,//7
  RoleGroupsUidSet,//8
  RoleForceShowName,
  RoleForceShowPassport,
  RoleGroupUserCreated,
  RolePassport,
  RolePassportFilter,
  RolePeakData,
  RoleComponentsData,
  RoleNormedComponentData,
  RoleExcluded,
  RoleIntId,
  RoleOrderNum,
  RoleLast
};
Q_ENUM_NS(EntityDataRoles)

enum AnalysisModelDataRoles
{
  ModelRoleCurrentXComponentNum,
  ModelRoleCurrentYComponentNum,
  ModelRoleCurrentSpectralComponentNum,
  ModelRoleComponentsExplVarianceMap,
  ModelRoleColorFilter,
  ModelRoleShapeFilter,
  ModelRoleIsSpectral,
  ModelRoleSpectralRenderType,
  ModelRolePassportKeys,
  ModelRolePassportHeaders,
  ModelRoleLast
};
Q_ENUM_NS(AnalysisModelDataRoles)

enum RenderType
{
  Line,
  Dot
};
Q_ENUM_NS(RenderType)

struct CurveData
{
  RenderType renderType;
  QVector<double> keys;
  QVector<double> values;
};

struct ShapeDescriptor
{
  QPainterPath path;
  int pixelSize;
};

struct EntityPassport
{
  QString field;
  QString well;
  QString wellCluster;
  QString layer;
  QString date;
  QString depth;
  QString filetitle;
  QString filename;
  int fileId;
};

struct PassportFilter : public QVariantMap
{
  PassportFilter() = default;
  bool match(const QVariantMap& passport) const
  {
    for(auto it = constBegin(); it != constEnd(); it++)
    {
      auto value = passport.value(it.key(), QVariant());
      if(!value.isValid() || value != it.value())
        return false;
    }
    return true;
  };
  PassportFilter concatMissed(const PassportFilter& other)
  {
    for(auto it = other.constBegin(); it != other.constEnd(); it++)
    {
      if(!contains(it.key()))
        insert(it.key(), it.value());
    }
    return *this;
  }
};

struct PeakData
{
  QString title;
  int orderNum;
};

struct LegendGroup
{
  QString title;
  QString tooltip;
  QColor color;
  ShapeDescriptor shape;
  PassportFilter filter;
  QUuid uid;
};

}
using TEntityType = AnalysisEntity::EntityTypes;
using TForceShowName = bool;
using TForceShowPassport = bool;
using TCurrentComponentNum = int;
using TGroupUserCreated = bool;
using TExcluded = bool;
using TIntId = int;
using TOrderNum = int;
using TComponentsExplVarianceMap = QHash<int, double>;
using TComponentsData = QHash<int, double>;
using TNormedComponentsData = QHash<int, double>;
using TPassport = QVariantMap;
using TPassportFilter = AnalysisEntity::PassportFilter;
using TPeakData = AnalysisEntity::PeakData;
using TLegendGroup = AnalysisEntity::LegendGroup;
using TShape = QPainterPath;
using TShapePixelSize = int;
using TColor = QColor;
using TGroupName = QString;
using TGroupsUidSet = QSet<TEntityUid>;
using TCurveData = AnalysisEntity::CurveData;
using TIsSpectral = bool;
using TSpectralRenderType = AnalysisEntity::RenderType;
using TPassportKeys = QStringList;
using TPassportHeaders = QStringList;

namespace PassportTags
{
const QString field("field_title");
const QString wellCluster("well_cluster_title");
const QString well("well_title");
const QString depth("depth");
const QString depthTVD("depth_tvd");
const QString geocomplexTitle("geo_complex_title");
const QString geothicknessTitle("geo_thickness_title");
const QString layer("layer_title");
const QString date("date");
const QString lithology("lithology");
const QString comment("comment");
const QString filename("filename");
const QString filetitle("filetitle");
const QString fileId("file_id");
}

class ShapeColorGroupedEntityPlotDataModel : public AbstractEntityDataModel
{
  Q_OBJECT
public:
  ShapeColorGroupedEntityPlotDataModel(QObject *parent = nullptr);

  QColor getActualColor(TEntityUid uid);
  QPainterPath getActualShape(TEntityUid uid);
  int getActualShapeSize(TEntityUid uid);
  AnalysisEntity::ShapeDescriptor getShapeDescriptor(TEntityUid uid);
  //функции для работы с группами обходят проверки, которые проводятся в
  //функциях добавления/модификации/удаления для избежания ненужных операций.
  //эти функции работают в предположении, что сущности не могут содержать
  //не валидные uid-ы групп
  bool setEntityGroup(TEntityUid entityUid, TEntityUid groupEntityUid);
  bool addEntitiesToGroup(QSet<TEntityUid> entityUids, TEntityUid groupEntityUid);
  bool resetEntityGroup(TEntityUid entityUid, AnalysisEntity::EntityTypes groupEntityType);
  bool resetEntityGroup(TEntityUid entityUid, TEntityUid groupEntityUid);

  // AbstractEntityDataModel interface
public:
  //переопределение функций добавления, модификации и удаления для сохранения актуальности
  //списков uid-ов груп. Если группа удаляется, удаляем ее uid из всех
  //сущностей, которые его хранят. Если добавляется или модифицируется сущность со
  //списком uid-ов групп, удаляем оттуда не-групповые uid-ы и uid-ы несуществующих групп
  bool setEntityData(TEntityUid Id, DataRoleType role, const QVariant &value) override;
  TEntityUid addNewEntity(EntityType type, const DataModel &data, QUuid uid = QUuid()) override;
  TEntityUid addNewEntity(EntityType type, DataModel *data = nullptr, QUuid uid = QUuid()) override;
  bool removeEntity(TEntityUid Id) override;
  void removeEntities(const QList<QPair<int, QVariant> > &filters) override;
  void removeEntitiesIf(std::function<bool (ConstDataPtr)> predicate) override;

private:
  template<typename T>
  T getGroupValue(int role, TEntityUid uid, QList<int> groupTypes = {AnalysisEntity::TypeGroup, AnalysisEntity::TypeHottelingGroup})
  {//if entity not in group, it will be entity value itself
    auto ent = getEntity(uid);
    if(!ent)
      return T();
    if(ent->hasDataAndItsValid(AnalysisEntity::RoleGroupsUidSet))
    {
      auto uids = ent->getData(AnalysisEntity::RoleGroupsUidSet).value<TGroupsUidSet>();
      QList<QPair<int, T>> prioritizedArr;
      for(auto& groupUid : uids)
      {
        auto groupEnt = getEntity(groupUid);
        if(!groupEnt)
          continue;
        auto groupType = groupEnt->getType();
        if(!groupTypes.contains(groupType))
          continue;
        else
        {
          QPair<int, T> value(groupType, getGroupValue<T>(role, groupUid));
          if(prioritizedArr.isEmpty())
          {
            prioritizedArr.append(value);
          }
          else
          {
            auto cmpType = [](QPair<int, T> a, QPair<int, T> b) { return a.first < b.first; };
            auto it = lower_bound(prioritizedArr.begin(), prioritizedArr.end(), value, cmpType);
            prioritizedArr.insert(it, value);
          }
        }
      }
      if(prioritizedArr.isEmpty())
        return ent->getData(role).value<T>();
      return prioritizedArr.last().second;//чем больше значение енумератора (TypeHottelingGroup > TypeGroup)
                                          //тем выше у его значения приоритет. Поменяйте местами енумераторы
                                          //что бы поменять приоритеты
    }
    return ent->getData(role).value<T>();
  }
  TEntityUid getCurrentValueGroup(TEntityUid uid);

signals:
  void groupColorChanged(QSet<TEntityUid> affectedEntities);
  void groupShapeChanged(QSet<TEntityUid> affectedEntities);

private:
  void validateGroupsUidsSet(TGroupsUidSet& set);
};

#endif // SHAPE_COLOR_GROUPED_ENTITY_PLOT_DATA_MODEL_H
