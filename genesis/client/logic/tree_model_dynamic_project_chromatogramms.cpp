#include "tree_model_dynamic_project_chromatogramms.h"
#include "tree_model_item.h"

#include "../api/api_rest.h"
#include "../logic/notification.h"
#include <logic/known_context_tag_names.h>
#include <logic/enums.h>

#include <logic/context_root.h>

using namespace Core;

namespace
{
  QStringList InvisibleChars = { u8"\u2000", u8"\u2001", u8"\u2002", u8"\u2003", u8"\u2004" };
}

const QColor TreeModelDynamicProjectChromatogramms::DefaultGroupMemberColor = QColor(0, 39, 59, 13);

TreeModelDynamicProjectChromatogramms::TreeModelDynamicProjectChromatogramms(QObject* parent)
  : TreeModelDynamic{ parent }
  , m_checkableOnlyWithFragments(false)
{
  //// Setup columns
  Columns[ColumnProjectId].Ids[Qt::EditRole] = "project_file_id";
  Columns[ColumnProjectId].Ids[Qt::DisplayRole] = "project_file_id";
  Columns[ColumnProjectId].HeaderData[Qt::DisplayRole] = tr("ID");
  Columns[ColumnProjectId].Filter = ColumnInfo::FilterTypeTextSubstring;

  //  Columns[ColumnFileId].Ids[Qt::EditRole]                           = "id";
  //  Columns[ColumnFileId].Ids[Qt::DisplayRole]                        = "id";
  //  Columns[ColumnFileId].HeaderData[Qt::DisplayRole]                 = tr("File ID");
  //  Columns[ColumnFileId].Filter                                      = ColumnInfo::FilterTypeTextSubstring;

  Columns[ColumnFileName].Ids[Qt::EditRole] = "filename";
  Columns[ColumnFileName].Ids[Qt::DisplayRole] = "filename";
  Columns[ColumnFileName].HeaderData[Qt::DisplayRole] = tr("File name");
  Columns[ColumnFileName].Filter = ColumnInfo::FilterTypeTextSubstring;

  Columns[ColumnSampleType].Ids[Qt::ToolTipRole] = "sample_type";
  Columns[ColumnSampleType].Ids[Qt::DisplayRole] = "sample_type_id";
  Columns[ColumnSampleType].HeaderData[Qt::DisplayRole] = tr("Sample type");
  Columns[ColumnSampleType].Filter = ColumnInfo::FilterTypeTextSubstring;

  Columns[ColumnType].Ids[Qt::EditRole] = "file_type";
  Columns[ColumnType].Ids[Qt::DisplayRole] = "file_type";
  Columns[ColumnType].Ids[Qt::UserRole] = "file_type_id";
  Columns[ColumnType].HeaderData[Qt::DisplayRole] = tr("File type");
  Columns[ColumnType].Filter = ColumnInfo::FilterTypeTextSubstring;

  Columns[ColumnField].Ids[Qt::EditRole] = "field_title";
  Columns[ColumnField].Ids[Qt::DisplayRole] = "field_title";
  Columns[ColumnField].HeaderData[Qt::DisplayRole] = tr("Field");
  Columns[ColumnField].Filter = ColumnInfo::FilterTypeTextSubstring;

  Columns[ColumnWellCluster].Ids[Qt::EditRole] = "well_cluster_title";
  Columns[ColumnWellCluster].Ids[Qt::DisplayRole] = "well_cluster_title";
  Columns[ColumnWellCluster].HeaderData[Qt::DisplayRole] = tr("Well cluster");
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
  ItemActions =
  {
    { "extract_ions", "", tr("Extract fragment ions") },
    { "change_grouping", "", tr("Change grouping") },
    // { "copy_to_other_project", "", tr("Copy to other project") },
    { "view_sample_passport", "", tr("View sample passport") },
    { "export", "", tr("Export") },
    { "delete", "", tr("Delete") }
  };
  DefaultGroupActions =
  {
    {"rename_group", "", tr("Rename group")},
    {"edit_group","", tr("Change grouping")},
    {"export", "", tr("Export")},
  };

  m_iconsToColumnsMap[0] = QPixmap();
  m_iconsToColumnsMap[1] = QPixmap(":/resource/icons/sample_type_1@2x.png");
  m_iconsToColumnsMap[2] = QPixmap(":/resource/icons/sample_type_2@2x.png");
  m_iconsToColumnsMap[3] = QPixmap(":/resource/icons/sample_type_3@2x.png");
  m_iconsToColumnsMap[4] = QPixmap(":/resource/icons/sample_type_4@2x.png");
  Reset();
}

TreeModelDynamicProjectChromatogramms::~TreeModelDynamicProjectChromatogramms()
{
}

QList<TreeModelItem*> TreeModelDynamicProjectChromatogramms::GetCheckedItems(TreeModelItem* parent)
{
  if (!parent)
    parent = Root;
  std::function<void(QList<TreeModelItem*>&, TreeModelItem*)> getCheckedChildren = [this, &getCheckedChildren](QList<TreeModelItem*>& list, TreeModelItem* item)
    {
      if (!item)
        return;
      if (!isItemIsGroup(item) && item != Root)
      {
        if (item->GetCheckState() == Qt::Checked)
          list.append(item);
      }
      for (auto& child : item->GetChildren())
      {
        getCheckedChildren(list, child);
      }
    };
  QList<TreeModelItem*> ret;
  getCheckedChildren(ret, parent);
  return ret;
}

void TreeModelDynamicProjectChromatogramms::ApplyContextUser(const QString& dataId, const QVariant& data)
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

