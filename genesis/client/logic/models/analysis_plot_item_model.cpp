#include "analysis_plot_item_model.h"
#include <logic/models/analysis_entity_model.h>
#include <QPixmap>
#include <QIcon>
#include <QPainter>

using namespace AnalysisEntity;

AnalysisPlotItemModel::AnalysisPlotItemModel(QObject *parent)
  : QAbstractItemModel(parent)
{}

AnalysisPlotItemModel::~AnalysisPlotItemModel()
{
  clearItems();
}

void AnalysisPlotItemModel::setEntityModel(AnalysisEntityModel *entityModel)
{
  mEntityModel = entityModel;
  onModelReset();
}

QModelIndex AnalysisPlotItemModel::index(int row, int column, const QModelIndex &parent) const
{
  if(row > rowCount(parent) || column > columnCount(parent)
      || row < 0 || column < 0)
    return QModelIndex();

  if(!parent.isValid())
  {//top lvl
    return createIndex(row, column, mTopLvlItems[row]);
  }
  else
  {
    auto item = itemFromIndex(parent);
    return createIndex(row, column, item->children[row]);
  }
}

QModelIndex AnalysisPlotItemModel::parent(const QModelIndex &index) const
{
  if(!index.isValid())
    return QModelIndex();
  auto item = itemFromIndex(index);
  if(!item->parent)
  {//topLvl
    return QModelIndex();
  }
  if(!item->parent->parent)
  {//looking for group parent
    auto iter = std::find(mTopLvlItems.begin(), mTopLvlItems.end(), item->parent);
    if(iter == mTopLvlItems.end())
      return QModelIndex();
    int parentRow = std::distance(mTopLvlItems.begin(), iter);
    return createIndex(parentRow, index.column(), item->parent);
  }
  //looking parent group for group member
  {
    auto parentParent = item->parent->parent;
    auto iter = std::find(parentParent->children.begin(), parentParent->children.end(), item->parent);
    if(iter == parentParent->children.end())
      return QModelIndex();
    int parentRow = std::distance(parentParent->children.begin(), iter);
    return createIndex(parentRow, index.column(), item->parent);
  }
  return QModelIndex();
}

int AnalysisPlotItemModel::rowCount(const QModelIndex &parent) const
{
  if(!parent.isValid())
  {
    return mTopLvlItems.size();
  }
  else
  {
    auto item = itemFromIndex(parent);
    return item->children.size();
  }
}

int AnalysisPlotItemModel::columnCount(const QModelIndex &parent) const
{
  return columnLast;
  // if (!parent.isValid())
  //   return 0;

  // FIXME: Implement me!
}

QVariant AnalysisPlotItemModel::data(const QModelIndex &index, int role) const
{
  if(!mEntityModel)
    return QVariant();
  if(!index.isValid())
    return QVariant();
  auto item = itemFromIndex(index);

  switch((Columns)index.column())
  {
  case columnTitle:
    if(role == Qt::DisplayRole || role == Qt::EditRole)
      return item->selfTitle;
    break;
  case columnColor:
    if(role == Qt::DisplayRole || role == Qt::EditRole)
      return QVariant();
    if(role == Qt::DecorationRole
        && !item->entityUid.isNull())
    {
      if(item->parent && !item->parent->entityUid.isNull())
      {
        auto parentEnt = mEntityModel->getEntity(item->parent->entityUid);
        auto type = parentEnt->getType();
        if(type == TypeColorGroup)
          return QVariant();
      }
      auto color = mEntityModel->getActualColor(item->entityUid);
      if(color.isValid())
        return color;
    }
    break;
  case columnShape:
    if(role == Qt::DisplayRole || role == Qt::EditRole)
      return QVariant();
    else if(role == Qt::DecorationRole)
    {
      if(item->parent && !item->parent->entityUid.isNull())
      {
        auto parentEnt = mEntityModel->getEntity(item->parent->entityUid);
        auto type = parentEnt->getType();
        if(type == TypeShapeGroup)
          return QVariant();
      }
      QPixmap pmap(64, 64);
      pmap.fill(QColor(0,0,0,0));
      QPainter painter(&pmap);
      painter.setPen(Qt::NoPen);
      painter.setBrush(QColor(0,0,0,255));
      painter.scale(64,64);
      painter.drawPath(mEntityModel->getActualShape(item->entityUid));
      return QIcon(pmap);
    }
    break;
  case columnSize:
    if(role == Qt::DisplayRole || role == Qt::EditRole)
    {
      if(item->entityUid.isNull())
        return QVariant();//top lvl group item
      auto ent = mEntityModel->getEntity(item->entityUid);
      auto type = ent->getType();

      if(type == TypePeak
          || type == TypeSample)
      {
        if(item->parent && !item->parent->entityUid.isNull())
        {
          auto parentEnt = mEntityModel->getEntity(item->parent->entityUid);
          auto parentType = parentEnt->getType();
          if(parentType == TypeShapeGroup)
            return QVariant();//member of shape group
          else if(parentType == TypeColorGroup
                   || parentType == TypeGroup)
            return mEntityModel->getActualShapeSize(item->entityUid);
        }
        else if(!item->parent)
        {
          return mEntityModel->getActualShapeSize(item->entityUid);
        }
      }
      else if(type == TypeColorGroup)
        return QVariant();//color group item
      else if(type == TypeShapeGroup)
        return mEntityModel->getActualShapeSize(item->entityUid);
      break;
    }
  case columnLast:
    break;
  }
  return QVariant();
}

