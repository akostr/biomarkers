#include "tree_model_dynamic_project_fragments.h"
#include "tree_model_item.h"

#include "../api/api_rest.h"
#include "../logic/notification.h"
#include <logic/known_context_tag_names.h>

#include <logic/context_root.h>
#include <logic/enums.h>

#include <QVariantMap>
#include <QDateTime>

using namespace Core;

namespace
{
  QStringList InvisibleChars = { u8"\u2000", u8"\u2001", u8"\u2002", u8"\u2003", u8"\u2004" };
  const QString checkable = "checkable";
  const QString filename = "filename";
  const QString file_type_id = "file_type_id";
}

const QColor TreeModelDynamicProjectFragments::m_defaultGroupMemberColor = QColor(0, 39, 59, 13);

TreeModelDynamicProjectFragments::TreeModelDynamicProjectFragments(QObject* parent)
  : TreeModelDynamic{ parent }
  , m_isInMarkupPickMode(false)
{
  //// Setup columns
  Columns[ColumnProjectId].Ids[Qt::EditRole] = "project_file_id";
  Columns[ColumnProjectId].Ids[Qt::DisplayRole] = "project_file_id";
  Columns[ColumnProjectId].HeaderData[Qt::DisplayRole] = tr("ID");
  Columns[ColumnProjectId].Filter = ColumnInfo::FilterTypeTextSubstring;

  Columns[ColumnFileName].Ids[Qt::EditRole] = filename;
  Columns[ColumnFileName].Ids[Qt::DisplayRole] = filename;
  Columns[ColumnFileName].HeaderData[Qt::DisplayRole] = tr("m/z");
  Columns[ColumnFileName].Filter = ColumnInfo::FilterTypeTextSubstring;

  Columns[ColumnSampleType].Ids[Qt::ToolTipRole] = "sample_type";
  Columns[ColumnSampleType].Ids[Qt::DisplayRole] = "sample_type_id";
  Columns[ColumnSampleType].HeaderData[Qt::DisplayRole] = tr("Sample type");
  Columns[ColumnSampleType].Filter = ColumnInfo::FilterTypeTextSubstring;

  Columns[ColumnType].Ids[Qt::EditRole] = "file_type";
  Columns[ColumnType].Ids[Qt::DisplayRole] = "file_type";
  Columns[ColumnType].Ids[Qt::UserRole] = file_type_id;
  Columns[ColumnType].HeaderData[Qt::DisplayRole] = tr("File type");
  Columns[ColumnType].Filter = ColumnInfo::FilterTypeTextSubstring;

  Columns[ColumnField].Ids[Qt::EditRole] = "field_title";
  Columns[ColumnField].Ids[Qt::DisplayRole] = "field_title";
  Columns[ColumnField].HeaderData[Qt::DisplayRole] = tr("Field");
  Columns[ColumnField].Filter = ColumnInfo::FilterTypeTextSubstring;

  Columns[ColumnWellCluster].Ids[Qt::EditRole] = "well_cluster_title";
  Columns[ColumnWellCluster].Ids[Qt::DisplayRole] = "well_cluster_title";
  Columns[ColumnWellCluster].HeaderData[Qt::DisplayRole] = tr("Bush");
  Columns[ColumnWellCluster].Filter = ColumnInfo::FilterTypeTextSubstring;

  Columns[ColumnWell].Ids[Qt::EditRole] = "well_title";
  Columns[ColumnWell].Ids[Qt::DisplayRole] = "well_title";
  Columns[ColumnWell].HeaderData[Qt::DisplayRole] = tr("Well");
  Columns[ColumnWell].Filter = ColumnInfo::FilterTypeTextSubstring;

  Columns[ColumnLayer].Ids[Qt::EditRole] = "layer_title";
  Columns[ColumnLayer].Ids[Qt::DisplayRole] = "layer_title";
  Columns[ColumnLayer].HeaderData[Qt::DisplayRole] = tr("Layer");
  Columns[ColumnLayer].Filter = ColumnInfo::FilterTypeTextSubstring;

  Columns[ColumnDepth].Ids[Qt::EditRole] = "depth";
  Columns[ColumnDepth].Ids[Qt::DisplayRole] = "depth";
  Columns[ColumnDepth].HeaderData[Qt::DisplayRole] = tr("Depth");
  Columns[ColumnDepth].Filter = ColumnInfo::FilterTypeTextSubstring;

  Columns[ColumnDate].Ids[Qt::EditRole] = "sample_date_time";
  Columns[ColumnDate].Ids[Qt::DisplayRole] = "sample_date_time";
  Columns[ColumnDate].HeaderData[Qt::DisplayRole] = tr("Date");
  Columns[ColumnDate].Filter = ColumnInfo::FilterTypeDateRange;

  //// Setup item actions
  m_fragmentActions =
  {
    // { "show_on_plot", "", tr("Show on plot") },
    { "view_sample_passport", "", tr("View sample passport") },
    { "edit_mz", "", tr("Edit m/z") },
    { "export", "", tr("Export") },
    { "delete", "", tr("Delete") }
  };
  m_fragmentWithParentActions =
  {
    // { "show_on_plot", "", tr("Show on plot") },
    { "view_sample_passport", "", tr("View sample passport") },
    { "export", "", tr("Export") },
    { "delete", "", tr("Delete") }
  };
  m_chromatogrammActions =
  {
    // { "show_on_plot", "", tr("Show on plot") },
    { "view_sample_passport", "", tr("View sample passport") },
    { "export", "", tr("Export") }
  };
  m_iconsToColumnsMap[0] = QPixmap();
  m_iconsToColumnsMap[1] = QPixmap(":/resource/icons/sample_type_1@2x.png");
  m_iconsToColumnsMap[2] = QPixmap(":/resource/icons/sample_type_2@2x.png");
  m_iconsToColumnsMap[3] = QPixmap(":/resource/icons/sample_type_3@2x.png");
  m_iconsToColumnsMap[4] = QPixmap(":/resource/icons/sample_type_4@2x.png");

  Reset();
}

