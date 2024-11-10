#include "tree_model_dynamic_access.h"
#include "tree_model_item.h"

#include "../api/api_rest.h"
#include "../logic/notification.h"
#include <logic/known_context_tag_names.h>
#include <logic/context_root.h>

#include <QVariantMap>
#include <QDateTime>

#include <QDebug>

using namespace Core;
////////////////////////////////////////////////////
//// Tree model dynamic / Access
TreeModelDynamicAccess::TreeModelDynamicAccess(QObject* parent)
  : TreeModelDynamic(parent)
{
  //// Setup columns
  Columns[ColumnId].Ids[Qt::EditRole]             = "id";
  Columns[ColumnId].Ids[Qt::DisplayRole]          = "id";
  Columns[ColumnId].HeaderData[Qt::DisplayRole]   = tr("Id");

  Columns[ColumnAccess].Ids[Qt::EditRole]           = "access";
  Columns[ColumnAccess].Ids[Qt::DisplayRole]        = "access";
  Columns[ColumnAccess].HeaderData[Qt::DisplayRole] = tr("Access");

  Reset();
}

TreeModelDynamicAccess::~TreeModelDynamicAccess()
{
}

//// Handle
void TreeModelDynamicAccess::ApplyContextUser(const QString& dataId, const QVariant& data)
{
  bool isReset = isDataReset(dataId, data);
  if(isReset)
  {
    auto context = GenesisContextRoot::Get()->GetContextUser();
    int userState = context->GetData(Names::UserContextTags::kUserState).toInt();
    if (userState == Names::UserContextTags::authorized)
      Reset();
    else
      Clear();
  }
  else if(dataId == Names::UserContextTags::kUserState)
  {
    if (Names::UserContextTags::kUserState.toInt() == Names::UserContextTags::authorized)
      Reset();
    else
      Clear();
  }
}

void TreeModelDynamicAccess::Reset()
{
  //// Clear
  TreeModelDynamic::Reset();

  //// Fetch
//  if (canFetchMore(QModelIndex()))
//  {
//    FetchMore(Root);
//  }
}

bool TreeModelDynamicAccess::canFetchMore(const QModelIndex& parent) const
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

void TreeModelDynamicAccess::fetchMore(const QModelIndex& parent)
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

void TreeModelDynamicAccess::FetchMore(QPointer<TreeModelItem> item)
{
  if (item->CanFetchMore())
  {
    item->SetFetched(true);

    API::REST::Tables::GetAccess(
      [item](QNetworkReply*, QVariantMap result)
      {
        if (!item)
          return;

        item->Load(result);
      },
      [item](QNetworkReply*, QNetworkReply::NetworkError err)
      {
        if (!item)
          return;

        item->Clear();
        Notification::NotifyError(tr("Failed to get access constants from database"), err);
      }
    );
  }
}
