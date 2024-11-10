#include "tree_model_dynamic_project_users.h"
#include "tree_model_item.h"

#include "context_root.h"

#include "../api/api_rest.h"
#include "../logic/notification.h"
#include <logic/known_context_tag_names.h>

#include <QVariantMap>
#include <QDateTime>

using namespace Core;

////////////////////////////////////////////////////
//// Tree model dynamic / ProjectUsers
TreeModelDynamicProjectUsers::TreeModelDynamicProjectUsers(QObject* parent)
  : TreeModelDynamic(parent)
{
  //// Setup columns
  Columns[ColumnFullName].Ids[Qt::EditRole]                     = "full_name";
  Columns[ColumnFullName].Ids[Qt::DisplayRole]                  = "full_name";
  Columns[ColumnFullName].HeaderData[Qt::DisplayRole]           = tr("Name");
  Columns[ColumnFullName].Filter                                = ColumnInfo::FilterTypeTextSubstring;

  Columns[ColumnRole].Ids[Qt::EditRole]                         = "role";
  Columns[ColumnRole].Ids[Qt::DisplayRole]                      = "role";
  Columns[ColumnRole].HeaderData[Qt::DisplayRole]               = tr("Role");
  Columns[ColumnRole].Filter                                    = ColumnInfo::FilterTypeTextValuesList;

  Columns[ColumnAccess].Ids[Qt::EditRole]                       = "access";
  Columns[ColumnAccess].Ids[Qt::DisplayRole]                    = "access";
  Columns[ColumnAccess].HeaderData[Qt::DisplayRole]             = tr("Access");
  Columns[ColumnAccess].Filter                                  = ColumnInfo::FilterTypeTextValuesList;

  //// Setup item actions
  ItemActions =
  {
    { "remove", QString(), tr("Remove") }
  };

  Reset();
}

TreeModelDynamicProjectUsers::~TreeModelDynamicProjectUsers()
{
}

//// Handle
void TreeModelDynamicProjectUsers::ApplyContextProject(const QString &dataId, const QVariant &data)
{
  bool isReset = isDataReset(dataId, data);
  if(isReset)
  {
    Reset();
  }
  else if (dataId ==  Names::ContextTagNames::ProjectId)
  {
    if(data.toInt())
      Reset();
    else
      Clear();
  }
}

void TreeModelDynamicProjectUsers::Reset()
{
  //// Clear
  TreeModelDynamic::Reset();

  //// Fetch
//  if (canFetchMore(QModelIndex()))
//  {
//    FetchMore(Root);
//  }
}

bool TreeModelDynamicProjectUsers::canFetchMore(const QModelIndex& parent) const
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

void TreeModelDynamicProjectUsers::fetchMore(const QModelIndex& parent)
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

void TreeModelDynamicProjectUsers::FetchMore(QPointer<TreeModelItem> item)
{
  if (item->CanFetchMore())
  {
    item->SetFetched(true);

    API::REST::Tables::GetProjectUsers(
      GenesisContextRoot::Get()->ProjectId(),
      [item](QNetworkReply*, QVariantMap result)
      {
        if (!item)
          return;

        //// Transform
        if (result.contains("children"))
        {
          QVariantList children = result["children"].toList();
          for (auto& c : children)
          {
            auto row = c.toMap();
            row["full_name"] = row["first_name"].toString() + " "
                             + row["last_name"].toString();
            c = row;
          }
          result["children"] = children;
        }

        item->Load(result);
      },
      [item](QNetworkReply*, QNetworkReply::NetworkError err)
      {
        if (!item)
          return;

        item->Clear();
        Notification::NotifyError(tr("Failed to get users from database"), err);
      }
    );
  }
}