QList<TreeModelItem*> TreeModelDynamicProjectFragments::getCheckedItems(TreeModelItem* parent)
{
  if (!parent)
    parent = Root;

  QList<TreeModelItem*> ret;
  for (auto child : parent->GetChildrenRecursive())
  {
    auto data = child->GetData(checkable);
    if (data.isValid() && !data.isNull() && data.toBool())
      if (child->GetCheckState() == Qt::Checked)
        ret.append(child);
  }
  return ret;
}

int TreeModelDynamicProjectFragments::getCheckableCount()
{
  if (!Checkable)
    return 0;
  int count = 0;
  for (auto child : GetRoot()->GetChildrenRecursive())
  {
    if (!m_isInMarkupPickMode)
    {
      count++;
    }
    else
    {
      auto data = child->GetData(checkable);
      if (data.isValid() && !data.isNull() && data.toBool())
        count++;
    }
  }
  return count;
}

void TreeModelDynamicProjectFragments::ApplyContextUser(const QString& dataId, const QVariant& data)
{
  bool isReset = isDataReset(dataId, data);
  if (isReset)
  {
    Reset();
  }
  else if (dataId == Names::UserContextTags::kUserState)
  {
    if (data.toInt() == Names::UserContextTags::authorized)
      Reset();
    else
      Clear();
  }
}

void TreeModelDynamicProjectFragments::ApplyContextProject(const QString& dataId, const QVariant& data)
{
  bool isReset = isDataReset(dataId, data);
  if (isReset)
  {
    Reset();
  }
  else if (dataId == Names::ContextTagNames::ProjectId)
  {
    if (data.toInt())
      Reset();
    else
      Clear();
  }
}

void TreeModelDynamicProjectFragments::Reset()
{
  beginResetModel();
  Root->Clear(false);
  Root->SetFetched(false);
  //  if (canFetchMore(QModelIndex()))
  //    FetchMore(Root);
  endResetModel();
}

bool TreeModelDynamicProjectFragments::canFetchMore(const QModelIndex& parent) const
{
  QPointer<TreeModelItem> parentItem;
  if (parent.column() > 0)
    return 0;

  if (!parent.isValid())
    parentItem = Root;
  else
    parentItem = static_cast<TreeModelItem*>(parent.internalPointer());

  return parentItem->CanFetchMore();
}

