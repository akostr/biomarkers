#include "tree_model_dynamic_identification_plots.h"

#include "../api/api_rest.h"
#include "../logic/notification.h"
#include "logic/context_root.h"
#include "logic/known_context_tag_names.h"
#include "logic/known_json_tag_names.h"

TreeModelDynamicIdentificationPlots::TreeModelDynamicIdentificationPlots(QObject *parent)
  : TreeModelDynamic(parent)
{
  Columns[ColumnTitle].Ids[Qt::EditRole] = "plot_title";
  Columns[ColumnTitle].Ids[Qt::DisplayRole] = "plot_title";
  Columns[ColumnTitle].HeaderData[Qt::DisplayRole] = tr("Title");
  Columns[ColumnTitle].Filter = ColumnInfo::FilterTypeTextValuesList;

  Columns[ColumnTableTitle].Ids[Qt::EditRole] = "data_table_title";
  Columns[ColumnTableTitle].Ids[Qt::DisplayRole] = "data_table_title";
  Columns[ColumnTableTitle].HeaderData[Qt::DisplayRole] = tr("Source table");
  Columns[ColumnTableTitle].Filter = ColumnInfo::FilterTypeTextValuesList;

  Columns[ColumnComment].Ids[Qt::EditRole] = "comment";
  Columns[ColumnComment].Ids[Qt::DisplayRole] = "comment";
  Columns[ColumnComment].HeaderData[Qt::DisplayRole] = tr("Comment");
  Columns[ColumnComment].Filter = ColumnInfo::FilterTypeTextValuesList;

  Columns[ColumnCreatedDate].Ids[Qt::EditRole] = "created";
  Columns[ColumnCreatedDate].Ids[Qt::DisplayRole] = "created";
  Columns[ColumnCreatedDate].HeaderData[Qt::DisplayRole] = tr("Created date");
  Columns[ColumnCreatedDate].Filter = ColumnInfo::FilterTypeDateRange;
  //// Setup item actions
  ItemActions = {
    { "edit_title", QString(), tr("Edit title and comment") },
    { "change_group", QString(), tr("Change group") },
    { "remove", QString(), tr("Remove") }
  };
  Reset();

}

void TreeModelDynamicIdentificationPlots::ApplyContextProject(const QString& dataId, const QVariant& data)
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

void TreeModelDynamicIdentificationPlots::ApplyContextUser(const QString &dataId, const QVariant &data)
{
  bool isReset = isDataReset(dataId, data);
  if(isReset)
  {
    Reset();
  }
  else if(dataId == Names::UserContextTags::kUserState)
  {
    if(data.toInt() == Names::UserContextTags::authorized)
      Reset();
    else
      Clear();
  }
}

bool TreeModelDynamicIdentificationPlots::canFetchMore(const QModelIndex& parent) const
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

void TreeModelDynamicIdentificationPlots::fetchMore(const QModelIndex& parent)
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

void TreeModelDynamicIdentificationPlots::FetchMore(QPointer<TreeModelItem> item)
{
  if (item->CanFetchMore())
  {
    item->SetFetched(true);
    int projectId = Core::GenesisContextRoot::Get()->ProjectId();
    API::REST::Tables::GetIdentificationPlots(
      projectId,
      [item, this](QNetworkReply*, QVariantMap result)
      {
        if (!item)
          return;

        item->Load(result);
        sortIntoGroups(item);
      },
      [item](QNetworkReply*, QNetworkReply::NetworkError err)
      {
        if (!item)
          return;

        item->Clear();
        Notification::NotifyError(tr("Failed to get file information from database"), err);
      }
      );
  }
}

void TreeModelDynamicIdentificationPlots::sortIntoGroups(QPointer<TreeModelItem> item)
{
  beginResetModel();
  QMap<int, QPointer<TreeModelItem>> groupsItems;
  int groupsCount = 0;

  for (auto& child : item->GetChildren())
  {
    if (!child->GetData("plot_group_id").isNull())
    {
      int groupId = child->GetData("plot_group_id").toInt();
      if (!groupsItems.contains(groupId))
      {
        auto groupItem = new TreeModelItem(item, groupsCount++);
        groupItem->SetData("plot_group_title", child->GetData("plot_group_title").toString());
        groupItem->SetData("plot_title", child->GetData("plot_group_title").toString());
        groupItem->SetData("plot_group_id", groupId);
        groupItem->SetData("group", true);
        groupsItems[groupId] = groupItem;
      }
      item->MoveChildTo(child->GetRow(), groupsItems[groupId], false);
    }
  }
  endResetModel();
}

bool TreeModelDynamicIdentificationPlots::isItemIsGroup(QPointer<TreeModelItem> item) const
{
  if (!item)
    return false;
  auto isGroup = item->GetData("group");
  return isGroup.isValid() && !isGroup.isNull() && isGroup.toBool();
}

//// Get actions for item
QList<ActionInfo> TreeModelDynamicIdentificationPlots::GetItemActions(const QModelIndex& index/*index*/)
{
  auto item = GetItem(index);
  if (isItemIsGroup(item))
  {
    ItemActions = {
      { "edit_group_title", QString(), tr("Edit title") },
      { "change_group", QString(), tr("Change group") },
    };
  }
  else
  {
    ItemActions = {
      { "edit_item_title", QString(), tr("Edit title and comment") },
      { "change_group", QString(), tr("Change group") },
      { "remove_item", QString(), tr("Remove") }
    };
  }
  return ItemActions;
}
