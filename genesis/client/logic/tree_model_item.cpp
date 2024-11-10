#include "tree_model_item.h"
#include "tree_model.h"

#include <QSet>
#include <QString>
#include <QUuid>
#include <QVariant>
#include <logic/context_root.h>
#include <logic/known_context_tag_names.h>

namespace Details
{
  void TreeNodeSwap(QObject*& left, QObject*& right)
  {
    TreeModelItem* itemLeft  = qobject_cast<TreeModelItem*>(left);
    TreeModelItem* itemRight = qobject_cast<TreeModelItem*>(right);

    if (!itemLeft || !itemRight)
      return;

    TreeModelItem* parentLeft  = itemLeft->GetParent();
    TreeModelItem* parentRight = itemRight->GetParent();

    QList<QPersistentModelIndex> parents;
    TreeModelItem* commonParent = nullptr;
    if (parentLeft == parentRight)
    {
      commonParent = parentLeft;
      if (commonParent)
      {
        parents << QPersistentModelIndex(commonParent->GetIndex());
      }
    }

    auto model = itemLeft->GetModel();
    QModelIndex parentIndex;
    if (commonParent)
      parentIndex = commonParent->GetIndex();

    QModelIndex indexLeft = itemLeft->GetIndex();
    QModelIndex indexRight = itemRight->GetIndex();

    int newRowLeft  = indexRight.row();
    int newRowRight = indexLeft.row();

    if (commonParent)
    {
      model->layoutAboutToBeChanged(parents, QAbstractItemModel::VerticalSortHint);
    }
    qSwap(left, right);
    if (!parents.empty())
    {
      model->changePersistentIndex(indexLeft,  model->index(newRowLeft,  0, parentIndex));
      model->changePersistentIndex(indexRight, model->index(newRowRight, 0, parentIndex));

      model->layoutChanged(parents, QAbstractItemModel::VerticalSortHint);
    }
  }

  template <typename RandomAccessIterator, typename T, typename LessThan>
  void TreeNodeSortHelper(RandomAccessIterator start, RandomAccessIterator end, const T &t, LessThan lessThan)
  {
  top:
    int span = int(end - start);
    if (span < 2)
      return;

    --end;
    RandomAccessIterator low = start, high = end - 1;
    RandomAccessIterator pivot = start + span / 2;

    if (lessThan(*end, *start))
      TreeNodeSwap(*end, *start);
    if (span == 2)
      return;

    if (lessThan(*pivot, *start))
      TreeNodeSwap(*pivot, *start);
    if (lessThan(*end, *pivot))
      TreeNodeSwap(*end, *pivot);
    if (span == 3)
      return;

    TreeNodeSwap(*pivot, *end);

    while (low < high)
    {
      while (low < high && lessThan(*low, *end))
        ++low;

      while (high > low && lessThan(*end, *high))
        --high;

      if (low < high)
      {
        TreeNodeSwap(*low, *high);
        ++low;
        --high;
      }
      else
      {
        break;
      }
    }

    if (lessThan(*low, *end))
      ++low;

    TreeNodeSwap(*end, *low);
    TreeNodeSortHelper(start, low, t, lessThan);

    start = low + 1;
    ++end;
    goto top;
  }

  template<typename Container, typename LessThan>
  void TreeNodeSort(Container &c, LessThan lessThan)
  {
    if (!c.empty())
    {
      TreeNodeSortHelper(c.begin(), c.end(), *c.begin(), lessThan);
    }
  }

  bool NodeLessThan(QObject* left, QObject* right)
  {
    int ltOrder = left->property("order").toInt();
    int rtOrder = right->property("order").toInt();
    if (ltOrder != rtOrder)
    {
      return ltOrder < rtOrder;
    }

    std::string sortSubject = "name";

    QVariant lt = left->property(sortSubject.c_str());
    QVariant rt = right->property(sortSubject.c_str());
    if (
        lt
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        .typeId()
#else
        .type()
#endif
         == QMetaType::QString
     && rt
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        .typeId()
#else
        .type()
#endif
         == QMetaType::QString)
    {
      return lt.toString().localeAwareCompare(rt.toString()) < 0;
    }

    Q_ASSERT(!"comparator inconsistent");
    return false;
  }
}