bool AnalysisPlotItemModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
  if(!index.isValid())
    return false;
  auto item = itemFromIndex(index);
  switch((Columns)index.column())
  {
  case columnTitle:
    if(role == Qt::DisplayRole || role == Qt::EditRole)
    {
      auto ent = mEntityModel->getEntity(item->entityUid);
      QSet<int> editableTypes = {TypeShapeGroup,
                                 TypeColorGroup,
                                 TypeGroup};
      auto entityType = ent->getType();

      if(!editableTypes.contains(entityType))
        return false;
      mEntityModel->setEntityData(item->entityUid, RoleTitle, value);
      return true;
    }
    break;
  case columnColor:
    if(role == Qt::EditRole)
    {
      mEntityModel->setEntityData(item->entityUid, RoleColor, value);
      return true;
    }
    break;
  case columnShape:
    if(role == Qt::EditRole)
    {
      mEntityModel->setEntityData(item->entityUid, RoleShape, value);
      return true;
    }
    break;
  case columnSize:
    if(role == Qt::EditRole)
    {
      mEntityModel->setEntityData(item->entityUid, RoleShapePixelSize, value.toInt());
      return true;
    }
    break;
  case columnLast:
    break;
  }

  // model have not it's own data, all changes notifiyng handles by entity model
  // emit dataChanged(index, index, {role});
  return false;
}

Qt::ItemFlags AnalysisPlotItemModel::flags(const QModelIndex &index) const
{
  if(!mEntityModel)
    return Qt::ItemFlags();
  Qt::ItemFlags flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable;
  if(!index.isValid())
    return flags | Qt::ItemIsDropEnabled;
  auto item = itemFromIndex(index);
  if (!index.parent().isValid())
  {//top lvl items
    if(!item->entityUid.isNull())
    {
      auto ent = mEntityModel->getEntity(item->entityUid);
      auto type = ent->getType();
      if(type == TypePeak
          || type == TypeSample)
      {//groupless items, can be dragged, any column except title can be edited
        flags |= Qt::ItemIsDragEnabled;
        if(index.column() != columnTitle)
          flags |= Qt::ItemIsEditable;
      }
      //else it is color/shape topLvl item, not editable and not draggable
    }
  }
  else
  {
    auto ent = mEntityModel->getEntity(item->entityUid);
    auto type = ent->getType();
    if(type == TypeColorGroup)
    {//color group item, not draggable, accept drops, title and color editable
      flags |= Qt::ItemIsDropEnabled;
      if(index.column() == columnTitle
          || index.column() == columnColor)
        flags |= Qt::ItemIsEditable;
    }
    else if (type == TypeShapeGroup)
    {//shape group item, not draggable, accept drops, title and shape editable
      flags |= Qt::ItemIsDropEnabled;
      if(index.column() == columnTitle
          || index.column() == columnShape
          || index.column() == columnSize)
        flags |= Qt::ItemIsEditable;
    }
    else if(type == TypePeak
               || type == TypeSample)
    {//grouped items, draggable, not editable
      flags |= Qt::ItemIsDragEnabled;
    }
  }
  return flags;
}

AnalysisItem *AnalysisPlotItemModel::itemFromIndex(const QModelIndex &index) const
{
  if(index.isValid())
    return (AnalysisItem*)index.internalPointer();
  else
    return nullptr;
}

