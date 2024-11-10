#include "tree_model_dynamic_project_children.h"
#include "tree_model_item.h"

#include "../api/api_rest.h"
#include "../logic/notification.h"
#include <logic/known_context_tag_names.h>

#include <logic/context_root.h>
#include <QVariantMap>
#include <QDateTime>

using namespace Core;

////////////////////////////////////////////////////
//// Tree model dynamic / ProjectChildren
TreeModelDynamicProjectChildren::TreeModelDynamicProjectChildren(QObject* parent)
  : TreeModelDynamic(parent)
{
  //// Setup columns
  Columns[ColumnId].Ids[Qt::EditRole]                           = "id";
  Columns[ColumnId].Ids[Qt::DisplayRole]                        = "id";
  Columns[ColumnId].HeaderData[Qt::DisplayRole]                 = tr("Id");
  Columns[ColumnId].Filter                                      = ColumnInfo::FilterTypeTextSubstring;

  Columns[ColumnName].Ids[Qt::EditRole]                         = "title";
  Columns[ColumnName].Ids[Qt::DisplayRole]                      = "title";
  Columns[ColumnName].HeaderData[Qt::DisplayRole]               = tr("Project name");
  Columns[ColumnName].Filter                                    = ColumnInfo::FilterTypeTextSubstring;

  /*
  Columns[ColumnParentName].Ids[Qt::EditRole]                   = "parent_title";
  Columns[ColumnParentName].Ids[Qt::DisplayRole]                = "parent_title";
  Columns[ColumnParentName].HeaderData[Qt::DisplayRole]         = tr("Parent project");

  Columns[ColumnChromatogramCount].Ids[Qt::EditRole]            = "count";
  Columns[ColumnChromatogramCount].Ids[Qt::DisplayRole]         = "count";
  Columns[ColumnChromatogramCount].HeaderData[Qt::DisplayRole]  = tr("Chromatogram\ncount");
  */

  /*
  Columns[ColumnStatus].Ids[Qt::EditRole]                       = "status";
  Columns[ColumnStatus].Ids[Qt::DisplayRole]                    = "status";
  Columns[ColumnStatus].HeaderData[Qt::DisplayRole]             = tr("Status");
  */

  /*Columns[ColumnAuthorFullName].Ids[Qt::EditRole]               = "full_name";
  Columns[ColumnAuthorFullName].Ids[Qt::DisplayRole]            = "full_name";
  Columns[ColumnAuthorFullName].HeaderData[Qt::DisplayRole]     = tr("Author");

  Columns[ColumnRole].Ids[Qt::EditRole]                         = "role";
  Columns[ColumnRole].Ids[Qt::DisplayRole]                      = "role";
  Columns[ColumnRole].HeaderData[Qt::DisplayRole]               = tr("Role");

  Columns[ColumnAccess].Ids[Qt::EditRole]                       = "access";
  Columns[ColumnAccess].Ids[Qt::DisplayRole]                    = "access";
  Columns[ColumnAccess].HeaderData[Qt::DisplayRole]             = tr("Access");*/

  Columns[ColumnDateCreated].Ids[Qt::EditRole]                  = "created";
  Columns[ColumnDateCreated].Ids[Qt::DisplayRole]               = "created";
  Columns[ColumnDateCreated].HeaderData[Qt::DisplayRole]        = tr("Creation date");
  Columns[ColumnDateCreated].Filter                             = ColumnInfo::FilterTypeDateRange;

  //// Setup item actions
  ItemActions =
  {
    { "open", QString(), tr("Open") }
  };

  Reset();
}

TreeModelDynamicProjectChildren::~TreeModelDynamicProjectChildren()
{
}

//// Handle
void TreeModelDynamicProjectChildren::ApplyContextUser(const QString &dataId, const QVariant &data)
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

void TreeModelDynamicProjectChildren::ApplyContextProject(const QString &dataId, const QVariant &data)
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

void TreeModelDynamicProjectChildren::Reset()
{
  //// Clear
  TreeModelDynamic::Reset();

  //// Fetch
//  if (canFetchMore(QModelIndex()))
//  {
//    FetchMore(Root);
//  }
}

bool TreeModelDynamicProjectChildren::canFetchMore(const QModelIndex& parent) const
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

void TreeModelDynamicProjectChildren::fetchMore(const QModelIndex& parent)
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

void TreeModelDynamicProjectChildren::FetchMore(QPointer<TreeModelItem> item)
{
  if (item->CanFetchMore())
  {
    item->SetFetched(true);

    if (int projectId = GenesisContextRoot::Get()->ProjectId())
    {
      API::REST::Tables::GetProjectChildren(
        projectId,
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
          Notification::NotifyError(tr("Failed to get subprojects from database"), err);
        }
        );
    }
  }
}
