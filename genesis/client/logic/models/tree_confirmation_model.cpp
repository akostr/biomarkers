#include "tree_confirmation_model.h"
#include <ui/dialogs/templates/tree_confirmation.h>

TreeConfirmationModel::TreeConfirmationModel(QObject *parent)
  : QAbstractItemModel(parent)
  , mRootItem(new TreeItem(nullptr))
{
}

TreeConfirmationModel::~TreeConfirmationModel()
{
  clearModel();
  delete mRootItem;
}

QModelIndex TreeConfirmationModel::index(int row, int column, const QModelIndex &parent) const
{
  TreeItem* parentItem;

  if (!parent.isValid())
    parentItem = mRootItem;
  else
    parentItem = itemFromIndex(parent);
  if (!parentItem)
    return QModelIndex();

  if(columnCount(parent) <= column ||
     rowCount(parent) <= row)
    return QModelIndex();

  TreeItem* childItem = parentItem->children[row];
  if (childItem)
    return createIndex(row, column, childItem);

  return QModelIndex();
}

QModelIndex TreeConfirmationModel::parent(const QModelIndex &index) const
{
  return parent(itemFromIndex(index));
}

QModelIndex TreeConfirmationModel::parent(TreeItem *item) const
{
  if(!item || !item->parent)
    return QModelIndex();
  const auto& parentChildren = item->parent->children;
  for(auto i = parentChildren.constBegin(); i != parentChildren.constEnd(); i++)
  {
    if(*i == item)
      return createIndex(std::distance(parentChildren.constBegin(), i), 0, item->parent);
  }
  return QModelIndex();
}

int TreeConfirmationModel::rowCount(const QModelIndex &parent) const
{
  if (!parent.isValid())
    return mRootItem->children.size();
  return itemFromIndex(parent)->children.size();
}

int TreeConfirmationModel::columnCount(const QModelIndex &parent) const
{
  return 1;
}

bool TreeConfirmationModel::hasChildren(const QModelIndex &parent) const
{
  if (!parent.isValid())
    return !mRootItem->children.isEmpty();
  return !itemFromIndex(parent)->children.isEmpty();
}

QVariant TreeConfirmationModel::data(const QModelIndex &index, int role) const
{
  TreeItem* item = nullptr;
  if (!index.isValid())
    item = mRootItem;
  else
    item = itemFromIndex(index);
  if(!item)
    return QVariant();
  switch(role)
  {
  case Qt::EditRole:
  case Qt::DisplayRole:
    return item->displayRoleData;
  case Qt::CheckStateRole:
    if(item->parent != mRootItem)
      return QVariant();
    if(item->isChecked)
      return Qt::Checked;
    else
      return Qt::Unchecked;
  case Qt::UserRole:
    return item->userRoleData;
  default:
    return QVariant();
  }
}

bool TreeConfirmationModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
  if (data(index, role) != value)
  {
    TreeItem* item = nullptr;
    if (!index.isValid())
      item = mRootItem;
    else
      item = itemFromIndex(index);
    if(!item)
      return false;

    switch(role)
    {
    case Qt::EditRole:
    case Qt::DisplayRole:
      item->displayRoleData = value;
      break;
    case Qt::CheckStateRole:
      item->isChecked = value.toBool();
      recalcCheckedCount();
      break;
    case Qt::UserRole:
      item->userRoleData = value;
      break;
    default:
      return false;
    }

    emit dataChanged(index, index, {role});
    return true;
  }
  return false;
}

void TreeConfirmationModel::setRootItem(TreeItem *newRootItem)
{
  beginResetModel();
  clearModel();
  if(newRootItem)
  {
    delete mRootItem;
    mRootItem = newRootItem;
  }
  endResetModel();
}

void TreeConfirmationModel::fillModel(Dialogs::Templates::TreeItem *item)
{
  using DialogTreeItem = Dialogs::Templates::TreeItem;
  TreeItem* newRoot = new TreeItem(nullptr);
  std::function<void(TreeItem* newItem, DialogTreeItem* item)> fillTree =
      [&fillTree, this, newRoot](TreeItem* newItem, DialogTreeItem* item)
  {
    if(!newItem || !item)
      return;
    newItem->displayRoleData = item->displayRoleData;
    newItem->userRoleData = item->userRoleData;
    if(newItem->parent == newRoot)
      newItem->isChecked = true;
    if(item->children.isEmpty())
      return;
    for(int row = 0; row < item->children.size(); row++)
    {
      auto newChild = new TreeItem(newItem);
      auto child = item->children[row];
      newItem->children.append(newChild);
      fillTree(newChild, child);
    }
  };
  fillTree(newRoot, item);
  setRootItem(newRoot);
  recalcCheckedCount();
}

Qt::ItemFlags TreeConfirmationModel::flags(const QModelIndex &index) const
{
  if (!index.isValid())
    return Qt::NoItemFlags;
  if(itemFromIndex(parent(index)) == mRootItem)
  {
    return QAbstractItemModel::flags(index) | Qt::ItemIsUserCheckable;
  }
  return QAbstractItemModel::flags(index);
}

bool TreeConfirmationModel::insertRows(int row, int count, const QModelIndex &parent)
{
  beginInsertRows(parent, row, row + count - 1);
  auto item = itemFromIndex(parent);
  for(int r = 0; r < count; r++)
  {
    item->children.insert(row, new TreeItem(item));
  }
  endInsertRows();
  return true;
}

bool TreeConfirmationModel::removeRows(int row, int count, const QModelIndex &parent)
{
  beginRemoveRows(parent, row, row + count - 1);
  auto item = itemFromIndex(parent);
  for(int r = 0; r < count; r++)
  {
    auto removedItem = item->children.takeAt(row);
    delete removedItem;
  }
  endRemoveRows();
  return true;
}

int TreeConfirmationModel::checkedCount() const
{
  return mCheckedCount;
}

TreeConfirmationModel::TreeItem *TreeConfirmationModel::itemFromIndex(const QModelIndex &index) const
{
  if(!index.isValid())
    return mRootItem;
  auto item = static_cast<TreeItem*>(index.internalPointer());
  return item;
}

void TreeConfirmationModel::clearModel()
{
  if(!mRootItem->children.isEmpty())
  {
    for(int row = 0; row < mRootItem->children.size(); row++)
    {
      clearItem(mRootItem->children[row]);
      mRootItem->children[row] = nullptr;
    }
    mRootItem->children.clear();
  }
}

void TreeConfirmationModel::clearItem(TreeItem* item)
{
  if(!item)
    return;

  if(!item->children.isEmpty())
  {
    for(int row = 0; row < item->children.size(); row++)
    {
      clearItem(item->children[row]);
      delete item->children[row];
      item->children[row] = nullptr;
    }
    item->children.clear();
  }
}

void TreeConfirmationModel::recalcCheckedCount()
{
  int counter = 0;
  for(auto& child : mRootItem->children)
  {
    if(child->isChecked)
      counter++;
  }
  if(counter != mCheckedCount)
  {
    mCheckedCount = counter;
    emit checkedCountChanged(mCheckedCount);
  }
}
