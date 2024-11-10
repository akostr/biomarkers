//#include "tree_model_dynamic_roles.h"
//#include "tree_model_item.h"

//#include "../api/api_rest.h"
//#include "../logic/notification.h"
//#include <logic/known_context_tag_names.h>

//#include <QVariantMap>
//#include <QDateTime>

//using namespace Core;

//////////////////////////////////////////////////////
////// Tree model dynamic / Roles
//TreeModelDynamicRoles::TreeModelDynamicRoles(QObject* parent)
//  : TreeModelDynamic(parent)
//{
//  //// Setup columns
//  Columns[ColumnId].Ids[Qt::EditRole]             = "id";
//  Columns[ColumnId].Ids[Qt::DisplayRole]          = "id";
//  Columns[ColumnId].HeaderData[Qt::DisplayRole]   = tr("Id");

//  Columns[ColumnRole].Ids[Qt::EditRole]           = "role";
//  Columns[ColumnRole].Ids[Qt::DisplayRole]        = "role";
//  Columns[ColumnRole].HeaderData[Qt::DisplayRole] = tr("Role");

//  Reset();
//}

//TreeModelDynamicRoles::~TreeModelDynamicRoles()
//{
//}

////// Handle
//void TreeModelDynamicRoles::ApplyContextUser(GenesisContextPtr context)
//{
//  if (context->GetData(Names::UserContextTags::kUserState).toInt() == Names::UserContextTags::authorized)
//  {
//    Reset();
//  }
//  else
//  {
//    Clear();
//  }
//}

//void TreeModelDynamicRoles::Reset()
//{
//  //// Clear
//  TreeModelDynamic::Reset();

//  //// Fetch
//  if (canFetchMore(QModelIndex()))
//  {
//    FetchMore(Root);
//  }
//}

//bool TreeModelDynamicRoles::canFetchMore(const QModelIndex& parent) const
//{
//  QPointer<TreeModelItem> parentItem;
//  if (parent.column() > 0)
//    return 0;

//  if (!parent.isValid())
//    parentItem = Root;
//  else
//    parentItem = static_cast<TreeModelItem*>(parent.internalPointer());

//  return parentItem->CanFetchMore();
//}

//void TreeModelDynamicRoles::fetchMore(const QModelIndex& parent)
//{
//  QPointer<TreeModelItem> parentItem;
//  if (parent.column() > 0)
//    return;

//  if (!parent.isValid())
//    parentItem = Root;
//  else
//    parentItem = static_cast<TreeModelItem*>(parent.internalPointer());

//  if (!parentItem->CanFetchMore())
//    return;

//  FetchMore(parentItem);
//}

//void TreeModelDynamicRoles::FetchMore(QPointer<TreeModelItem> item)
//{
//  if (item->CanFetchMore())
//  {
//    item->SetFetched(true);

//    API::REST::Tables::GetRoles(
//      [item](QNetworkReply*, QVariantMap result)
//      {
//        if (!item)
//          return;

//        item->Load(result);
//      },
//      [item](QNetworkReply*, QNetworkReply::NetworkError err)
//      {
//        if (!item)
//          return;

//        item->Clear();
//        Notification::NotifyError(tr("Failed to get roles constants from database"), err);
//      }
//    );
//  }
//}