void TreeModelDynamicProjectFragments::fetchMore(const QModelIndex& parent)
{
  QPointer<TreeModelItem> parentItem;
  if (parent.column() > 0)
    return;

  if (!parent.isValid())
    parentItem = Root;
  else
    parentItem = static_cast<TreeModelItem*>(parent.internalPointer());

  if (!parentItem->CanFetchMore())
    return;

  FetchMore(parentItem);
}

void TreeModelDynamicProjectFragments::FetchMore(QPointer<TreeModelItem> item)
{
  if (item->CanFetchMore())
  {
    item->SetFetched(true);

    if (int projectId = GenesisContextRoot::Get()->ProjectId())
    {
      API::REST::Tables::GetProjectFiles(
        projectId,
        [item, this](QNetworkReply*, QVariantMap result)
        {
          if (!item)
            return;
          preprocessFetch(result);
          beginResetModel();
          item->Load(result, false);
          postprocessFetch(item);
          endResetModel();
        },
        [item](QNetworkReply*, QNetworkReply::NetworkError err)
        {
          if (!item)
            return;

          item->Clear();
          Notification::NotifyError(tr("Failed to get imported files from database"), err);
        }
      );
    }
  }
}

QList<ActionInfo> TreeModelDynamicProjectFragments::GetItemActions(const QModelIndex& index)
{
  auto item = GetItem(index);
  auto itemValue = item->GetData(file_type_id);
  if (!itemValue.isValid())
    return ItemActions;
  auto itemType = itemValue.toInt();
  if (itemType == ProjectFiles::sim)
  {
    if (item->parent() != GetRoot())
      return m_fragmentWithParentActions;
    else
      return m_fragmentActions;
  }
  else if (itemType == ProjectFiles::tic)
    return m_chromatogrammActions;
  else
    return ItemActions;
}

int TreeModelDynamicProjectFragments::nonGroupItemsCount(TreeModelItem* parent)
{
  if (!parent)
    parent = Root;
  int counter = 0;
  std::function<void(int&, TreeModelItem*)> walkThrough = [&walkThrough, this](int& counter, TreeModelItem* current)
    {
      if (!current->IsRoot() && !isItemIsGroup(current)) //non root and non group-item
        counter++;
      for (auto& child : current->GetChildren())
        walkThrough(counter, child);
    };
  walkThrough(counter, parent);
  return counter;
}

void TreeModelDynamicProjectFragments::setChildrenCheckState(Qt::CheckState state, TreeModelItem* parent)
{
  beginResetModel();
  std::function<void(TreeModelItem*)> walkThrough = [this, &walkThrough, state](TreeModelItem* current)
    {
      if (Checkable)
      {
        if (!m_isInMarkupPickMode)
        {
          current->SetCheckState(state);
          for (auto& child : current->GetChildren())
            walkThrough(child);
        }
        else
        {
          auto data = current->GetData(checkable);
          if (data.isValid() && !data.isNull() && data.toBool())
            current->SetCheckState(state);
          for (auto& child : current->GetChildren())
            walkThrough(child);
        }
      }
    };
  walkThrough(parent);

  endResetModel();
}

TreeModelItem* TreeModelDynamicProjectFragments::getChildItemByFileId(int fileId, TreeModelItem* item, bool recursive)
{
  if (!item)
    item = GetRoot();
  for (auto& child : item->GetChildren())
  {
    if (child->GetData("id").toInt() == fileId)
      return child;
  }
  if (recursive)
  {
    for (auto& child : item->GetChildren())
    {
      auto foundedItem = getChildItemByFileId(fileId, child, true);
      if (foundedItem)
        return foundedItem;
    }
  }
  return nullptr;
}