QModelIndex AnalysisPlotItemModel::indexFromItem(AnalysisItem *item, int column) const
{
  if(!item)
    return QModelIndex();
  if(!item->parent)
  {
    auto iter = std::find(mTopLvlItems.begin(), mTopLvlItems.end(), item);
    if(iter == mTopLvlItems.end())
      return QModelIndex();

    return createIndex(std::distance(mTopLvlItems.begin(), iter), column, item);
  }
  else
  {
    auto iter = std::find(item->parent->children.begin(), item->parent->children.end(), item);
    if(iter == item->parent->children.end())
      return QModelIndex();
    return createIndex(std::distance(item->parent->children.begin(), iter), column, item);
  }
  return QModelIndex();
}

void AnalysisPlotItemModel::clearItems()
{
  for(auto& item : mTopLvlItems)
  {
    clearRecursive(item);
  }
  mTopLvlItems.clear();
  mUidToItemMap.clear();
  mColorGroupsParentItem = nullptr;
  mShapeGroupsParentItem = nullptr;
}

void AnalysisPlotItemModel::clearRecursive(AnalysisItem *item)
{
  for(auto& child : item->children)
  {
    clearRecursive(child);
  }
  item->children.clear();
  delete item;
}

void AnalysisPlotItemModel::connectSignals(AnalysisEntityModel *model)
{
  if(!model)
    return;
  disconnect(model, &AnalysisEntityModel::modelDataChanged, this, &AnalysisPlotItemModel::onModelDataChanged);
  // disconnect(model, &ShapeColorGroupedEntityPlotDataModel::groupColorChanged, this, &AnalysisPlotItemModel::onGroupColorChanged);
  // disconnect(model, &ShapeColorGroupedEntityPlotDataModel::groupShapeChanged, this, &AnalysisPlotItemModel::onGroupShapeChanged);
  disconnect(model, &AbstractEntityDataModel::entityChanged, this, &AnalysisPlotItemModel::onEntityChanged);
  disconnect(model, &AbstractEntityDataModel::entityResetted, this, &AnalysisPlotItemModel::onEntityResetted);
  disconnect(model, &AbstractEntityDataModel::entityAdded, this, &AnalysisPlotItemModel::onEntityAdded);
  disconnect(model, &AbstractEntityDataModel::entityAboutToRemove, this, &AnalysisPlotItemModel::onEntityAboutToRemove);
  disconnect(model, &AbstractEntityDataModel::modelAboutToReset, this, &AnalysisPlotItemModel::onModelAboutToReset);
  disconnect(model, &AbstractEntityDataModel::modelReset, this, &AnalysisPlotItemModel::onModelReset);
}

void AnalysisPlotItemModel::disconnectSignals(AnalysisEntityModel *model)
{
  if(!model)
    return;
  connect(model, &AnalysisEntityModel::modelDataChanged, this, &AnalysisPlotItemModel::onModelDataChanged);
  // connect(model, &ShapeColorGroupedEntityPlotDataModel::groupColorChanged, this, &AnalysisPlotItemModel::onGroupColorChanged);
  // connect(model, &ShapeColorGroupedEntityPlotDataModel::groupShapeChanged, this, &AnalysisPlotItemModel::onGroupShapeChanged);
  connect(model, &AbstractEntityDataModel::entityChanged, this, &AnalysisPlotItemModel::onEntityChanged);
  connect(model, &AbstractEntityDataModel::entityResetted, this, &AnalysisPlotItemModel::onEntityResetted);
  connect(model, &AbstractEntityDataModel::entityAdded, this, &AnalysisPlotItemModel::onEntityAdded);
  connect(model, &AbstractEntityDataModel::entityAboutToRemove, this, &AnalysisPlotItemModel::onEntityAboutToRemove);
  connect(model, &AbstractEntityDataModel::modelAboutToReset, this, &AnalysisPlotItemModel::onModelAboutToReset);
  connect(model, &AbstractEntityDataModel::modelReset, this, &AnalysisPlotItemModel::onModelReset);
}

