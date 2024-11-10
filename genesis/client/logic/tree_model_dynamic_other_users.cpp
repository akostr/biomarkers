//#include "tree_model_dynamic_other_users.h"
//#include "tree_model_item.h"

//#include "context_root.h"

//#include "../api/api_rest.h"
//#include "../logic/notification.h"
//#include <logic/known_context_tag_names.h>

//#include <QVariantMap>
//#include <QDateTime>

//using namespace Core;

//////////////////////////////////////////////////////
////// Tree model dynamic / OtherUsers
//TreeModelDynamicOtherUsers::TreeModelDynamicOtherUsers(QObject* parent)
//  : TreeModelDynamic(parent)
//{
//  //// Setup columns
//  Columns[ColumnId].Ids[Qt::EditRole]                           = "id";
//  Columns[ColumnId].Ids[Qt::DisplayRole]                        = "id";
//  Columns[ColumnId].HeaderData[Qt::DisplayRole]                 = tr("Id");

//  Columns[ColumnFullName].Ids[Qt::EditRole]                     = "full_name";
//  Columns[ColumnFullName].Ids[Qt::DisplayRole]                  = "full_name";
//  Columns[ColumnFullName].HeaderData[Qt::DisplayRole]           = tr("Name");

//  Columns[ColumnEmail].Ids[Qt::EditRole]                        = "email";
//  Columns[ColumnEmail].Ids[Qt::DisplayRole]                     = "email";
//  Columns[ColumnEmail].HeaderData[Qt::DisplayRole]              = tr("Email");

//  Columns[ColumnPhone].Ids[Qt::EditRole]                        = "phone";
//  Columns[ColumnPhone].Ids[Qt::DisplayRole]                     = "phone";
//  Columns[ColumnPhone].HeaderData[Qt::DisplayRole]              = tr("Phone");

//  Columns[ColumnFunction].Ids[Qt::EditRole]                     = "function";
//  Columns[ColumnFunction].Ids[Qt::DisplayRole]                  = "function";
//  Columns[ColumnFunction].HeaderData[Qt::DisplayRole]           = tr("Function");

//  Reset();
//}

//TreeModelDynamicOtherUsers::~TreeModelDynamicOtherUsers()
//{
//}

////// Handle
//void TreeModelDynamicOtherUsers::ApplyContextUser(GenesisContextPtr context)
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

//void TreeModelDynamicOtherUsers::Reset()
//{
//  //// Clear
//  TreeModelDynamic::Reset();

//  //// Fetch
//  if (canFetchMore(QModelIndex()))
//  {
//    FetchMore(Root);
//  }
//}

//bool TreeModelDynamicOtherUsers::canFetchMore(const QModelIndex& parent) const
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

//void TreeModelDynamicOtherUsers::fetchMore(const QModelIndex& parent)
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

//void TreeModelDynamicOtherUsers::FetchMore(QPointer<TreeModelItem> item)
//{
//  if (item->CanFetchMore())
//  {
//    item->SetFetched(true);

//    API::REST::Tables::GetOtherUsers(
//      GenesisContextRoot::Get()->GetContextUser()->GetData(Names::UserContextTags::kUserId).toString().toInt(),
//      [item](QNetworkReply*, QVariantMap result)
//      {
//        if (!item)
//          return;

//        //// Transform
//        if (result.contains("children"))
//        {
//          QVariantList children = result["children"].toList();
//          for (auto& c : children)
//          {
//            auto row = c.toMap();
//            row["full_name"] = row["first_name"].toString() + " "
//                             + row["last_name"].toString();
//            c = row;
//          }
//          result["children"] = children;
//        }

//        item->Load(result);
//      },
//      [item](QNetworkReply*, QNetworkReply::NetworkError err)
//      {
//        if (!item)
//          return;

//        item->Clear();
//        Notification::NotifyError(tr("Failed to get users from database"), err);
//      }
//    );
//  }
//}