void TreeModelDynamicProjectFragments::sortIntoGroups(QPointer<TreeModelItem> item)
{
  for (auto& child : item->GetChildren())
  {
    auto parentIdVariant = child->GetData("parent_id");
    if (parentIdVariant.isValid() && !parentIdVariant.isNull())
    {
      auto parentFileId = parentIdVariant.toInt();
      auto parentItem = getChildItemByFileId(parentFileId, item);
      if (parentItem)
      {
        parentItem->SetData(file_type_id, ProjectFiles::tic);
        parentItem->SetData("file_type", "TIC");
        child->SetData("field_title", parentItem->GetData("field_title"));
        child->SetData("well_cluster_title", parentItem->GetData("well_cluster_title"));
        child->SetData("well_title", parentItem->GetData("well_title"));
        child->SetData("layer_title", parentItem->GetData("layer_title"));
        child->SetData("depth", parentItem->GetData("depth"));
        child->SetData("sample_date_time", parentItem->GetData("sample_date_time"));
        item->MoveChildTo(child->GetRow(), parentItem, false);
      }
    }
  }
}

void TreeModelDynamicProjectFragments::cleanupEmptyGroups(TreeModelItem* item)
{
  if (!item)
    item = GetRoot();
  item->RemoveChildrenRecursively([](TreeModelItem* child)->bool
    {
      return child->GetData(file_type_id).toInt() == ProjectFiles::GC_MS;
    }, false);
}

void TreeModelDynamicProjectFragments::parseIonsNamesToIonsLists(TreeModelItem* item)
{
  if (!item)
    item = GetRoot();
  for (auto& child : item->GetChildren())
  {
    if (child->GetData(file_type_id).toInt() == ProjectFiles::sim)
    {
      auto stringIonsList = child->GetData(filename).toString().split(", ");
      QList<int> ionsList;
      for (const auto& strIon : stringIonsList)
        if (!strIon.isEmpty())
          ionsList << strIon.toInt();
      child->SetData("ions_int_list", QVariant::fromValue(ionsList));
    }
  }
}

bool TreeModelDynamicProjectFragments::isItemIsGroup(const QModelIndex& index) const
{
  return TreeModelDynamicProjectFragments::isItemIsGroup(GetItem(index));
}

void TreeModelDynamicProjectFragments::setIonNameCheckedFilter(QString name)
{
  ionReferenceName = name;
  auto root = GetRoot();
  for (auto& child : root->GetChildrenRecursive())
      child->SetData(checkable, child->GetData(filename).toString() == ionReferenceName);
}

void TreeModelDynamicProjectFragments::resetIonNameCheckedFilter()
{
  ionReferenceName = "";
  auto root = GetRoot();
  for (auto& child : root->GetChildrenRecursive())
    child->SetData(checkable, child->GetData(file_type_id).toInt() != ProjectFiles::tic);
}

void TreeModelDynamicProjectFragments::renameItems(TreeModelItem* item)
{
  for (auto& child : item->GetChildrenRecursive())
  {
    if (child->GetData(file_type_id).toInt() == ProjectFiles::tic)
    {
      auto name = child->GetData("filename").toString();
      name = "TIC(" + name + ")";
      child->SetData("filename", name);
    }
    else if (child->GetData(file_type_id).toInt() == ProjectFiles::sim)
    {
      auto name = child->GetData("m_z").toString();
      name.prepend("ion ");
      child->SetData(filename, name);
    }
  }
}

void TreeModelDynamicProjectFragments::preprocessFetch(QVariantMap& variantMap)
{
  if (variantMap.contains("children"))
  {
    QVariantList children = variantMap["children"].toList();
    children.removeIf([children](const QVariant& val)->bool
      {
        auto type = val.toMap()[file_type_id].toInt();
        if (type != ProjectFiles::sim &&
          type != ProjectFiles::GC_MS)
          return true;
        return false;
      });
    for (auto& c : children)
    {
      auto row = c.toMap();
      QStringList filePath = row[filename].toString().split("/");
      if (!filePath.empty())
        row["filename_short"] = filePath.last();
      c = row;
    }
    variantMap["children"] = children;
  }
}

void TreeModelDynamicProjectFragments::postprocessFetch(TreeModelItem* item)
{
  sortIntoGroups(item);
  cleanupEmptyGroups(item);
  renameItems(item);
  resetIonNameCheckedFilter();
}

bool TreeModelDynamicProjectFragments::isItemIsGroup(const TreeModelItem* item)
{
  if (!item)
    return false;
  const auto isGroup = item->GetData("group");
  return isGroup.isValid() && !isGroup.isNull() && isGroup.toBool();
}