AnalysisItem *AnalysisPlotItemModel::parseItem(AbstractEntityDataModel::ConstDataPtr ent, AnalysisItem *parent)
{
  auto item = new AnalysisItem();
  if(parent)
  {
    item->parent = parent;
    parent->children << item;
  }
  item->entityUid = ent->getUid();
  mUidToItemMap[item->entityUid] << item;
  switch(ent->getType())
  {
  case TypePeak:
    item->selfTitle = ent->getData(RolePeakData).value<TPeakData>().title;
    break;
  case TypeSample:
  {
    auto passport = ent->getData(RolePassport).value<TPassport>();
    item->selfTitle = passport.value(PassportTags::filetitle).toString();
    if(item->selfTitle.isEmpty())
    {
      item->selfTitle = passport.value(PassportTags::filename).toString().section('/', -1);
      if(item->selfTitle.contains('\\'))
        item->selfTitle = item->selfTitle.section('\\', -1);
    }
    else
    {
      item->selfTitle = passport.value(PassportTags::filetitle).toString();
    }
    break;
  }
  default:
    break;
  }
  return item;
}

AnalysisItem *AnalysisPlotItemModel::parseGroup(AbstractEntityDataModel::ConstDataPtr ent, AnalysisItem *parent)
{
  auto group = new AnalysisItem();
  group->selfTitle = ent->getData(RoleTitle).toString();
  group->entityUid = ent->getUid();
  mUidToItemMap[group->entityUid] << group;
  if(parent)
  {
    group->parent = parent;
    parent->children << group;
  }
  auto childEntities = mEntityModel->getEntities(
      [group](AbstractEntityDataModel::ConstDataPtr data)->bool
      {
        if(data->getData(RoleGroupsUidSet).
            value<TGroupsUidSet>().
            contains(group->entityUid))
          return true;
        return false;
      });
  for(auto& ent : childEntities)
    parseItem(ent, group);
  return group;
}

void AnalysisPlotItemModel::onModelDataChanged(const QHash<int, QVariant> &roleDataMap)
{
  if(isEntityModelInResetState)
    return;
  //do nothing for now
}

// void AnalysisPlotItemModel::onGroupColorChanged(QSet<TEntityUid> affectedEntities)
// {

// }

// void AnalysisPlotItemModel::onGroupShapeChanged(QSet<TEntityUid> affectedEntities)
// {

// }

void AnalysisPlotItemModel::onEntityChanged(EntityType type, TEntityUid eId, DataRoleType role, const QVariant &value)
{
  if(isEntityModelInResetState)
    return;
  auto itemsList = mUidToItemMap.value(eId, {});
  if(itemsList.isEmpty())
    return;
  switch(role)
  {
  case RolePassport:
  {
    if(type != TypeSample)
      break;
    for(auto item : itemsList)
    {
      auto passport = value.value<TPassport>();
      item->selfTitle = passport.value(PassportTags::filetitle).toString();
      if(item->selfTitle.isEmpty())
      {
        item->selfTitle = passport.value(PassportTags::filename).toString().section('/', -1);
        if(item->selfTitle.contains('\\'))
          item->selfTitle = item->selfTitle.section('\\', -1);
      }
      else
      {
        item->selfTitle = passport.value(PassportTags::filetitle).toString();
      }
      auto ind = indexFromItem(item, columnTitle);
      emit dataChanged(ind, ind, {Qt::DisplayRole, Qt::EditRole});
    }
    break;
  }
  case RolePeakData:
  {
    if(type != TypePeak)
      break;
    for(auto item : itemsList)
    {
      item->selfTitle = value.value<TPeakData>().title;
      auto ind = indexFromItem(item, columnTitle);
      emit dataChanged(ind, ind, {Qt::DisplayRole, Qt::EditRole});
    }
    break;
  }
  case RoleShape:
  {
    for(auto item : itemsList)
    {
      auto ind = indexFromItem(item, columnShape);
      emit dataChanged(ind, ind, {Qt::DecorationRole});
    }
    break;
  }
  case RoleShapePixelSize:
  {
    for(auto item : itemsList)
    {
      auto ind = indexFromItem(item, columnSize);
      emit dataChanged(ind, ind, {Qt::DisplayRole, Qt::EditRole});
    }
    break;
  }
  case RoleColor:
  {
    for(auto item : itemsList)
    {
      auto ind = indexFromItem(item, columnColor);
      emit dataChanged(ind, ind, {Qt::DecorationRole});
    }
    break;
  }
  case RoleTitle:
  {
    if(type != TypeColorGroup
        && type != TypeShapeGroup
        && type != TypeGroup)
      break;
    for(auto item : itemsList)
    {
      item->selfTitle = value.toString();
      auto ind = indexFromItem(item, columnTitle);
      emit dataChanged(ind, ind, {Qt::DisplayRole, Qt::EditRole});
    }
    break;
  }
  case RoleGroupsUidSet:
  {
    if(type != TypePeak
        && type != TypeSample)
      break;
    auto newGroups = value.value<TGroupsUidSet>();
    auto items = mUidToItemMap.value(eId, {});
    TGroupsUidSet oldGroups;
    QSet<AnalysisItem*> toRemove;
    for(auto& item : items)
    {
      if(item->parent)
      {
        oldGroups << item->parent->entityUid;
      }
      else
      {
        if(!newGroups.isEmpty())
          toRemove << item;
      }
    }
    if(newGroups == oldGroups)
      break;
    auto removeFrom = oldGroups;
    removeFrom.subtract(newGroups);
    auto addTo = newGroups;
    addTo.subtract(oldGroups);
    for(auto& uid : removeFrom)
    {
      for(auto& item : mUidToItemMap.value(uid, {}))
        toRemove << item;
    }
    for(auto& item : toRemove)
    {
      auto index = indexFromItem(item);
      beginRemoveRows(index.parent(), index.row(), index.row());
      if(item->parent)
        item->parent->children.removeOne(item);
      else
        mTopLvlItems.removeOne(item);
      delete item;
      endRemoveRows();
    }
    if(!newGroups.isEmpty())
    {
      for(auto& uid : addTo)
      {
        for(auto& item : mUidToItemMap.value(uid, {}))
        {
          auto index = indexFromItem(item);
          beginInsertRows(index, item->children.size(), item->children.size());
          parseItem(mEntityModel->getEntity(eId), item);
          endInsertRows();
        }
      }
    }
    else
    {
      beginInsertRows(QModelIndex(), mTopLvlItems.size(), mTopLvlItems.size());
      mTopLvlItems << parseItem(mEntityModel->getEntity(eId), nullptr);
      endInsertRows();
    }
    break;
  }
  default:
    break;
  }
}