void TreeModelDynamicProjectChromatogramms::ApplyContextProject(const QString& dataId, const QVariant& data)
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

void TreeModelDynamicProjectChromatogramms::Reset()
{
  beginResetModel();
  Root->Clear(false);
  Root->SetFetched(false);
  //  if (canFetchMore(QModelIndex()))
  //    FetchMore(Root);
  endResetModel();
}

bool TreeModelDynamicProjectChromatogramms::canFetchMore(const QModelIndex& parent) const
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

void TreeModelDynamicProjectChromatogramms::fetchMore(const QModelIndex& parent)
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

void TreeModelDynamicProjectChromatogramms::FetchMore(QPointer<TreeModelItem> item)
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
          if (result.contains("children"))
          {
            QVariantList children = result["children"].toList();
            children.removeIf([children](const QVariant& val)->bool
              {
                return (val.toMap()["file_type_id"].toInt() == ProjectFiles::sim);
              });
            for (auto& c : children)
            {
              auto row = c.toMap();
              QStringList filePath = row["filename"].toString().split("/");
              if (!filePath.empty())
                row["filename_short"] = filePath.last();
              c = row;
            }
            result["children"] = children;
          }
          beginResetModel();
          item->Load(result, false);
          sortIntoGroups(item);
          endResetModel();
          //          setupItemFlags(item);
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

QList<ActionInfo> TreeModelDynamicProjectChromatogramms::GetItemActions(const QModelIndex& index)
{
  if (isItemIsGroup(index))
    return DefaultGroupActions;
  if (GetItem(index)->GetData("file_type_id").toInt() != ProjectFiles::GC_MS)
  {
    auto actions = ItemActions;
    actions.removeIf([](const ActionInfo& v) { return v.Id == "extract_ions"; });
    return actions;
  }
  return ItemActions;
}

int TreeModelDynamicProjectChromatogramms::NonGroupItemsCount(TreeModelItem* parent)
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

void TreeModelDynamicProjectChromatogramms::SetChildrenCheckState(Qt::CheckState state, TreeModelItem* parent)
{
  beginResetModel();
  std::function<void(TreeModelItem*)> walkThrough = [this, &walkThrough, state](TreeModelItem* current)
    {
      auto index = current->GetIndex();
      if (index.isValid() && flags(index).testFlags(Qt::ItemIsUserCheckable))
        current->SetCheckState(state);
      for (auto& child : current->GetChildren())
        walkThrough(child);
    };
  walkThrough(parent);

  endResetModel();
}

int TreeModelDynamicProjectChromatogramms::GetCheckableCount()
{
  if (!Checkable)
    return 0;
  int count = 0;
  for (auto child : GetRoot()->GetChildrenRecursive())
  {
    if (flags(child->GetIndex()).testFlags(Qt::ItemIsUserCheckable))
      count++;
  }
  return count;
}

void TreeModelDynamicProjectChromatogramms::sortIntoGroups(QPointer<TreeModelItem> item)
{
  QMap<int, QPointer<TreeModelItem>> groupsItems;
  int groupsCount = 0;

  for (auto& child : item->GetChildren())
  {
    if (!child->GetData("id_file_group").isNull())
    {
      int groupId = child->GetData("id_file_group").toInt();
      if (!groupsItems.contains(groupId))
      {
        auto groupItem = new TreeModelItem(item, groupsCount++);
        groupItem->SetData("project_file_id", child->GetData("group_title").toString());
        groupItem->SetData("group_id", groupId);
        groupItem->SetData("group", true);
        groupsItems[groupId] = groupItem;
      }
      item->MoveChildTo(child->GetRow(), groupsItems[groupId], false);
    }
  }
}

bool TreeModelDynamicProjectChromatogramms::isItemIsGroup(const QModelIndex& index) const
{
  return isItemIsGroup(GetItem(index));
}

bool TreeModelDynamicProjectChromatogramms::isItemIsGroup(const TreeModelItem* item)
{
  if (!item)
    return false;
  auto isGroup = item->GetData("group");
  return isGroup.isValid() && !isGroup.isNull() && isGroup.toBool();
}

QVariant TreeModelDynamicProjectChromatogramms::data(const QModelIndex& index, int role) const
{
  if (role == Qt::BackgroundRole)
    return Root->GetIndex() != index.parent() ? DefaultGroupMemberColor : QVariant();

  if (index.column() == ColumnSampleType)
  {
    auto v = TreeModelDynamic::data(index, Qt::DisplayRole).toInt();
    if(role == Qt::DecorationRole)
      return m_iconsToColumnsMap[v];
    if(role == Qt::DisplayRole)
      return InvisibleChars[v];
  }
  return TreeModelDynamic::data(index, role);
}

void TreeModelDynamicProjectChromatogramms::setCheckableOnlyWithFragments(bool enabled)
{
  m_checkableOnlyWithFragments = enabled;
}


Qt::ItemFlags TreeModelDynamicProjectChromatogramms::flags(const QModelIndex& index) const
{
  Qt::ItemFlags f = TreeModel::flags(index);
  if (Checkable)
  {
    if (m_checkableOnlyWithFragments)
    {
      auto item = GetItem(index);
      if (item)
      {
        auto data = GetItem(index)->GetData("file_type_id");
        if (data.isValid())
        {
          f.setFlag(Qt::ItemIsUserCheckable, data.toInt() == ProjectFiles::GC_MS);
        }
        else
        {
          f.setFlag(Qt::ItemIsUserCheckable, false);
        }
      }
    }
    else
      f |= Qt::ItemIsUserCheckable;
  }
  if (index.column() == columnCount() - 1)
    f |= Qt::ItemIsEditable;
  return f;
}