TreeModelItem::TreeModelItem(TreeModelItem* parent, int row)
  : QObject(parent)
  , Model(parent ? parent->GetModel() : nullptr)
  , Row(row)
  , RowCount(0)
  , Check(Qt::Unchecked)
{
  if (Model && parent)
  {
    {
      parent->RowCount++;
//      Model->beginInsertRows(parent->GetIndex(), parent->GetRowCount() - 1, parent->GetRowCount() - 1);
//      Model->endInsertRows();
    }

    {
      QVariant p;
      p.setValue((QObject*)parent);
      setProperty("parent", p);

      QVariant i;
      i.setValue(GetIndex());
      setProperty("model_index", i);
    }
  }
}

TreeModelItem::~TreeModelItem()
{
  if (TreeModelItem* parent = GetParent())
  {
    parent->RowCount--;
  }
}

const char* TreeModelItem::GetUniquePropertyName() const
{
  return Model->GetUniquePropertyName();
}

//// Serializable
void TreeModelItem::Save(QJsonObject& json) const
{
  QVariantMap data;
  Save(data);
  json = QJsonObject::fromVariantMap(data);
}

void TreeModelItem::Load(const QJsonObject& json, bool emitResetModel)
{
  Load(json.toVariantMap(), emitResetModel);
}

void TreeModelItem::Save(QVariantMap& data) const
{
  //// Self
  data = Data;

  //// Children
  QVariantList childrenArray;
  auto children = GetChildren();
  for (auto c : children)
  {
    QVariantMap j;
    c->Save(j);
    childrenArray.push_back(j);
  }
  data["children"] = childrenArray;
}

void TreeModelItem::Load(const QVariantMap& data, bool emitResetModel)
{
  //// Clear
  {
    //// Silently
    QSignalBlocker sb(this);
    Clear(emitResetModel);
  }

  if (Model && IsRoot() && emitResetModel)
    Model->beginResetModel();

  //// Self
  QStringList keys = data.keys();
  for (auto& k : keys)
  {
    if (k != "children")
    {
      std::string ks = k.toStdString();
      setProperty(ks.c_str(), data[ks.c_str()]);
      Data[k] = data[ks.c_str()];
    }
  }

  //// Children
  QVariantList childrenArray = data["children"].toList();
  if(!emitResetModel && !childrenArray.isEmpty())
    Model->beginInsertRows(GetIndex(), 0, childrenArray.size() - 1);
  int row = 0;
  for (auto& a : childrenArray)
  {
    QVariantMap object = a.toMap();
    TreeModelItem* child = new TreeModelItem(this, row);
    child->Load(object);
    ++row;
  }
  if(!emitResetModel && !childrenArray.isEmpty())
    Model->endInsertRows();

  if (Model && IsRoot() && emitResetModel)
    Model->endResetModel();
}

//// Data
QVariantMap TreeModelItem::GetData() const
{
  return Data;
}

void TreeModelItem::SetData(const QVariantMap& data)
{
  Data = data;
}

QVariant TreeModelItem::GetData(const QString& key) const
{
  return Data.value(key);
}

void TreeModelItem::SetData(const QString& key, const QVariant& data)
{
  if (Data[key] != data)
  {
    Data[key] = data;
    NotifyChanged();
  }
}

void TreeModelItem::SetCheckState(Qt::CheckState check)
{
  switch (check)
  {
  case Qt::Unchecked:
  case Qt::Checked:
  {
    Check = check;
    NotifyChanged();

    QList<TreeModelItem*> children = GetChildren();
    QList<TreeModelItem*>::iterator c = children.begin();
    while (c != children.end())
    {
      (*c)->SetCheckState(check);
      ++c;
    }

    if (TreeModelItem* parent = GetParent())
    {
      parent->UpdateCheckState();
    }
  }
  break;
  default:
    break;
  }
}

Qt::CheckState TreeModelItem::GetCheckState() const
{
  return Check;
}

void TreeModelItem::UpdateCheckState()
{
  QMap<Qt::CheckState, int> states;
  states[Qt::Checked] = 0;
  states[Qt::Unchecked] = 0;
  states[Qt::PartiallyChecked] = 0;

  QList<TreeModelItem*> children = GetChildren();
  QList<TreeModelItem*>::iterator c = children.begin();
  while (c != children.end())
  {
    states[(*c)->GetCheckState()] = states[(*c)->GetCheckState()] + 1;
    ++c;
  }

  Qt::CheckState check = Check;
  if (states[Qt::Checked] == children.size())
    check = Qt::Checked;
  else if (states[Qt::Unchecked] == children.size())
    check = Qt::Unchecked;
  else
    check = Qt::PartiallyChecked;

  if (check != Check)
  {
    Check = check;
    NotifyChanged();

    if (TreeModelItem* parent = GetParent())
    {
      parent->UpdateCheckState();
    }
  }
}