void AnalysisPlotItemModel::onEntityResetted(EntityType type, TEntityUid eId, AbstractEntityDataModel::ConstDataPtr data)
{
  if(isEntityModelInResetState)
    return;
  switch(type)
  {
  case TypePeak:
  case TypeSample:
  {
    beginResetModel();
    for(auto& item : mUidToItemMap.value(eId, {}))
    {
      if(item->parent)
      {
        item->parent->children.removeOne(item);
      }
      else
      {
        mTopLvlItems.removeOne(item);
      }
      clearRecursive(item);
    }

    auto groups = data->getData(RoleGroupsUidSet).value<TGroupsUidSet>();
    if(groups.isEmpty())
    {
      mTopLvlItems << parseItem(data, nullptr);
    }
    else
    {
      for(auto& uid : groups)
      {
        for(auto& group : mUidToItemMap.value(uid, {}))
        {
          parseItem(data, group);
        }
      }
    }

    endResetModel();
    break;
  }
  case TypeShapeGroup:
  case TypeColorGroup:
  case TypeGroup:
  {
    for(auto& group : mUidToItemMap.value(eId, {}))
    {
      group->selfTitle = data->getData(RoleTitle).toString();
      auto index = indexFromItem(group, columnTitle);
      emit dataChanged(index, index, {Qt::DisplayRole, Qt::EditRole});
    }
    break;
  }
  default:
    break;
  }

}

void AnalysisPlotItemModel::onEntityAdded(EntityType type, TEntityUid eId, AbstractEntityDataModel::ConstDataPtr data)
{
  if(isEntityModelInResetState)
    return;
  switch(type)
  {
  case TypePeak:
  case TypeSample:
  {
    auto groups = data->getData(RoleGroupsUidSet).value<TGroupsUidSet>();
    if(groups.isEmpty())
    {
      beginInsertRows(QModelIndex(), mTopLvlItems.size(), mTopLvlItems.size());
      mTopLvlItems << parseItem(data, nullptr);
      endInsertRows();
    }
    else
    {
      for(auto& uid : groups)
      {
        for(auto& group : mUidToItemMap.value(uid, {}))
        {
          beginInsertRows(indexFromItem(group), group->children.size(), group->children.size());
          parseItem(data, group);
          endInsertRows();
        }
      }
    }
    break;
  }
  case TypeColorGroup:
    beginInsertRows(indexFromItem(mColorGroupsParentItem),
                    mColorGroupsParentItem->children.size(),
                    mColorGroupsParentItem->children.size());
    parseGroup(data, mColorGroupsParentItem);
    endInsertRows();
    break;
  case TypeShapeGroup:
    beginInsertRows(indexFromItem(mShapeGroupsParentItem),
                    mShapeGroupsParentItem->children.size(),
                    mShapeGroupsParentItem->children.size());
    parseGroup(data, mShapeGroupsParentItem);
    endInsertRows();
    break;
  case TypeGroup:
    beginInsertRows(indexFromItem(mGroupsParentItem),
                    mShapeGroupsParentItem->children.size(),
                    mShapeGroupsParentItem->children.size());
    parseGroup(data, mGroupsParentItem);
    endInsertRows();
    break;
  default:
    break;
  }
}