void TreeModelDynamicProjectFragments::setCheckedByMZValue(const QString& value, int state)
{
  for (auto& item : Root->GetChildrenRecursive())
  {
    if (item->GetData(checkable).toBool())
    {
      item->SetCheckState(static_cast<Qt::CheckState>(state));
    }
  }
}

QVariant TreeModelDynamicProjectFragments::data(const QModelIndex& index, int role) const
{
  if (role == Qt::BackgroundRole)
    return Root->GetIndex() != index.parent() ? m_defaultGroupMemberColor : QVariant();
  if (index.column() == ColumnSampleType)
  {
    int v = TreeModelDynamic::data(index, Qt::DisplayRole).toInt();
    if (role == Qt::DecorationRole)
      return m_iconsToColumnsMap[v];
    if (role == Qt::DisplayRole)
      return InvisibleChars[v];
  }
  return TreeModelDynamic::data(index, role);
}

bool TreeModelDynamicProjectFragments::setData(const QModelIndex& index, const QVariant& value, int role)
{
  if (const auto item = GetItem(index);
    item->GetData(file_type_id).toInt() == ProjectFiles::sim)
  {
    if (role == Qt::CheckStateRole)
    {
      if (value.value<Qt::CheckState>() == Qt::Checked)
      {
        if (m_isInMarkupPickMode)
        {
          if (ionReferenceName.isEmpty())
          {
            setIonNameCheckedFilter(item->GetData(filename).toString());
          }
        }
      }
      else
      {
        if (getCheckedItems(GetRoot()).size() == 1)
        {
          resetIonNameCheckedFilter();
        }
      }
    }
  }
  return TreeModelDynamic::setData(index, value, role);
}

Qt::ItemFlags TreeModelDynamicProjectFragments::flags(const QModelIndex& index) const
{
  Qt::ItemFlags f = TreeModel::flags(index);
  if (Checkable)
  {
    if (m_isInPassportPickMode)
    {
      auto item = GetItem(index);
      auto data = item->GetData(checkable);
      if (data.isValid() && !data.isNull())
        f.setFlag(Qt::ItemIsUserCheckable, data.toBool());
      else
        f.setFlag(Qt::ItemIsUserCheckable, true);
    }
    else if (m_isInMarkupPickMode)
    {
      auto item = GetItem(index);
      if (item->GetData(file_type_id).toInt() != ProjectFiles::sim)
      {
        f.setFlag(Qt::ItemIsUserCheckable, false);
      }
      else
      {
        auto data = item->GetData(checkable);
        if (data.isValid() && !data.isNull())
          f.setFlag(Qt::ItemIsUserCheckable, data.toBool());
        else
          f.setFlag(Qt::ItemIsUserCheckable, true);
      }
    }
    else
    {
      f.setFlag(Qt::ItemIsUserCheckable, true);
    }
  }
  if (index.column() == columnCount() - 1)
    f |= Qt::ItemIsEditable;
  return f;
}

void TreeModelDynamicProjectFragments::setMarkupPickModeEnabled(bool isInMarkupPickMode)
{
  beginResetModel();
  if (m_isInMarkupPickMode && m_isInMarkupPickMode != isInMarkupPickMode)
    resetIonNameCheckedFilter();
  m_isInMarkupPickMode = isInMarkupPickMode;
  endResetModel();
}

void TreeModelDynamicProjectFragments::enterPassportMode()
{
  beginResetModel();
  m_isInPassportPickMode = true;
  for (auto& item : Root->GetChildrenRecursive())
  {
    switch (item->GetData(file_type_id).toInt()) {
      case ProjectFiles::tic:
        item->SetData(checkable, true);
        break;

      case ProjectFiles::sim:
          item->SetData(checkable, item->GetParent() == Root);
        break;

      default:
        item->SetData(checkable, false);
        break;
    }
  }
  endResetModel();
}

void TreeModelDynamicProjectFragments::exitPassportMode()
{
  beginResetModel();
  m_isInPassportPickMode = false;
  for (auto& item : Root->GetChildrenRecursive())
    item->SetData(checkable, true);
  endResetModel();
}

bool TreeModelDynamicProjectFragments::isInMarkupPickMode() const
{
  return m_isInMarkupPickMode;
}