QSet<QString> TreeModelItem::GetCheckedPaths(Qt::CheckState check) const
{
  QSet<QString> paths;

  QList<TreeModelItem*> children = GetChildrenRecursive();
  for (auto c : children)
  {
    if (c->GetCheckState() == check)
    {
      paths << c->GetPath();
    }
  }

  return paths;
}

void TreeModelItem::SetCheckedPaths(Qt::CheckState check, const QSet<QString>& paths)
{
  QList<TreeModelItem*> children = GetChildrenRecursive();
  for (auto c : children)
  {
    if (paths.contains(c->GetPath()))
    {
      c->SetCheckState(check);
    }
  }
}

TreeModelItem* TreeModelItem::GetParent() const
{
  return qobject_cast<TreeModelItem*>(parent());
}

TreeModelItem* TreeModelItem::GetPrev() const
{
  auto index = GetIndex();
  if (!index.isValid())
    return 0;

  if (index.row() == 0)
    return 0;

  if (auto parent = GetParent())
  {
    return parent->GetChild(index.row() - 1);
  }
  return 0;
}

TreeModelItem* TreeModelItem::GetNext() const
{
  auto index = GetIndex();
  if (!index.isValid())
    return 0;

  if (auto parent = GetParent())
  {
    return parent->GetChild(index.row() + 1);
  }
  return 0;
}

int TreeModelItem::GetRow() const
{
  if (Row != -1)
    return Row;

  TreeModelItem* parentItem = GetParent();
  if (parentItem)
    return parentItem->GetChildren().indexOf(const_cast<TreeModelItem*>(this));

  return 0;
}

bool TreeModelItem::CanFetchMore() const
{
  return IsRoot()
    && IsEmpty()
    && !IsFetched()
    && property("error").toInt() == 0
    && Core::GenesisContextRoot::Get()->GetContextUser()->GetData(Names::UserContextTags::kUserState).toInt() != Names::UserContextTags::UserState::notAuthorized;
}

bool TreeModelItem::IsEmpty() const
{
  return findChild<TreeModelItem*>() == nullptr;
}

bool TreeModelItem::IsFetched() const
{
  return property("fetched").toBool();
}

void TreeModelItem::SetFetched(bool fetched)
{
  setProperty("fetched", fetched);
}

QModelIndex TreeModelItem::GetIndex(int column) const
{
  TreeModel* model = GetModel();
  if (!model)
    return QModelIndex();

  TreeModelItem* parentItem = GetParent();
  if (!parentItem)
    return QModelIndex();

  QModelIndex parentIndex = parentItem->GetIndex();
  return model->index(GetRow(), column, parentIndex);
}

QList<TreeModelItem*> TreeModelItem::GetChildren() const
{
  QList<TreeModelItem*> childItems;
  const QObjectList& cn = children();
  childItems.reserve(cn.size());
  for (const auto& child : cn)
  {
    if (auto ci = qobject_cast<TreeModelItem*>(child))
      childItems.push_back(ci);
  }
  return childItems;
}

QList<TreeModelItem*> TreeModelItem::GetChildrenRecursive() const
{
  return findChildren<TreeModelItem*>();
}

TreeModel* TreeModelItem::GetModel() const
{
  return Model;
}

void TreeModelItem::SetModel(TreeModel* model)
{
  Model = model;
}

int TreeModelItem::GetRowCount() const
{
  return RowCount;
}

TreeModelItem* TreeModelItem::GetChild(int row) const
{
  const auto& cn = children();
  if (row < cn.size())
    return qobject_cast<TreeModelItem*>(cn.at(row));

  return 0;
}

TreeModelItem* TreeModelItem::GetChild(const QString& id) const
{
  QList<TreeModelItem*> children = GetChildren();
  QList<TreeModelItem*>::iterator c = children.begin();
  while (c != children.end())
  {
    if ((*c)->property("id").toString() == id)
      return *c;
    ++c;
  }
  return nullptr;
}