void AnalysisPlotItemModel::onEntityAboutToRemove(EntityType type, TEntityUid eId)
{
  if(isEntityModelInResetState)
    return;
  for(auto& item : mUidToItemMap.value(eId, {}))
  {
    auto index = indexFromItem(item);
    auto parentIndex = index.parent();
    QList<AnalysisItem*>* arr = nullptr;
    if(parentIndex.isValid())
      arr = &item->parent->children;
    else
      arr = &mTopLvlItems;

    auto iter = std::find(arr->begin(), arr->end(), item);
    if(iter != arr->end())
    {
      auto i = std::distance(arr->begin(), iter);
      beginRemoveRows(parentIndex, i, i);
      clearRecursive(item);
      arr->removeOne(item);
      endRemoveRows();
    }
  }
}

void AnalysisPlotItemModel::onModelAboutToReset()
{
  isEntityModelInResetState = true;
}

void AnalysisPlotItemModel::onModelReset()
{
  isEntityModelInResetState = false;
  beginResetModel();
  clearItems();
  if(!mEntityModel)
  {
    endResetModel();
    return;
  }
  mColorGroupsParentItem = new AnalysisItem();
  mColorGroupsParentItem->selfTitle = tr("Color groups");
  mTopLvlItems << mColorGroupsParentItem;
  mShapeGroupsParentItem = new AnalysisItem();
  mTopLvlItems << mShapeGroupsParentItem;
  mShapeGroupsParentItem->selfTitle = tr("Shape groups");
  mGroupsParentItem = new AnalysisItem();
  mTopLvlItems << mGroupsParentItem;
  mGroupsParentItem->selfTitle = tr("Groups");


  for(auto& groupEnt : mEntityModel->getEntities(TypeColorGroup))
  {
    parseGroup(groupEnt, mColorGroupsParentItem);
  }
  for(auto& groupEnt : mEntityModel->getEntities(TypeShapeGroup))
  {
    parseGroup(groupEnt, mShapeGroupsParentItem);
  }
  for(auto& groupEnt : mEntityModel->getEntities(TypeGroup))
  {
    parseGroup(groupEnt, mGroupsParentItem);
  }
  auto grouplessEntities = mEntityModel->getEntities(
      [](AbstractEntityDataModel::ConstDataPtr data)->bool
      {
        if(data->getData(RoleGroupsUidSet).value<TGroupsUidSet>().isEmpty()
            && (data->getType() == TypeSample || data->getType() == TypePeak))
          return true;
        return false;
      });
  for(auto& ent : grouplessEntities)
  {
    mTopLvlItems << parseItem(ent, nullptr);
  }
  endResetModel();
}


Qt::DropActions AnalysisPlotItemModel::supportedDropActions() const
{
  return Qt::MoveAction | Qt::CopyAction;
}

Qt::DropActions AnalysisPlotItemModel::supportedDragActions() const
{
  return Qt::MoveAction;
}

QVariant AnalysisPlotItemModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  if(orientation != Qt::Horizontal)
    return QVariant();
  if(role != Qt::DisplayRole && role != Qt::EditRole)
    return QVariant();
  if(section < 0)
    return QVariant();
  if(section >= columnCount())
    return QVariant();
  switch((Columns)section)
  {
  case columnTitle:
    return tr("Title");
  case columnColor:
    return tr("Color");
  case columnShape:
    return tr("Shape");
  case columnSize:
    return tr("Size");
  case columnLast:
    break;
  }
  return QVariant();
}
