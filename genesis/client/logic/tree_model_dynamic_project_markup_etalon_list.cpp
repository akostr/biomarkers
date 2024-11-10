#include "tree_model_dynamic_project_markup_etalon_list.h"

#include "tree_model_item.h"
#include "logic/context_root.h"
#include "logic/known_context_tag_names.h"
#include "logic/notification.h"
#include "api/api_rest.h"

#include <QPainter>

using namespace Core;
using namespace Names;

TreeModelDynamicProjectMarkupEtalonList::TreeModelDynamicProjectMarkupEtalonList(QObject* parent)
  : TreeModelDynamic(parent)
{
  //// Setup columns
  Columns[ColumnTitle].Ids[Qt::EditRole] = "_title";
  Columns[ColumnTitle].Ids[Qt::DisplayRole] = "_title";
  Columns[ColumnTitle].HeaderData[Qt::DisplayRole] = tr("File name");
  Columns[ColumnTitle].Filter = ColumnInfo::FilterTypeTextSubstring;

  Columns[ColumnMarkup].Ids[Qt::EditRole] = "_markup_title";
  Columns[ColumnMarkup].Ids[Qt::DisplayRole] = "_markup_title";
  Columns[ColumnMarkup].HeaderData[Qt::DisplayRole] = tr("Markup name");
  Columns[ColumnMarkup].Filter = ColumnInfo::FilterTypeTextSubstring;

  Columns[ColumnComment].Ids[Qt::EditRole] = "_comment";
  Columns[ColumnComment].Ids[Qt::DisplayRole] = "_comment";
  Columns[ColumnComment].HeaderData[Qt::DisplayRole] = tr("Comment");
  Columns[ColumnComment].Filter = ColumnInfo::FilterTypeTextSubstring;

  Columns[ColumnCreated].Ids[Qt::EditRole] = "_changed";
  Columns[ColumnCreated].Ids[Qt::DisplayRole] = "_changed";
  Columns[ColumnCreated].HeaderData[Qt::DisplayRole] = tr("Date");
  Columns[ColumnCreated].Filter = ColumnInfo::FilterTypeTextSubstring;

  //// Setup item actions
  ItemActions =
  {
    { "edit", QString(), tr("Edit title and comment") },
    { "remove", QString(), tr("Remove etalon") },
  };

  Reset();
}

void TreeModelDynamicProjectMarkupEtalonList::ApplyContextProject(const QString &dataId, const QVariant &data)
{
  bool isReset = isDataReset(dataId, data);
  if(isReset)
  {
    Reset();
  }
  else if (dataId == Names::ContextTagNames::ProjectId)
  {
    if(data.toInt())
      Reset();
    else
      Clear();
  }
}

bool TreeModelDynamicProjectMarkupEtalonList::canFetchMore(const QModelIndex& parent) const
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

void TreeModelDynamicProjectMarkupEtalonList::fetchMore(const QModelIndex& parent)
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

void TreeModelDynamicProjectMarkupEtalonList::FetchMore(QPointer<TreeModelItem> item)
{
  if (item->CanFetchMore())
  {
    item->SetFetched(true);
    const auto context = GenesisContextRoot::Get()->GetContextProject();
    const auto projectId = context->GetData(ContextTagNames::ProjectId).toInt();
    API::REST::Tables::GetProjectReferences(projectId,
      [item](QNetworkReply*, QVariantMap map)
      {
        item->Load(map);
      },
      [item](QNetworkReply*, QNetworkReply::NetworkError err)
      {
        item->Clear();
        Notification::NotifyError(tr("Error receive reference data."), err);
      });
  }
}