TreeModelItem* TreeModelItem::GetChild(const char* propertyName, const QUuid& id) const
{
  QList<TreeModelItem*> children = GetChildren();
  QList<TreeModelItem*>::iterator c = children.begin();
  while (c != children.end())
  {
    if ((*c)->property(propertyName).toUuid() == id)
      return *c;
    ++c;
  }
  return nullptr;
}

void TreeModelItem::Clear(bool emitResetModel)
{
  if (IsEmpty())
    return;

  if (Model && IsRoot() && emitResetModel)
    Model->beginResetModel();

  QList<TreeModelItem*> children = GetChildren();
  if(!emitResetModel)
  {
    Model->beginRemoveRows(GetIndex(), 0, children.count() - 1);
  }
  QList<TreeModelItem*>::iterator c = children.begin();
  while (c != children.end())
  {
    delete *c;
    ++c;
  }
  setProperty("fetched", false);
  RowCount = 0;
  Data.clear();
  if(!emitResetModel)
  {
    Model->endRemoveRows();
  }

  if (Model && IsRoot() && emitResetModel)
    Model->endResetModel();
}

void TreeModelItem::NotifyChanged()
{
  if (Model && !Model->IsInResetState())
  {
    QVector<int> roles;
    roles << Qt::EditRole;
    roles << Qt::DisplayRole;
    roles << Qt::DecorationRole;
    roles << Qt::CheckStateRole;
    QModelIndex index = GetIndex();
    emit Model->dataChanged(index, index, roles);
  }
}

void TreeModelItem::Sort(bool recursive)
{
  QObjectList& childrenList = const_cast<QObjectList&>(children());
  Details::TreeNodeSort(childrenList, &Details::NodeLessThan);

  if (recursive)
  {
    QList<TreeModelItem*> children = GetChildren();
    QList<TreeModelItem*>::iterator c = children.begin();
    while (c != children.end())
    {
      (*c)->Sort(true);
      ++c;
    }
  }
}

void TreeModelItem::RemoveChild(int row)
{
  if (Model)
    Model->beginRemoveRows(GetIndex(), row, row);
  delete GetChildren().value(row);
  if (Model)
    Model->endRemoveRows();
}

void TreeModelItem::RemoveChild(TreeModelItem* child)
{
  int r = child->GetRow();
  if (Model)
    Model->beginRemoveRows(GetIndex(), r, r);
  delete child;
  if (Model)
    Model->endRemoveRows();
}

void TreeModelItem::RemoveChild(const char* propertyName, const QUuid& id)
{
  QList<TreeModelItem*> children = GetChildren();
  QList<TreeModelItem*>::iterator c = children.begin();
  while (c != children.end())
  {
    if ((*c)->property(propertyName) == id)
    {
      RemoveChild(*c);
    }
    ++c;
  }
}

void TreeModelItem::RemoveChildren(const char* propertyName, const QList<QUuid>& ids)
{
  QList<TreeModelItem*> children = GetChildren();
  QList<TreeModelItem*>::iterator c = children.begin();
  while (c != children.end())
  {
    if (ids.contains((*c)->property(propertyName).toUuid()))
    {
      RemoveChild(*c);
    }
    ++c;
  }
}

void TreeModelItem::RemoveChildrenRecursively(std::function<bool (TreeModelItem *)> predicate, bool emitResetModel)
{
  auto children = GetChildrenRecursive();
  if(Model && emitResetModel)
    Model->beginResetModel();
  for(auto& child : children)
  {
    if(predicate(child))
    {
      delete child;
    }
  }
  if(Model && emitResetModel)
    Model->endResetModel();
}

TreeModelItem *TreeModelItem::MoveChildTo(int childRow, TreeModelItem* newParent, bool emitReset)
{
  RowCount--;
  if (Model && emitReset)
    Model->beginResetModel();

  auto item = GetChildren().value(childRow);
  for(auto& child : GetChildren())
  {
    if(child->Row > childRow)
      child->Row--;
  }

  int newParentRow = newParent->RowCount;
  newParent->RowCount++;
  for(auto& child : newParent->GetChildren())
  {
    if(child->Row >= newParentRow)
      child->Row++;
  }
  item->Row = newParentRow;
  item->setParent(newParent);
  if (Model && emitReset)
    Model->endResetModel();
  return item;
}

//void TreeModelItem::AppendChild(TreeModelItem *newChild, int row)
//{
//  RowCount++;
//  qDebug() << Model->rowCount(GetIndex());
//  qDebug() << Model->rowCount(newChild->GetIndex());
//  Model->beginInsertRows(GetIndex(), row, row);

