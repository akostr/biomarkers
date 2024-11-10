#ifndef ANALYSIS_PLOT_ITEM_MODEL_H
#define ANALYSIS_PLOT_ITEM_MODEL_H

#include <QAbstractItemModel>
#include <logic/models/analysis_entity_model.h>

struct AnalysisItem;
class AnalysisPlotItemModel : public QAbstractItemModel
{
  Q_OBJECT

public:
  enum Columns
  {
    columnTitle,
    columnColor,
    columnShape,
    columnSize,
    columnLast
  };
  enum Roles
  {
    UidRole = Qt::UserRole + 1,
    TitleRole,
    ColorRole,
    ShapeRole,
    ShapeSizeRole
  };

  explicit AnalysisPlotItemModel(QObject *parent = nullptr);
  ~AnalysisPlotItemModel();
  void setEntityModel(AnalysisEntityModel* entityModel);

  // Basic functionality:
  QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
  QModelIndex parent(const QModelIndex &index) const override;

  int rowCount(const QModelIndex &parent = QModelIndex()) const override;
  int columnCount(const QModelIndex &parent = QModelIndex()) const override;

  QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
  QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

  // Editable:
  bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

  Qt::ItemFlags flags(const QModelIndex &index) const override;

  Qt::DropActions supportedDropActions() const override;
  Qt::DropActions supportedDragActions() const override;

  AnalysisItem* itemFromIndex(const QModelIndex& index) const;
  QModelIndex indexFromItem(AnalysisItem* item, int column = 0) const;

private:
  void clearItems();
  void clearRecursive(AnalysisItem* item);;
  void connectSignals(AnalysisEntityModel* model);
  void disconnectSignals(AnalysisEntityModel* model);
  AnalysisItem* parseItem(AbstractEntityDataModel::ConstDataPtr ent, AnalysisItem* parent);;
  AnalysisItem* parseGroup(AbstractEntityDataModel::ConstDataPtr ent, AnalysisItem* parent);;

private slots:
  void onModelDataChanged(const QHash<int, QVariant>& roleDataMap);
  // void onGroupColorChanged(QSet<TEntityUid> affectedEntities);
  // void onGroupShapeChanged(QSet<TEntityUid> affectedEntities);
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

private:
  AnalysisEntityModel* mEntityModel = nullptr;
  static const int sColorGroupsRow = 0;
  static const int sShapesGroupsRow = 1;
  QList<AnalysisItem*> mTopLvlItems;
  AnalysisItem* mColorGroupsParentItem = nullptr;
  AnalysisItem* mShapeGroupsParentItem = nullptr;
  AnalysisItem* mGroupsParentItem = nullptr;
  QMap<TEntityUid, QList<AnalysisItem*>> mUidToItemMap;
  bool isEntityModelInResetState = false;
};

struct AnalysisItem
{
public:
  TEntityUid entityUid;
  QString selfTitle;
  AnalysisItem* parent;
  QList<AnalysisItem*> children;
};

#endif // ANALYSIS_PLOT_ITEM_MODEL_H