//  for(auto& child : GetChildren())
//  {
//    if(child->Row >= row)
//      child->Row++;
//  }
//  newChild->Row = row;
//  newChild->setParent(this);

//  Model->endInsertRows();
//}

void TreeModelItem::CleanupChildrenExcept(const char* propertyName, const QList<QUuid>& newIds)
{
  QList<QUuid> idsToRemove = FindExistingChildIds(propertyName);
  QList<QUuid>::const_iterator p = newIds.cbegin();
  while (p != newIds.cend())
  {
    idsToRemove.removeAll(*p);
    ++p;
  }

  RemoveChildren(propertyName, idsToRemove);
}

void TreeModelItem::CleanupChildrenExcept(const char *propertyName, const QList<QVariant>& data)
{
  auto childs = GetChildren();
  for (const auto& child : childs)
  {
    if (child->property(propertyName).isValid())
      if (!data.contains(child->property(propertyName)))
        RemoveChild(child);
  }
}

//// Get slash separated path
QString TreeModelItem::GetPath() const
{
  TreeModelItem* parent = GetParent();
  QString path = GetNodeId();
  if (parent)
  {
    path.prepend("/");
    path.prepend(parent->GetPath());
  }
  return path;
}

QString TreeModelItem::GetNodeId() const
{
  if (IsRoot())
    return "root";

  return property(GetUniquePropertyName()).toString();
}

bool TreeModelItem::IsRoot() const
{
  return GetParent() == nullptr;
}

TreeModelItem* TreeModelItem::FindRecursive(TreeModelItem* item)
{
  if (item == this)
    return this;

  auto children = GetChildren();
  auto c = children.cbegin();
  while (c != children.cend())
  {
    TreeModelItem* childResult = (*c)->FindRecursive(item);
    if (childResult)
    {
      return childResult;
    }
    ++c;
  }
  return nullptr;
}

TreeModelItem* TreeModelItem::FindChild(const char* propertyName, const QVariant& propertyValue) const
{
  auto children = GetChildren();
  auto c = children.cbegin();
  while (c != children.cend())
  {
    if ((*c)->property(propertyName) == propertyValue)
    {
      return *c;
    }
    ++c;
  }
  return nullptr;
}

TreeModelItem* TreeModelItem::FindParent(const char* propertyName, const QVariant& propertyValue) const
{
  if (property(propertyName) == propertyValue)
    return const_cast<TreeModelItem*>(this);

  if (auto parent = GetParent())
    return parent->FindParent(propertyName, propertyValue);

  return nullptr;
}

TreeModelItem* TreeModelItem::FindChild(const QString& path) const
{
  QStringList pathNodes = path.split('/');
  if (pathNodes.empty())
    return nullptr;

  auto children = GetChildren();
  auto c = children.cbegin();
  while (c != children.cend())
  {
    if ((*c)->GetNodeId() == pathNodes.first())
    {
      if (pathNodes.size() == 1)
      {
        return *c;
      }
      else
      {
        pathNodes.removeFirst();
        return (*c)->FindChild(pathNodes.join('/'));
      }
    }
    ++c;
  }
  return nullptr;
}

TreeModelItem* TreeModelItem::FindChildAbs(const QString& path) const
{
  QString p = GetPath();
  if (path.startsWith(p))
  {
    QString childPath = path;
    childPath.remove(QString("%1/").arg(p));
    return FindChild(childPath);
  }
  return nullptr;
}

QList<QUuid> TreeModelItem::FindExistingChildIds(const char* propertyName)
{
  QList<QUuid> childIds;
  auto children = GetChildren();
  auto c = children.cbegin();
  while (c != children.cend())
  {
    if ((*c)->dynamicPropertyNames().contains(propertyName))
    {
      childIds << (*c)->property(propertyName).toUuid();
    }
    ++c;
  }
  return std::move(childIds);
}

TreeModelItem* TreeModelItem::FindRecursive(const char* propertyName, const QVariant& propertyValue) const
{
  auto children = GetChildren();
  auto c = children.cbegin();
  while (c != children.cend())
  {
    if ((*c)->property(propertyName) == propertyValue)
    {
      return *c;
    }

    TreeModelItem* childResult = (*c)->FindRecursive(propertyName, propertyValue);
    if (childResult)
    {
      return childResult;
    }
    ++c;
  }
  return nullptr;
}

